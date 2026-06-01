//
//  StackChanController.swift
//  Apple Watch -> StackChan BLE 提线木偶控制器
//
//  作为 BLE central 连接固件里的 NimBLE 外设(广播名 "StackChan"),
//  向 Motion 特征写入 {"yawServo":{"angle":N},"pitchServo":{"angle":N}}。
//
//  固件实测约束(见 companion/watch-puppet/README.md):
//   - Motion 特征只支持 Write With Response(无 WRITE_NO_RSP),
//     所以这里做"单次在途"节流:上一帧 ACK 回来前不发下一帧,
//     永远只发最新姿态,天然丢弃过期帧,避免堆积延迟。
//   - 角度单位 degx10。yaw ∈ [-1280,1280],pitch ∈ [30,870]。
//

import Foundation
import CoreBluetooth

/// 与 firmware/main/hal/utils/bleprph/bleprph.h 保持一致
private enum StackChanUUID {
    static let service = CBUUID(string: "e2e5e5e0-1234-5678-1234-56789abcdef0")
    static let motion  = CBUUID(string: "e2e5e5e1-1234-5678-1234-56789abcdef0")
    // 固件还有一个 alt service e2e5e5ff-...,如固件以 alt 模式启动可加入扫描列表
    static let serviceAlt = CBUUID(string: "e2e5e5ff-1234-5678-1234-56789abcdef0")
}

enum LinkState: String {
    case poweredOff   = "蓝牙未开启"
    case scanning     = "搜索中…"
    case connecting   = "连接中…"
    case connected    = "已连接"
    case disconnected = "未连接"
}

final class StackChanController: NSObject, ObservableObject {
    @Published private(set) var state: LinkState = .disconnected
    @Published private(set) var lastSentYaw: Int = 0
    @Published private(set) var lastSentPitch: Int = 450

    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var motionChar: CBCharacteristic?

    // —— 单次在途节流 ——
    private var writeInFlight = false
    private var pendingYaw: Int?
    private var pendingPitch: Int?

    // 死区:角度变化小于该值(degx10)不发,减少 BLE 流量与舵机微抖
    private let deadband = 20  // = 2°

    override init() {
        super.init()
        central = CBCentralManager(delegate: self, queue: .main)
    }

    // MARK: - 公开 API

    func startScan() {
        guard central.state == .poweredOn else { return }
        state = .scanning
        central.scanForPeripherals(
            withServices: [StackChanUUID.service, StackChanUUID.serviceAlt],
            options: [CBCentralManagerScanOptionAllowDuplicatesKey: false]
        )
    }

    func disconnect() {
        if let p = peripheral { central.cancelPeripheralConnection(p) }
    }

    /// 由姿态采样回调高频调用;只缓存"最新目标",真正发送由 flush 决定
    func sendAngles(yaw: Int, pitch: Int) {
        let y = clamp(yaw, -1280, 1280)
        let p = clamp(pitch, 30, 870)
        pendingYaw = y
        pendingPitch = p
        flush()
    }

    // MARK: - 内部

    private func flush() {
        guard !writeInFlight,
              let char = motionChar,
              let p = peripheral,
              let yaw = pendingYaw,
              let pitch = pendingPitch else { return }

        // 死区判定:与上次实际发送相比变化太小则跳过
        if abs(yaw - lastSentYaw) < deadband && abs(pitch - lastSentPitch) < deadband {
            return
        }

        let json = "{\"yawServo\":{\"angle\":\(yaw)},\"pitchServo\":{\"angle\":\(pitch)}}"
        guard let data = json.data(using: .utf8) else { return }

        writeInFlight = true
        lastSentYaw = yaw
        lastSentPitch = pitch
        pendingYaw = nil
        pendingPitch = nil
        p.writeValue(data, for: char, type: .withResponse)
    }

    private func clamp(_ v: Int, _ lo: Int, _ hi: Int) -> Int { min(max(v, lo), hi) }
}

// MARK: - CBCentralManagerDelegate

extension StackChanController: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:  startScan()
        case .poweredOff: state = .poweredOff
        default:          state = .disconnected
        }
    }

    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String: Any],
                        rssi RSSI: NSNumber) {
        // 第一台匹配到的就连(家里一般只有一台);多台可在此按名字筛选
        central.stopScan()
        self.peripheral = peripheral
        peripheral.delegate = self
        state = .connecting
        central.connect(peripheral, options: nil)
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        peripheral.discoverServices([StackChanUUID.service, StackChanUUID.serviceAlt])
    }

    func centralManager(_ central: CBCentralManager,
                        didDisconnectPeripheral peripheral: CBPeripheral,
                        error: Error?) {
        state = .disconnected
        motionChar = nil
        writeInFlight = false
        // 自动重连
        startScan()
    }

    func centralManager(_ central: CBCentralManager,
                        didFailToConnect peripheral: CBPeripheral,
                        error: Error?) {
        state = .disconnected
        startScan()
    }
}

// MARK: - CBPeripheralDelegate

extension StackChanController: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for svc in services where svc.uuid == StackChanUUID.service || svc.uuid == StackChanUUID.serviceAlt {
            peripheral.discoverCharacteristics([StackChanUUID.motion], for: svc)
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didDiscoverCharacteristicsFor service: CBService,
                    error: Error?) {
        guard let chars = service.characteristics else { return }
        for c in chars where c.uuid == StackChanUUID.motion {
            motionChar = c
            state = .connected
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didWriteValueFor characteristic: CBCharacteristic,
                    error: Error?) {
        // 上一帧已落地,放行下一帧(发最新缓存的姿态)
        writeInFlight = false
        flush()
    }
}
