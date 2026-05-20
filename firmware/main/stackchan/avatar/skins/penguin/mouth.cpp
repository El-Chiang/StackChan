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
    // Speech amplitude toggles between the emotion's (closed, open) pair.
    int next = (weight >= 50) ? _open_variant : _closed_variant;
    if (next != _variant) {
        _variant = next;
        applyVariant();
    }
}

void PenguinMouth::setEmotion(const Emotion& emotion)
{
    if (getIgnoreEmotion()) return;
    // Mouth atlas: 0=neutral 1=happy 2=joy 3=angry 4=sad 5=doubt 6=surprise 7=excited
    // Each emotion picks a (closed, open) pair so setWeight can sync with speech.
    switch (emotion) {
        case Emotion::Neutral: _closed_variant = 0; _open_variant = 1; break;  // neutral / happy
        case Emotion::Happy:   _closed_variant = 0; _open_variant = 1; break;  // neutral / happy
        case Emotion::Angry:   _closed_variant = 5; _open_variant = 6; break;  // doubt / surprise
        case Emotion::Sad:     _closed_variant = 4; _open_variant = 6; break;  // sad / surprise
        case Emotion::Doubt:   _closed_variant = 5; _open_variant = 6; break;  // doubt / surprise
        case Emotion::Sleepy:  _closed_variant = 0; _open_variant = 0; break;  // neutral only
        default:               _closed_variant = 0; _open_variant = 1; break;
    }
    int next = (getWeight() >= 50) ? _open_variant : _closed_variant;
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
