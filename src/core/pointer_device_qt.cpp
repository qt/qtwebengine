// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on chromium/ui/base/pointer/pointer_device_linux.cc
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/pointer/pointer_device.h"

#include <algorithm>
#include <QPointingDevice>

namespace ui {

namespace {

bool IsTouchScreen(const QInputDevice *device)
{
    return device->type() & QPointingDevice::DeviceType::TouchScreen;
}

bool IsMouseOrTouchpad(const QInputDevice *device)
{
    return device->type()
            & (QPointingDevice::DeviceType::TouchPad | QPointingDevice::DeviceType::Mouse);
}

bool IsTouchDevicePresent()
{
    QList<const QInputDevice *> devices = QInputDevice::devices();
    return std::any_of(devices.constBegin(), devices.constEnd(), IsTouchScreen);
}

bool IsMouseOrTouchpadPresent()
{
    QList<const QInputDevice *> devices = QInputDevice::devices();
    return std::any_of(devices.constBegin(), devices.constEnd(), IsMouseOrTouchpad);
}

} // namespace

int GetAvailablePointerTypes()
{
    int available_pointer_types = POINTER_TYPE_NONE;
    if (IsMouseOrTouchpadPresent())
        available_pointer_types |= POINTER_TYPE_FINE;

    if (IsTouchDevicePresent())
        available_pointer_types |= POINTER_TYPE_COARSE;

    return available_pointer_types;
}

int GetAvailableHoverTypes()
{
    if (IsMouseOrTouchpadPresent())
        return HOVER_TYPE_HOVER;

    return HOVER_TYPE_NONE;
}

TouchScreensAvailability GetTouchScreensAvailability()
{
    if (!IsTouchDevicePresent())
        return TouchScreensAvailability::NONE;

    return TouchScreensAvailability::ENABLED;
}

int MaxTouchPoints()
{
    int max_touch = 0;
    for (const auto *device : QInputDevice::devices()) {
        if (IsTouchScreen(device)) {
            int points = static_cast<const QPointingDevice *>(device)->maximumPoints();
            max_touch = points > max_touch ? points : max_touch;
        }
    }

    return max_touch;
}

PointerType GetPrimaryPointerType(int available_pointer_types)
{
    if (available_pointer_types & POINTER_TYPE_FINE)
        return POINTER_TYPE_FINE;
    if (available_pointer_types & POINTER_TYPE_COARSE)
        return POINTER_TYPE_COARSE;
    Q_ASSERT(available_pointer_types == POINTER_TYPE_NONE);
    return POINTER_TYPE_NONE;
}

HoverType GetPrimaryHoverType(int available_hover_types)
{
    if (available_hover_types & HOVER_TYPE_HOVER)
        return HOVER_TYPE_HOVER;
    Q_ASSERT(available_hover_types == HOVER_TYPE_NONE);
    return HOVER_TYPE_NONE;
}

} // namespace ui
