/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "hal.h"
#include "drivers/Si12T/Si12T.h"
#include "board/hal_bridge.h"
#include <mooncake_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstdlib>

static const std::string_view _tag = "HAL-HeadTouch";

// 触摸状态
enum class TouchState { IDLE, TOUCHED, SWIPING };

// 配置参数
struct TouchConfig {
    uint8_t touch_threshold = 1;
    int16_t swipe_threshold = 40;  // 使用百分比，范围-100到100
};

// 触摸数据
struct TouchData {
    uint8_t intensity[3];
    uint32_t timestamp;

    // 计算位置（返回-100到100的整数）
    int16_t get_position() const
    {
        uint16_t total = intensity[0] + intensity[1] + intensity[2];
        if (total == 0) return 0;

        int32_t weighted = intensity[0] * (-100) + intensity[1] * 0 + intensity[2] * 100;
        return static_cast<int16_t>(weighted / total);
    }

    uint8_t get_max_intensity() const
    {
        uint8_t max_val = intensity[0];
        if (intensity[1] > max_val) max_val = intensity[1];
        if (intensity[2] > max_val) max_val = intensity[2];
        return max_val;
    }

    bool is_touched() const
    {
        return get_max_intensity() >= 1;
    }
};

// 手势识别器类
class GestureRecognizer {
public:
    GestureRecognizer() : current_state(TouchState::IDLE), initial_position(0)
    {
    }

    // 更新状态机，返回识别到的手势
    HeadPetGesture update(const TouchData& data)
    {
        HeadPetGesture gesture = HeadPetGesture::None;

        switch (current_state) {
            case TouchState::IDLE:
                if (data.is_touched()) {
                    current_state    = TouchState::TOUCHED;
                    initial_position = data.get_position();
                    gesture          = HeadPetGesture::Press;
                    // mclog::tagInfo(_tag, "Touch detected at position: {}", initial_position);
                }
                break;

            case TouchState::TOUCHED:
                if (!data.is_touched()) {
                    current_state = TouchState::IDLE;
                    gesture       = HeadPetGesture::Release;
                } else {
                    // Check for swipe
                    int16_t current_pos = data.get_position();
                    int16_t delta       = current_pos - initial_position;

                    if (delta > config.swipe_threshold) {
                        current_state = TouchState::SWIPING;
                        gesture       = HeadPetGesture::SwipeForward;
                        // mclog::tagInfo(_tag, "Swipe forward detected, delta: {}", delta);
                    } else if (delta < -config.swipe_threshold) {
                        current_state = TouchState::SWIPING;
                        gesture       = HeadPetGesture::SwipeBackward;
                        // mclog::tagInfo(_tag, "Swipe backward detected, delta: {}", delta);
                    }
                }
                break;

            case TouchState::SWIPING:
                if (!data.is_touched()) {
                    current_state = TouchState::IDLE;
                    gesture       = HeadPetGesture::Release;
                }
                break;
        }

        return gesture;
    }

    void set_config(const TouchConfig& cfg)
    {
        config = cfg;
    }

private:
    TouchConfig config;
    TouchState current_state;
    int16_t initial_position;
};

// 双击窗口：第二次 Press 距上次 Press 落在 [80, 600] ms 内、且头顶按压位置接近
static constexpr uint32_t kDoubleTapMinGapMs       = 80;
static constexpr uint32_t kDoubleTapMaxGapMs       = 600;
static constexpr int16_t  kDoubleTapPositionTol    = 30;

static void _head_touch_update_task(void* param)
{
    mclog::tagInfo(_tag, "start update task");

    si12t_handle_t si12t = (si12t_handle_t)param;
    uint8_t touch_result = 0;
    TouchData data;

    GestureRecognizer recognizer;
    HeadPetGesture gesture;

    uint32_t last_press_tick     = 0;
    int16_t  last_press_position = 0;

    vTaskDelay(pdMS_TO_TICKS(200));

    while (1) {
        // Read data
        si12t_read_touch_result(si12t, &touch_result);
        si12t_parse_touch_result_to(touch_result, data.intensity);
        data.timestamp = xTaskGetTickCount();

        // Update and fire event
        gesture = recognizer.update(data);
        if (gesture != HeadPetGesture::None) {
            GetHAL().onHeadPetGesture.emit(gesture);

            if (gesture == HeadPetGesture::Press) {
                uint32_t now        = xTaskGetTickCount();
                int16_t  pos        = data.get_position();
                uint32_t elapsed_ms = pdTICKS_TO_MS(now - last_press_tick);
                int16_t  pos_delta  = std::abs(pos - last_press_position);

                if (last_press_tick != 0
                    && elapsed_ms >= kDoubleTapMinGapMs
                    && elapsed_ms <= kDoubleTapMaxGapMs
                    && pos_delta  <= kDoubleTapPositionTol) {
                    mclog::tagInfo(_tag, "DoubleTap detected (gap={}ms, dpos={})", elapsed_ms, pos_delta);
                    GetHAL().onHeadPetGesture.emit(HeadPetGesture::DoubleTap);
                    last_press_tick = 0;  // 重置，避免连续三次按压被识别为两次双击
                } else {
                    last_press_tick     = now;
                    last_press_position = pos;
                }
            } else if (gesture == HeadPetGesture::SwipeForward
                    || gesture == HeadPetGesture::SwipeBackward) {
                last_press_tick = 0;  // 滑动手势打断双击序列
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void Hal::head_touch_init()
{
    mclog::tagInfo(_tag, "init");

    auto i2c_bus = hal_bridge::board_get_i2c_bus();

    si12t_config_t si12t_cfg = {
        .i2c_bus  = i2c_bus,
        .dev_addr = SI12T_GND_ADDRESS,
    };
    static si12t_handle_t si12t;
    si12t_init(&si12t_cfg, &si12t);
    si12t_setup(si12t, SI12T_TYPE_LOW, SI12T_SENSITIVITY_LEVEL_3);

    // xTaskCreateWithCaps(_head_touch_update_task, "headtouch", 1024 * 6, si12t, 2, NULL, MALLOC_CAP_SPIRAM);
    xTaskCreatePinnedToCoreWithCaps(_head_touch_update_task, "headtouch", 1024 * 6, si12t, 2, NULL, 1,
                                    MALLOC_CAP_SPIRAM);
}
