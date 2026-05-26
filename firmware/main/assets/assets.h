/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <lvgl.h>
#include <string_view>

LV_FONT_DECLARE(MontserratSemiBold26);

extern const char ogg_camera_shutter_start[] asm("_binary_camera_shutter_ogg_start");
extern const char ogg_camera_shutter_end[] asm("_binary_camera_shutter_ogg_end");
static const std::string_view OGG_CAMERA_SHUTTER{
    static_cast<const char*>(ogg_camera_shutter_start),
    static_cast<size_t>(ogg_camera_shutter_end - ogg_camera_shutter_start)};

extern const char ogg_new_notification_start[] asm("_binary_new_notification_ogg_start");
extern const char ogg_new_notification_end[] asm("_binary_new_notification_ogg_end");
static const std::string_view OGG_NEW_NOTIFICATION{
    static_cast<const char*>(ogg_new_notification_start),
    static_cast<size_t>(ogg_new_notification_end - ogg_new_notification_start)};

extern const char ogg_gugu_gaga_start[] asm("_binary_gugu_gaga_ogg_start");
extern const char ogg_gugu_gaga_end[] asm("_binary_gugu_gaga_ogg_end");
static const std::string_view OGG_GUGU_GAGA{
    static_cast<const char*>(ogg_gugu_gaga_start),
    static_cast<size_t>(ogg_gugu_gaga_end - ogg_gugu_gaga_start)};

extern const char ogg_en_question_start[] asm("_binary_en_question_ogg_start");
extern const char ogg_en_question_end[] asm("_binary_en_question_ogg_end");
static const std::string_view OGG_EN_QUESTION{
    static_cast<const char*>(ogg_en_question_start),
    static_cast<size_t>(ogg_en_question_end - ogg_en_question_start)};

namespace assets {

lv_image_dsc_t get_image(std::string_view name);

}  // namespace assets
