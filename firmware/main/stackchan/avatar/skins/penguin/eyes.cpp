/*
 * SPDX-FileCopyrightText: 2026 El-Chiang
 *
 * SPDX-License-Identifier: MIT
 */
#include "penguin.h"

using namespace uitk;
using namespace uitk::lvgl_cpp;
using namespace stackchan::avatar;

LV_IMAGE_DECLARE(penguin_eyelid_left);
LV_IMAGE_DECLARE(penguin_eyelid_right);

PenguinEyes::PenguinEyes(lv_obj_t* parent, bool isLeftEye)
{
    _is_left_eye = isLeftEye;

    _image = std::make_unique<Image>(parent);
    _image->setSrc(_is_left_eye ? (const void*)&penguin_eyelid_left : (const void*)&penguin_eyelid_right);
    lv_image_set_inner_align(_image->get(), LV_IMAGE_ALIGN_TOP_LEFT);

    int cx = _is_left_eye ? PenguinLayout::LEFT_EYE_CX : PenguinLayout::RIGHT_EYE_CX;
    int cy = PenguinLayout::LEFT_EYE_CY;
    _image->setPos(cx - PenguinLayout::EYE_W / 2, cy - PenguinLayout::EYE_H / 2);
    _image->setSize(PenguinLayout::EYE_W, PenguinLayout::EYE_H);

    applyVariant();
}

PenguinEyes::~PenguinEyes()
{
    _image.reset();
}

void PenguinEyes::applyVariant()
{
    if (!_image) return;
    // offset_x is the image's position offset within the widget; negative
    // pulls the image left so the next variant cell scrolls into view.
    lv_image_set_offset_x(_image->get(), -PenguinLayout::EYE_W * _variant);
}

void PenguinEyes::setPosition(const Vector2i& position)
{
    Element::setPosition(position);
    // Stage 1: position offset (eye gaze) not yet wired through. The atlas
    // sprites are static frames so eye-direction can't be expressed without
    // either new sprites or pixel-translating the sheet.
}

void PenguinEyes::setWeight(int weight)
{
    Feature::setWeight(weight);
    // weight 0 = fully closed eyelids, weight 100 = fully open. The atlas
    // only ships two variants so anything below 50 looks closed.
    int next = (weight >= 50) ? 0 : 1;
    if (next != _variant) {
        _variant = next;
        applyVariant();
    }
}

void PenguinEyes::setRotation(int rotation)
{
    Element::setRotation(rotation);
}

void PenguinEyes::setEmotion(const Emotion& emotion)
{
    if (getIgnoreEmotion()) return;
    // Approximate emotions with the two-variant atlas until dedicated
    // sprites are added.
    switch (emotion) {
        case Emotion::Sleepy:
            setWeight(20);  // closed
            break;
        case Emotion::Happy:
            setWeight(40);  // half-closed (squint)
            break;
        default:
            setWeight(100);  // open
            break;
    }
}

void PenguinEyes::setVisible(bool visible)
{
    Element::setVisible(visible);
    if (_image) _image->setHidden(!visible);
}

void PenguinEyes::setSize(int size)
{
    Feature::setSize(size);
    // Stage 1: no size variants in the atlas.
}
