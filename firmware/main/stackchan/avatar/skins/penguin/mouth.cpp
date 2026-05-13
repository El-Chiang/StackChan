/*
 * SPDX-FileCopyrightText: 2026 El-Chiang
 *
 * SPDX-License-Identifier: MIT
 */
#include "penguin.h"

using namespace uitk;
using namespace uitk::lvgl_cpp;
using namespace stackchan::avatar;

PenguinMouth::PenguinMouth(lv_obj_t* parent)
{
    _container = std::make_unique<Container>(parent);
    _container->setAlign(LV_ALIGN_CENTER);
    _container->setBorderWidth(0);
    _container->setBgOpa(0);
    _container->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
    _container->setSize(1, 1);
}

PenguinMouth::~PenguinMouth()
{
    _container.reset();
}

void PenguinMouth::setPosition(const Vector2i& position)
{
    Element::setPosition(position);
}

void PenguinMouth::setWeight(int weight)
{
    Feature::setWeight(weight);
}

void PenguinMouth::setRotation(int rotation)
{
    Element::setRotation(rotation);
}

void PenguinMouth::setVisible(bool visible)
{
    Element::setVisible(visible);
    _container->setHidden(!visible);
}
