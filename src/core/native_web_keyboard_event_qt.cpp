// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "content/public/browser/native_web_keyboard_event.h"
#include <QKeyEvent>

namespace {

// We need to copy |os_event| in NativeWebKeyboardEvent because it is
// queued in RenderWidgetHost and may be passed and used
// RenderViewHostDelegate::HandledKeybardEvent after the original aura
// event is destroyed.
gfx::NativeEvent CopyEvent(gfx::NativeEvent event)
{
    if (!event)
        return nullptr;

    QKeyEvent *keyEvent = reinterpret_cast<QKeyEvent *>(event);
    return reinterpret_cast<gfx::NativeEvent>(keyEvent->clone());
}

void DestroyEvent(gfx::NativeEvent event)
{
    delete reinterpret_cast<QKeyEvent*>(event);
}

}  // namespace

using blink::WebKeyboardEvent;

namespace content {

NativeWebKeyboardEvent::NativeWebKeyboardEvent(const blink::WebKeyboardEvent &web_event, gfx::NativeView)
    : WebKeyboardEvent(web_event)
    , os_event(nullptr)
    , skip_in_browser(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(blink::WebInputEvent::Type type, int modifiers,
                                               base::TimeTicks timestamp)
    : WebKeyboardEvent(type, modifiers, timestamp)
    , os_event(nullptr)
    , skip_in_browser(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(gfx::NativeEvent native_event)
    : os_event(CopyEvent(native_event))
    , skip_in_browser(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(const NativeWebKeyboardEvent& other)
    : WebKeyboardEvent(other)
    , os_event(CopyEvent(other.os_event))
    , skip_in_browser(other.skip_in_browser)
{
}

NativeWebKeyboardEvent &NativeWebKeyboardEvent::operator=(const NativeWebKeyboardEvent &other)
{
    if (this == &other)
        return *this;
    WebKeyboardEvent::operator=(other);
    DestroyEvent(os_event);
    os_event = CopyEvent(other.os_event);
    skip_in_browser = other.skip_in_browser;
    return *this;
}

NativeWebKeyboardEvent::~NativeWebKeyboardEvent() {
    DestroyEvent(os_event);
}

}  // namespace content
