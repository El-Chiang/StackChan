/*
 * SPDX-FileCopyrightText: 2026 El-Chiang
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "../../avatar/avatar.h"
#include "../../avatar/elements/feature.h"
#include <lvgl.h>
#include <smooth_lvgl.hpp>
#include <memory>

namespace stackchan::avatar {

class PenguinAvatar : public Avatar {
public:
    void init(lv_obj_t* parent, const lv_font_t* font = &lv_font_montserrat_16);
    uitk::lvgl_cpp::Container* getPanel() const;

private:
    std::unique_ptr<uitk::lvgl_cpp::Container> _panel;
};

class PenguinEyes : public Feature {
public:
    PenguinEyes(lv_obj_t* parent, bool isLeftEye);
    ~PenguinEyes();

    void setPosition(const uitk::Vector2i& position) override;
    void setWeight(int weight) override;
    void setRotation(int rotation) override;
    void setEmotion(const Emotion& emotion) override;
    void setVisible(bool visible) override;
    void setSize(int size) override;

private:
    bool _is_left_eye = false;
    std::unique_ptr<uitk::lvgl_cpp::Container> _container;
};

class PenguinMouth : public Feature {
public:
    PenguinMouth(lv_obj_t* parent);
    ~PenguinMouth();

    void setPosition(const uitk::Vector2i& position) override;
    void setWeight(int weight) override;
    void setRotation(int rotation) override;
    void setVisible(bool visible) override;

private:
    std::unique_ptr<uitk::lvgl_cpp::Container> _container;
};

class PenguinSpeechBubble : public SpeechBubble {
public:
    PenguinSpeechBubble(lv_obj_t* parent, const lv_font_t* font);
    ~PenguinSpeechBubble();

    void setSpeech(std::string_view text) override;
    void clearSpeech() override;
    void setVisible(bool visible) override;
    void setTextFont(void* font) override;

private:
    std::unique_ptr<uitk::lvgl_cpp::Container> _container;
    std::unique_ptr<uitk::lvgl_cpp::Label> _text;
};

}  // namespace stackchan::avatar
