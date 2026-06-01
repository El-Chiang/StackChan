// BugC2 最小可动 demo:前进 1 秒 -> 停 2 秒,循环
// 砍自官方 examples/driver/driver.ino,只保留让电机动起来的最小逻辑。
#include "M5HatBugC.h"

M5HatBugC bugc;

void setup() {
    Serial.begin(115200);
    // StickC HAT 口的 I2C: SDA=G0, SCL=G26, 地址 0x38
    while (!bugc.begin(&Wire, BUGC_DEFAULT_I2C_ADDR, 0, 26, 400000U)) {
        Serial.println("Couldn't find BugC");  // 多半是 BugC2 电源没开
        delay(1000);
    }
    bugc.setAllMotorSpeed(0, 0, 0, 0);
}

void loop() {
    bugc.move(MOVE_FORWARD, 50);  // 前进,速度 0~127
    delay(1000);
    bugc.move(MOVE_STOP);
    delay(2000);
}
