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
    int w  = _is_left_eye ? PenguinLayout::LEFT_EYE_W  : PenguinLayout::RIGHT_EYE_W;
    int cy = PenguinLayout::EYE_CY;
    _image->setPos(cx - w / 2, cy - PenguinLayout::EYE_H / 2);
    _image->setSize(w, PenguinLayout::EYE_H);

    applyVariant();
    // Seed _weight so BlinkModifier's first cycle captures 100 (open) as the
    // rest weight to restore on each blink. Without this it captures the
    // default 0 and the eyes stay closed forever after the first blink.
    setWeight(100);
}

PenguinEyes::~PenguinEyes()
{
    _image.reset();
}

void PenguinEyes::applyVariant()
{
    if (!_image) return;
    int w = _is_left_eye ? PenguinLayout::LEFT_EYE_W : PenguinLayout::RIGHT_EYE_W;
    lv_image_set_offset_x(_image->get(), -w * _variant);
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
    // While an emotion is active the variant is owned by setEmotion — blink
    // animations only drive the atlas when emotion is Neutral / not set.
    if (_emotion_locked) return;
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
    // Variant indexes match the eye atlas built by tools/penguin-atlas:
    // 0=open  1=closed  2=happy  3=angry  4=sad  5=doubt  6=surprise
    int next = 0;
    switch (emotion) {
        case Emotion::Neutral: next = 0; break;
        case Emotion::Sleepy:  next = 1; break;
        case Emotion::Happy:   next = 2; break;
        case Emotion::Angry:   next = 3; break;
        case Emotion::Sad:     next = 4; break;
        case Emotion::Doubt:   next = 5; break;
        default:               next = 0; break;
    }
    // Neutral releases the lock so blink animations can drive the eye again.
    _emotion_locked = (emotion != Emotion::Neutral);
    if (next != _variant) {
        _variant = next;
        applyVariant();
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
