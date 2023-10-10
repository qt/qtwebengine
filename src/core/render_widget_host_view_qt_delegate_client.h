// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_CLIENT_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_CLIENT_H

#include "compositor/compositor.h"

#include <QMap>
#include <QtGui/QCursor>
#include <QtGui/QTouchEvent>

QT_BEGIN_NAMESPACE
class QEvent;
class QVariant;
class QMouseEvent;
class QKeyEvent;
class QTabletEvent;
class QNativeGestureEvent;
class QHoverEvent;
class QFocusEvent;
class QInputMethodEvent;
class QInputMethodQueryEvent;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class RenderWidgetHostViewQt;

struct MultipleMouseClickHelper
{
    QPoint lastPressPosition;
    Qt::MouseButton lastPressButton = Qt::NoButton;
    int clickCounter = 0;
    ulong lastPressTimestamp = 0;
};

class Q_WEBENGINECORE_PRIVATE_EXPORT RenderWidgetHostViewQtDelegateClient
{
public:
    RenderWidgetHostViewQtDelegateClient(RenderWidgetHostViewQt *rwhv);

    Compositor::Id compositorId();
    void notifyShown();
    void notifyHidden();
    void visualPropertiesChanged();
    bool forwardEvent(QEvent *);
    QVariant inputMethodQuery(Qt::InputMethodQuery query);
    void closePopup();

private:
    friend class RenderWidgetHostViewQt;

    template<class T>
    void handlePointerEvent(T *);
    void handleMouseEvent(QMouseEvent *);
    void handleKeyEvent(QKeyEvent *);
    void handleTouchEvent(QTouchEvent *);
#if QT_CONFIG(tabletevent)
    void handleTabletEvent(QTabletEvent *);
#endif
#if QT_CONFIG(gestures)
    void handleGestureEvent(QNativeGestureEvent *);
#endif
    void handleHoverEvent(QHoverEvent *);
    void handleFocusEvent(QFocusEvent *);
    void handleInputMethodEvent(QInputMethodEvent *);
    void handleInputMethodQueryEvent(QInputMethodQueryEvent *);

    QRect viewRectInDips() const { return m_viewRectInDips; }
    QRect windowRectInDips() const { return m_windowRectInDips; }

    // Mouse
    void resetPreviousMousePosition() { m_previousMousePosition = QCursor::pos(); }

    // Touch
    void clearPreviousTouchMotionState();

    // IME
    void selectionChanged();
    void setCursorPosition(uint pos) { m_cursorPosition = pos; }
    void setSurroundingText(const QString &text) { m_surroundingText = text; }
    bool isPreviousSelectionEmpty() const { return m_emptyPreviousSelection; }

    RenderWidgetHostViewQt *m_rwhv;

    // Mouse
    uint m_mouseButtonPressed = 0;
    QPoint m_previousMousePosition;
    MultipleMouseClickHelper m_clickHelper;

    // Key
    std::string m_editCommand;

    // Touch
    typedef QPair<int, QTouchEvent::TouchPoint> TouchPoint;
    QList<TouchPoint> mapTouchPointIds(const QList<QTouchEvent::TouchPoint> &input);
    QMap<int, int> m_touchIdMapping;
    QList<TouchPoint> m_previousTouchPoints;
    bool m_touchMotionStarted = false;
    bool m_sendMotionActionDown = false;
    int64_t m_eventsToNowDelta = 0; // delta for first touch in microseconds

    // IME
    bool m_receivedEmptyImeEvent = false;
    bool m_imeInProgress = false;
    bool m_emptyPreviousSelection = true;
    uint m_cursorPosition = 0;
    int m_anchorPositionWithinSelection = -1;
    int m_cursorPositionWithinSelection = -1;
    QString m_surroundingText;

    // Geometry of the view in screen DIPs.
    QRect m_viewRectInDips;
    // Geometry of the window, including frame, in screen DIPs.
    QRect m_windowRectInDips;
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_CLIENT_H
