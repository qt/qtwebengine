// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "touchmockingapplication.h"

#include <QCursor>
#include <QEvent>
#include <QPixmap>

#if defined(QUICK_TOUCHBROWSER)
#    include <QQuickView>
#endif

#if defined(WIDGET_TOUCHBROWSER)
#    include <QMainWindow>
#endif

static inline bool isMouseEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
        return true;
    default:
        return false;
    }
}

TouchMockingApplication::TouchMockingApplication(int &argc, char **argv)
    : Application(argc, argv), m_touchPoint(new QCursor(QPixmap(":touchpoint.png")))
{
}

TouchMockingApplication::~TouchMockingApplication()
{
    delete m_touchPoint;
}

bool TouchMockingApplication::notify(QObject *target, QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
        setOverrideCursor(*m_touchPoint);
        break;
    case QEvent::TouchEnd:
        restoreCursor();
        break;
    default:
        break;
    }

// All mouse events that are not accepted by the application will be translated to touch events
// instead (see Qt::AA_SynthesizeTouchForUnhandledMouseEvents).
#if defined(QUICK_TOUCHBROWSER)
    if (isMouseEvent(event) && qobject_cast<QQuickView *>(target)) {
        event->ignore();
        return false;
    }
#elif defined(WIDGET_TOUCHBROWSER)
    // Popups ignore touch evenets so we send MouseEvents directly.
    if (isMouseEvent(event)) {
        if (activePopupWidget()) {
            restoreCursor();
        } else {
            event->ignore();
            return false;
        }
    }
#endif
    return Application::notify(target, event);
}

void TouchMockingApplication::restoreCursor()
{
    while (overrideCursor())
        restoreOverrideCursor();
}
