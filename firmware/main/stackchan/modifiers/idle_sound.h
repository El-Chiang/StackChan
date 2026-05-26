/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "../modifiable.h"
#include "../utils/random.h"
#include <hal/hal.h>
#include <hal/board/hal_bridge.h>
#include <assets/assets.h>
#include <cstdint>

namespace stackchan {

/**
 * @brief Agent 待机时不定时“自言自语”——随机间隔播放咕咕嘎嘎。
 *        仅在待机状态挂载，离开待机/进入 sleepy 时被移除。
 */
class IdleSoundModifier : public Modifier {
public:
    IdleSoundModifier(uint32_t interval_min = 20000, uint32_t interval_max = 60000)
        : _interval_min(interval_min), _interval_max(interval_max)
    {
        // 首次延迟也走随机区间，避免一进待机立刻出声
        _next_tick = GetHAL().millis() + Random::getInstance().getInt(_interval_min, _interval_max);
    }

    void _update(Modifiable& stackchan) override
    {
        uint32_t now = GetHAL().millis();
        if (now < _next_tick) {
            return;
        }

        hal_bridge::app_play_sound(OGG_GUGU_GAGA);

        _next_tick = now + Random::getInstance().getInt(_interval_min, _interval_max);
    }

private:
    uint32_t _interval_min;
    uint32_t _interval_max;
    uint32_t _next_tick = 0;
};

}  // namespace stackchan
