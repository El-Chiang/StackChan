/*
 * SPDX-FileCopyrightText: 2026 El-Chiang
 *
 * SPDX-License-Identifier: MIT
 */
#include "penguin.h"

using namespace uitk;
using namespace uitk::lvgl_cpp;
using namespace stackchan::avatar;

LV_IMAGE_DECLARE(penguin_mouth);

PenguinMouth::PenguinMouth(lv_obj_t* parent)
{
    _image = std::make_unique<Image>(parent);
    _image->setSrc(&penguin_mouth);
    lv_image_set_inner_align(_image->get(), LV_IMAGE_ALIGN_TOP_LEFT);
    _image->setPos(PenguinLayout::MOUTH_CX - PenguinLayout::MOUTH_W / 2,
                   PenguinLayout::MOUTH_CY - PenguinLayout::MOUTH_H / 2);
    _image->setSize(PenguinLayout::MOUTH_W, PenguinLayout::MOUTH_H);

    applyVariant();
}

PenguinMouth::~PenguinMouth()
{
    _image.reset();
}

void PenguinMouth::applyVariant()
{
    if (!_image) return;
    lv_image_set_offset_x(_image->get(), -PenguinLayout::MOUTH_W * _variant);
}

void PenguinMouth::setPosition(const Vector2i& position)
{
    Element::setPosition(position);
}

void PenguinMouth::setWeight(int weight)
{
    Feature::setWeight(weight);
    // weight 0 = closed (smile_small), 100 = wide open (smile_wide).
    int next = (weight >= 50) ? 1 : 0;
    if (next != _variant) {
        _variant = next;
        applyVariant();
    }
}

void PenguinMouth::setRotation(int rotation)
{
    Element::setRotation(rotation);
}

void PenguinMouth::setVisible(bool visible)
{
    Element::setVisible(visible);
    if (_image) _image->setHidden(!visible);
}
