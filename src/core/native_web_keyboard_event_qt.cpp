// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include <QtCore/qglobal.h>

#if !defined(Q_OS_MACOS)
#include "native_web_keyboard_event_qt.h"

#include <QtGui/QKeyEvent>

namespace QtWebEngineCore {
gfx::NativeEvent ToNativeEvent(QKeyEvent *keyEvent)
{
    return reinterpret_cast<gfx::NativeEvent>(keyEvent);
}

QKeyEvent *ToKeyEvent(gfx::NativeEvent nativeEvent)
{
    return reinterpret_cast<QKeyEvent *>(nativeEvent);
}
} // namespace QtWebEngineCore

namespace {
// We need to copy |os_event| in NativeWebKeyboardEvent because it is
// queued in RenderWidgetHost and may be passed and used
// RenderViewHostDelegate::HandledKeybardEvent after the original aura
// event is destroyed.
gfx::NativeEvent CopyEvent(gfx::NativeEvent nativeEvent)
{
    if (!nativeEvent)
        return nullptr;

    QKeyEvent *keyEvent = QtWebEngineCore::ToKeyEvent(nativeEvent);
    return QtWebEngineCore::ToNativeEvent(keyEvent->clone());
}

void DestroyEvent(gfx::NativeEvent nativeEvent)
{
    delete QtWebEngineCore::ToKeyEvent(nativeEvent);
}
} // namespace


namespace content {

NativeWebKeyboardEvent::NativeWebKeyboardEvent(const blink::WebKeyboardEvent &web_event, gfx::NativeView)
    : blink::WebKeyboardEvent(web_event)
    , os_event(nullptr)
    , skip_if_unhandled(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(blink::WebInputEvent::Type type, int modifiers,
                                               base::TimeTicks timestamp)
    : blink::WebKeyboardEvent(type, modifiers, timestamp)
    , os_event(nullptr)
    , skip_if_unhandled(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(gfx::NativeEvent native_event)
    : os_event(CopyEvent(native_event))
    , skip_if_unhandled(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(const NativeWebKeyboardEvent& other)
    : blink::WebKeyboardEvent(other)
    , os_event(CopyEvent(other.os_event))
    , skip_if_unhandled(other.skip_if_unhandled)
{
}

NativeWebKeyboardEvent &NativeWebKeyboardEvent::operator=(const NativeWebKeyboardEvent &other)
{
    if (this == &other)
        return *this;
    blink::WebKeyboardEvent::operator=(other);
    DestroyEvent(os_event);
    os_event = CopyEvent(other.os_event);
    skip_if_unhandled = other.skip_if_unhandled;
    return *this;
}

NativeWebKeyboardEvent::~NativeWebKeyboardEvent() {
    DestroyEvent(os_event);
}

}  // namespace content
#endif // !defined(Q_OS_MACOS)
