//
//  WristTracker.swift
//  读取手腕姿态并映射成 StackChan 舵机角度(degx10)
//
//  用 CMMotionManager 的 deviceMotion.attitude:
//   - pitch / roll 是相对重力的姿态,不会像 yaw(指南针航向)那样漂移,
//     所以提线木偶用 pitch+roll 最稳。
//   - 抬高/压低手腕  -> attitude.pitch -> 头部 pitch 舵机
//   - 手腕左右翻转    -> attitude.roll  -> 头部 yaw 舵机
//
//  "校准归中":记录当前姿态为基准,之后只映射相对基准的增量,
//  这样无论手臂朝哪个方向都舒服。
//

import Foundation
import CoreMotion

final class WristTracker: ObservableObject {
    @Published private(set) var isTracking = false

    private let motion = CMMotionManager()
    private let queue = OperationQueue()

    // 基准姿态(校准时刻),弧度
    private var refPitch = 0.0
    private var refRoll = 0.0

    // 映射参数
    private let pitchCenter = 450          // pitch 舵机中位(degx10),范围 30..870
    private let pitchGain = 600.0          // 每弧度对应多少 degx10(越大越灵敏)
    private let yawGain = 1400.0           // roll -> yaw 的增益
    // 注意符号:根据实际装配方向,若头转反了把对应 gain 取负即可

    /// 角度回调:(yawDegx10, pitchDegx10),约 50Hz
    var onAngles: ((Int, Int) -> Void)?

    func start() {
        guard motion.isDeviceMotionAvailable, !isTracking else { return }
        isTracking = true
        motion.deviceMotionUpdateInterval = 1.0 / 50.0  // 50Hz
        motion.startDeviceMotionUpdates(to: queue) { [weak self] dm, _ in
            guard let self, let dm else { return }
            let dPitch = dm.attitude.pitch - self.refPitch
            let dRoll  = dm.attitude.roll  - self.refRoll

            let pitch = Int(Double(self.pitchCenter) + dPitch * self.pitchGain)
            let yaw   = Int(dRoll * self.yawGain)

            DispatchQueue.main.async {
                self.onAngles?(yaw, pitch)
            }
        }
    }

    func stop() {
        guard isTracking else { return }
        motion.stopDeviceMotionUpdates()
        isTracking = false
    }

    /// 把当前手腕姿态设为"头部正中"的基准
    func calibrate() {
        guard let dm = motion.deviceMotion else { return }
        refPitch = dm.attitude.pitch
        refRoll = dm.attitude.roll
    }
}
