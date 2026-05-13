/*
 * SPDX-FileCopyrightText: 2026 El-Chiang
 *
 * SPDX-License-Identifier: MIT
 */
#include "penguin.h"

using namespace uitk::lvgl_cpp;
using namespace stackchan::avatar;

void PenguinAvatar::init(lv_obj_t* parent, const lv_font_t* font)
{
    _panel = std::make_unique<Container>(parent);
    _panel->align(LV_ALIGN_CENTER, 0, 0);
    _panel->setSize(320, 240);
    _panel->setRadius(0);
    _panel->setBorderWidth(0);
    _panel->setBgColor(lv_color_black());
    _panel->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

    _key_elements.leftEye      = std::make_unique<PenguinEyes>(_panel->get(), true);
    _key_elements.rightEye     = std::make_unique<PenguinEyes>(_panel->get(), false);
    _key_elements.mouth        = std::make_unique<PenguinMouth>(_panel->get());
    _key_elements.speechBubble = std::make_unique<PenguinSpeechBubble>(_panel->get(), font);
}

Container* PenguinAvatar::getPanel() const
{
    return _panel ? _panel.get() : nullptr;
}
