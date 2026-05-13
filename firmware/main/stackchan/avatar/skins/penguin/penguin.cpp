/*
 * SPDX-FileCopyrightText: 2026 El-Chiang
 *
 * SPDX-License-Identifier: MIT
 */
#include "penguin.h"

using namespace uitk::lvgl_cpp;
using namespace stackchan::avatar;

LV_IMAGE_DECLARE(penguin_base);

void PenguinAvatar::init(lv_obj_t* parent, const lv_font_t* font)
{
    _panel = std::make_unique<Container>(parent);
    _panel->align(LV_ALIGN_CENTER, 0, 0);
    _panel->setSize(PenguinLayout::PANEL_W, PenguinLayout::PANEL_H);
    _panel->setRadius(0);
    _panel->setBorderWidth(0);
    _panel->setBgColor(lv_color_black());
    _panel->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

    _face_region = std::make_unique<Container>(_panel->get());
    _face_region->setPos(PenguinLayout::FACE_LEFT, PenguinLayout::FACE_TOP);
    _face_region->setSize(PenguinLayout::FACE_W, PenguinLayout::FACE_H);
    _face_region->setRadius(0);
    _face_region->setBorderWidth(0);
    _face_region->setBgOpa(0);
    _face_region->setPaddingAll(0);
    _face_region->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(_face_region->get(), LV_LAYOUT_NONE);

    _base = std::make_unique<Image>(_face_region->get());
    _base->setSrc(&penguin_base);
    lv_image_set_inner_align(_base->get(), LV_IMAGE_ALIGN_TOP_LEFT);
    _base->setPos(0, 0);
    _base->setSize(PenguinLayout::FACE_W, PenguinLayout::FACE_H);

    _key_elements.leftEye      = std::make_unique<PenguinEyes>(_face_region->get(), true);
    _key_elements.rightEye     = std::make_unique<PenguinEyes>(_face_region->get(), false);
    _key_elements.mouth        = std::make_unique<PenguinMouth>(_face_region->get());
    _key_elements.speechBubble = std::make_unique<PenguinSpeechBubble>(_panel->get(), font);
}

Container* PenguinAvatar::getPanel() const
{
    return _panel ? _panel.get() : nullptr;
}
