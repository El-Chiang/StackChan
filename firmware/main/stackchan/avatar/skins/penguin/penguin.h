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

// Layout constants in the 320x240 LVGL panel, mirroring the Moddable
// PenguinFace atlas (firmware/mods/penguin-face/penguin-atlas.js).
//
// face_region occupies a 240x240 area centered horizontally with 40px
// black bars on either side. All sprite positions (cx,cy) are centers
// relative to the face_region top-left.
struct PenguinLayout {
    static constexpr int PANEL_W   = 320;
    static constexpr int PANEL_H   = 240;
    static constexpr int FACE_LEFT = 40;
    static constexpr int FACE_TOP  = 0;
    static constexpr int FACE_W    = 240;
    static constexpr int FACE_H    = 240;

    static constexpr int EYE_CY       = 126;
    static constexpr int EYE_H        = 36;
    static constexpr int LEFT_EYE_CX  = 85;
    static constexpr int LEFT_EYE_W   = 50;
    static constexpr int RIGHT_EYE_CX = 144;
    static constexpr int RIGHT_EYE_W  = 53;

    static constexpr int MOUTH_CX = 120;
    static constexpr int MOUTH_CY = 160;
    static constexpr int MOUTH_W  = 56;
    static constexpr int MOUTH_H  = 32;
};

class PenguinAvatar : public Avatar {
public:
    void init(lv_obj_t* parent, const lv_font_t* font = &lv_font_montserrat_16);
    uitk::lvgl_cpp::Container* getPanel() const;

private:
    std::unique_ptr<uitk::lvgl_cpp::Container> _panel;
    std::unique_ptr<uitk::lvgl_cpp::Container> _face_region;
    std::unique_ptr<uitk::lvgl_cpp::Image> _base;
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
    void applyVariant();

    bool _is_left_eye    = false;
    int _variant         = 0;  // index into the eye sprite atlas (see eyes.cpp setEmotion)
    bool _emotion_locked = false;
    std::unique_ptr<uitk::lvgl_cpp::Image> _image;
};

class PenguinMouth : public Feature {
public:
    PenguinMouth(lv_obj_t* parent);
    ~PenguinMouth();

    void setPosition(const uitk::Vector2i& position) override;
    void setWeight(int weight) override;
    void setRotation(int rotation) override;
    void setEmotion(const Emotion& emotion) override;
    void setVisible(bool visible) override;

private:
    void applyVariant();

    int _variant         = 0;  // index into the mouth sprite atlas (see mouth.cpp setEmotion)
    // setEmotion sets a (closed, open) pair from the atlas; setWeight toggles
    // between them based on speech amplitude so the mouth opens/closes in sync
    // even while an emotion is active.
    int _closed_variant  = 0;
    int _open_variant    = 1;
    std::unique_ptr<uitk::lvgl_cpp::Image> _image;
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
