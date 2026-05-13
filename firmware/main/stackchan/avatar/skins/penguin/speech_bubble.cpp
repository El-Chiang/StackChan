/*
 * SPDX-FileCopyrightText: 2026 El-Chiang
 *
 * SPDX-License-Identifier: MIT
 */
#include "penguin.h"

using namespace uitk;
using namespace uitk::lvgl_cpp;
using namespace stackchan::avatar;

PenguinSpeechBubble::PenguinSpeechBubble(lv_obj_t* parent, const lv_font_t* font)
{
    _container = std::make_unique<Container>(parent);
    _container->setAlign(LV_ALIGN_CENTER);
    _container->setBorderWidth(0);
    _container->setBgOpa(0);
    _container->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
    _container->setSize(1, 1);

    _text = std::make_unique<Label>(_container->get());
    _text->setTextFont(font);
    _text->setText("");

    clearSpeech();
}

PenguinSpeechBubble::~PenguinSpeechBubble()
{
    _text.reset();
    _container.reset();
}

void PenguinSpeechBubble::setSpeech(std::string_view text)
{
    if (text.empty()) {
        clearSpeech();
        return;
    }
    _text->setText(text);
    setVisible(true);
}

void PenguinSpeechBubble::clearSpeech()
{
    _text->setText("");
    setVisible(false);
}

void PenguinSpeechBubble::setVisible(bool visible)
{
    SpeechBubble::setVisible(visible);
    _container->setHidden(!visible);
}

void PenguinSpeechBubble::setTextFont(void* font)
{
    if (_text && font) {
        _text->setTextFont((lv_font_t*)font);
    }
}
