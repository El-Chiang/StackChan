# Apple Watch 提线木偶遥控 StackChan

用 Apple Watch 的手腕姿态实时控制 StackChan 的头部转动(yaw/pitch)。
手表作为 **BLE central** 直连固件里的 NimBLE 外设,写 Motion 特征——
**固件无需任何改动**。

适用:Apple Watch Series 8 / SE 及以前(无官方双击手势,改用 CoreMotion 姿态)。

---

## 工作原理

```
手腕姿态 (CMMotionManager.attitude)
   pitch ──► 头部 pitch 舵机
   roll  ──► 头部 yaw   舵机          ← 用重力参考的 pitch/roll,不漂移
        │
        ▼  50Hz 采样 + 死区 + 单次在途节流
BLE Write With Response
   特征 e2e5e5e1-...  {"yawServo":{"angle":N},"pitchServo":{"angle":N}}
        │
        ▼
固件 updateMotionFromJson() ──► Servo.move()(默认弹簧,自动平滑)
```

### 与固件对齐的关键事实(已核对源码)

| 项 | 值 | 出处 |
|---|---|---|
| 广播名 | `StackChan` | `bleprph.c:698` |
| 服务 UUID | `e2e5e5e0-1234-5678-1234-56789abcdef0` | `bleprph.h:43` |
| Motion 特征 | `e2e5e5e1-1234-5678-1234-56789abcdef0` | `bleprph.h:51` |
| 写入类型 | 仅 Write **With** Response | `gatt_svr.c:91`(无 `WRITE_NO_RSP`) |
| 角度单位 | degx10 | `hal_servo.cpp` |
| yaw 范围 | `-1280 ~ 1280`(±128°) | `hal_servo.cpp:174` |
| pitch 范围 | `30 ~ 870`(3°~87°,中位 ~450) | `hal_servo.cpp:183` |
| JSON 字段 | `yawServo` / `pitchServo` 下 `angle`(省略 speed 走默认弹簧) | `json_helper.cpp:130` |

---

## 在 Xcode 里跑起来

这些是纯 Swift 源码,需要你在 Mac 的 Xcode 里建工程编译、装到手表。

1. Xcode → File → New → Project → **watchOS → App**,产品名随意(如 `StackChanPuppet`),
   Interface 选 **SwiftUI**,Language 选 **Swift**。
2. 把 `Sources/` 下 4 个 `.swift` 文件拖进 Watch App target(删掉模板自带的 `ContentView.swift` / `*App.swift` 以免重名)。
3. 在 Watch App target 的 **Info**(或 Info.plist)里加一条:
   - Key:`NSBluetoothAlwaysUsageDescription`
   - Value:`用于连接并控制 StackChan`
   （没有这条会在 `CBCentralManager` 初始化时崩。）
4. 用数据线 + 配对的 iPhone 把 App 装到手表(BLE 不能在模拟器测,必须真机)。
5. StackChan 上电、确保在 BLE 广播(没被别的 App 占用连接)。打开手表 App →
   状态变"已连接" → 点"开始跟随",转手腕即可。

> 手表 App 在前台时 BLE 才稳定;锁屏/退后台会被 watchOS 挂起,这是平台限制。
> 想后台常驻就得走"手表→iPhone→BLE"两段式(另说)。

---

## 调参(都在 `WristTracker.swift`)

| 参数 | 作用 |
|---|---|
| `pitchGain` / `yawGain` | 灵敏度,越大头转得越多。头转反了把对应 gain 改成**负数** |
| `pitchCenter` | pitch 中位,默认 450(=45°) |
| `deadband`(在 `StackChanController.swift`) | 死区,默认 20(=2°),越大越省流量但越"顿" |

- 开始跟随会自动以当前手腕姿态"归中";手酸了换姿势后点"归中"重设基准。
- 用 `roll`(手腕左右翻)控 yaw 是因为它和 pitch 一样有重力参考、不漂移;
  若你更想用"手臂左右摆"控 yaw,可改读 `attitude.yaw`,但需接受航向漂移。

---

## 可选:让跟随更跟手(改 1 行固件)

当前 Motion 特征只支持 Write With Response,每帧要等 ESP32 ACK 才能发下一帧,
实际约 15~30Hz。若想更低延迟,可在固件给特征加上 `WRITE_NO_RSP`:

```c
// firmware/main/hal/utils/bleprph/gatt_svr.c:91  (Motion 特征)
.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE
       | BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_NOTIFY,
```

然后把 `StackChanController.swift` 里的写入类型改成 `.withoutResponse`,
并配合 `peripheral.canSendWriteWithoutResponse` 做发送门控。
属于锦上添花,先用默认版本验证体验再决定。
