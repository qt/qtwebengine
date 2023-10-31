// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef NATIVE_WEB_KEYBOARD_EVENT_QT_H
#define NATIVE_WEB_KEYBOARD_EVENT_QT_H

#include <QtCore/qglobal.h>

#include "content/public/common/input/native_web_keyboard_event.h"

QT_FORWARD_DECLARE_CLASS(QKeyEvent)

namespace QtWebEngineCore {

gfx::NativeEvent ToNativeEvent(QKeyEvent *keyEvent);
QKeyEvent *ToKeyEvent(gfx::NativeEvent event);

} // namespace QtWebEngineCore

#endif // NATIVE_WEB_KEYBOARD_EVENT_QT_H
