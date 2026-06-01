//
//  ContentView.swift
//  表盘 UI:连接状态 + 跟随开关 + 校准归中
//

import SwiftUI

struct ContentView: View {
    @StateObject private var ble = StackChanController()
    @StateObject private var tracker = WristTracker()

    var body: some View {
        VStack(spacing: 10) {
            // 连接状态
            HStack(spacing: 6) {
                Circle()
                    .fill(ble.state == .connected ? .green : .orange)
                    .frame(width: 10, height: 10)
                Text(ble.state.rawValue)
                    .font(.footnote)
            }

            // 跟随开关
            Button {
                if tracker.isTracking {
                    tracker.stop()
                } else {
                    tracker.calibrate()   // 开始前先以当前姿态归中
                    tracker.start()
                }
            } label: {
                Label(tracker.isTracking ? "停止跟随" : "开始跟随",
                      systemImage: tracker.isTracking ? "stop.circle.fill" : "play.circle.fill")
            }
            .tint(tracker.isTracking ? .red : .green)
            .disabled(ble.state != .connected)

            // 跟随中显示当前角度 + 重新归中
            if tracker.isTracking {
                Text("yaw \(ble.lastSentYaw)  pitch \(ble.lastSentPitch)")
                    .font(.system(.caption2, design: .monospaced))
                    .foregroundStyle(.secondary)
                Button("归中") { tracker.calibrate() }
                    .font(.caption)
            }
        }
        .padding()
        .onAppear {
            // 把姿态角度桥接到 BLE 发送
            tracker.onAngles = { [weak ble] yaw, pitch in
                ble?.sendAngles(yaw: yaw, pitch: pitch)
            }
        }
    }
}

#Preview {
    ContentView()
}
