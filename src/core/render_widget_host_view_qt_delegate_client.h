/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
