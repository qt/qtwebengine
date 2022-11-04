// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_EVENT_FACTORY_H
#define WEB_EVENT_FACTORY_H

#include "QtGui/qtguiglobal.h"

#include "content/public/browser/native_web_keyboard_event.h"
#if QT_CONFIG(gestures)
#include "third_party/blink/public/common/input/web_gesture_event.h"
#endif
#include "third_party/blink/public/common/input/web_mouse_event.h"
#include "third_party/blink/public/common/input/web_mouse_wheel_event.h"

QT_BEGIN_NAMESPACE
class QEvent;
class QHoverEvent;
class QKeyEvent;
class QMouseEvent;
#if QT_CONFIG(tabletevent)
class QTabletEvent;
#endif
class QWheelEvent;
#if QT_CONFIG(gestures)
class QNativeGestureEvent;
#endif
QT_END_NAMESPACE

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegate;

class WebEventFactory {

public:
    static blink::WebMouseEvent toWebMouseEvent(QMouseEvent *);
    static blink::WebMouseEvent toWebMouseEvent(QHoverEvent *);
#if QT_CONFIG(tabletevent)
    static blink::WebMouseEvent toWebMouseEvent(QTabletEvent *);
#endif
    static blink::WebMouseEvent toWebMouseEvent(QEvent *);
#if QT_CONFIG(gestures)
    static blink::WebGestureEvent toWebGestureEvent(QNativeGestureEvent *);
#endif
    static blink::WebMouseWheelEvent toWebWheelEvent(QWheelEvent *);
    static bool coalesceWebWheelEvent(blink::WebMouseWheelEvent &, QWheelEvent *);
    static void sendUnhandledWheelEvent(const blink::WebGestureEvent &, RenderWidgetHostViewQtDelegate *);
    static content::NativeWebKeyboardEvent toWebKeyboardEvent(QKeyEvent*);
    static bool getEditCommand(QKeyEvent *event, std::string *editCommand);
};

} // namespace QtWebEngineCore

#endif // WEB_EVENT_FACTORY_H
