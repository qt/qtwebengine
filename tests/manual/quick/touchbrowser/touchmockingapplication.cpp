/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "touchmockingapplication.h"

#include <qpa/qwindowsysteminterface.h>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QTouchDevice>
#include <QtGui/QTouchEvent>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>

static inline bool isTouchEvent(const QEvent* event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        return true;
    default:
        return false;
    }
}

static inline bool isMouseEvent(const QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        return true;
    default:
        return false;
    }
}

TouchMockingApplication::TouchMockingApplication(int& argc, char** argv)
    : QGuiApplication(argc, argv)
    , m_realTouchEventReceived(false)
    , m_pendingFakeTouchEventCount(0)
    , m_holdingControl(false)
{
}

bool TouchMockingApplication::notify(QObject* target, QEvent* event)
{
    // We try to be smart, if we received real touch event, we are probably on a device
    // with touch screen, and we should not have touch mocking.

    if (!event->spontaneous() || m_realTouchEventReceived)
        return QGuiApplication::notify(target, event);

    if (isTouchEvent(event)) {
        if (m_pendingFakeTouchEventCount)
            --m_pendingFakeTouchEventCount;
        else
            m_realTouchEventReceived = true;
        return QGuiApplication::notify(target, event);
    }

    QQuickView* window = qobject_cast<QQuickView*>(target);
    if (!window)
        return QGuiApplication::notify(target, event);

    m_holdingControl = QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

    if (event->type() == QEvent::KeyRelease && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Control) {
        foreach (int id, m_heldTouchPoints)
            if (m_touchPoints.contains(id) && !QGuiApplication::mouseButtons().testFlag(Qt::MouseButton(id))) {
                m_touchPoints[id].setState(Qt::TouchPointReleased);
                m_heldTouchPoints.remove(id);
            } else
                m_touchPoints[id].setState(Qt::TouchPointStationary);

        sendTouchEvent(window, m_heldTouchPoints.isEmpty() ? QEvent::TouchEnd : QEvent::TouchUpdate, static_cast<QKeyEvent*>(event)->timestamp());
    }

    if (isMouseEvent(event)) {
        const QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);

        QTouchEvent::TouchPoint touchPoint;
        touchPoint.setPressure(1);

        QEvent::Type touchType = QEvent::None;

        switch (mouseEvent->type()) {
        case QEvent::MouseButtonPress:
            touchPoint.setId(mouseEvent->button());
            if (m_touchPoints.contains(touchPoint.id())) {
                touchPoint.setState(Qt::TouchPointMoved);
                touchType = QEvent::TouchUpdate;
            } else {
                touchPoint.setState(Qt::TouchPointPressed);
                // Check if more buttons are held down than just the event triggering one.
                if (mouseEvent->buttons() > mouseEvent->button())
                    touchType = QEvent::TouchUpdate;
                else
                    touchType = QEvent::TouchBegin;
            }
            break;
        case QEvent::MouseMove:
            if (!mouseEvent->buttons()) {
                // We have to swallow the event instead of propagating it,
                // since we avoid sending the mouse release events and if the
                // Flickable is the mouse grabber it would receive the event
                // and would move the content.
                event->accept();
                return true;
            }
            touchType = QEvent::TouchUpdate;
            touchPoint.setId(mouseEvent->buttons());
            touchPoint.setState(Qt::TouchPointMoved);
            break;
        case QEvent::MouseButtonRelease:
            // Check if any buttons are still held down after this event.
            if (mouseEvent->buttons())
                touchType = QEvent::TouchUpdate;
            else
                touchType = QEvent::TouchEnd;
            touchPoint.setId(mouseEvent->button());
            touchPoint.setState(Qt::TouchPointReleased);
            break;
        case QEvent::MouseButtonDblClick:
            // Eat double-clicks, their accompanying press event is all we need.
            event->accept();
            return true;
        default:
            Q_ASSERT_X(false, "multi-touch mocking", "unhandled event type");
        }

        // A move can have resulted in multiple buttons, so we need check them individually.
        if (touchPoint.id() & Qt::LeftButton)
            updateTouchPoint(mouseEvent, touchPoint, Qt::LeftButton);
        if (touchPoint.id() & Qt::MiddleButton)
            updateTouchPoint(mouseEvent, touchPoint, Qt::MiddleButton);
        if (touchPoint.id() & Qt::RightButton)
            updateTouchPoint(mouseEvent, touchPoint, Qt::RightButton);

        if (m_holdingControl && touchPoint.state() == Qt::TouchPointReleased) {
            // We avoid sending the release event because the Flickable is
            // listening to mouse events and would start a bounce-back
            // animation if it received a mouse release.
            event->accept();
            return true;
        }

        // Update states for all other touch-points
        for (QHash<int, QTouchEvent::TouchPoint>::iterator it = m_touchPoints.begin(), end = m_touchPoints.end(); it != end; ++it) {
            if (!(it.value().id() & touchPoint.id()))
                it.value().setState(Qt::TouchPointStationary);
        }

        Q_ASSERT(touchType != QEvent::None);

        if (!sendTouchEvent(window, touchType, mouseEvent->timestamp()))
            return QGuiApplication::notify(target, event);

        event->accept();
        return true;
    }

    return QGuiApplication::notify(target, event);
}

void TouchMockingApplication::updateTouchPoint(const QMouseEvent* mouseEvent, QTouchEvent::TouchPoint touchPoint, Qt::MouseButton mouseButton)
{
    // Ignore inserting additional touch points if Ctrl isn't held because it produces
    // inconsistent touch events and results in assers in the gesture recognizers.
    if (!m_holdingControl && m_touchPoints.size() && !m_touchPoints.contains(mouseButton))
        return;

    if (m_holdingControl && touchPoint.state() == Qt::TouchPointReleased) {
        m_heldTouchPoints.insert(mouseButton);
        return;
    }

    // Gesture recognition uses the screen position for the initial threshold
    // but since the canvas translates touch events we actually need to pass
    // the screen position as the scene position to deliver the appropriate
    // coordinates to the target.
    touchPoint.setPos(mouseEvent->localPos());
    touchPoint.setScenePos(mouseEvent->screenPos());

    if (touchPoint.state() == Qt::TouchPointPressed)
        touchPoint.setStartScenePos(mouseEvent->screenPos());
    else {
        const QTouchEvent::TouchPoint& oldTouchPoint = m_touchPoints[mouseButton];
        touchPoint.setStartScenePos(oldTouchPoint.startScenePos());
        touchPoint.setLastPos(oldTouchPoint.pos());
        touchPoint.setLastScenePos(oldTouchPoint.scenePos());
    }

    // Update current touch-point.
    touchPoint.setId(mouseButton);
    m_touchPoints.insert(mouseButton, touchPoint);
}

bool TouchMockingApplication::sendTouchEvent(QQuickView* window, QEvent::Type type, ulong timestamp)
{
    static QTouchDevice* device = 0;
    if (!device) {
        device = new QTouchDevice;
        device->setType(QTouchDevice::TouchScreen);
        QWindowSystemInterface::registerTouchDevice(device);
    }

    m_pendingFakeTouchEventCount++;

    const QList<QTouchEvent::TouchPoint>& currentTouchPoints = m_touchPoints.values();
    Qt::TouchPointStates touchPointStates = Qt::TouchPointState();
    foreach (const QTouchEvent::TouchPoint& touchPoint, currentTouchPoints)
        touchPointStates |= touchPoint.state();

    QTouchEvent event(type, device, Qt::NoModifier, touchPointStates, currentTouchPoints);
    event.setTimestamp(timestamp);
    event.setAccepted(false);

    QGuiApplication::notify(window, &event);

    updateVisualMockTouchPoints(window,m_holdingControl ? currentTouchPoints : QList<QTouchEvent::TouchPoint>());

    // Get rid of touch-points that are no longer valid
    foreach (const QTouchEvent::TouchPoint& touchPoint, currentTouchPoints) {
        if (touchPoint.state() ==  Qt::TouchPointReleased)
            m_touchPoints.remove(touchPoint.id());
    }

    return event.isAccepted();
}

void TouchMockingApplication::updateVisualMockTouchPoints(QQuickView* window,const QList<QTouchEvent::TouchPoint>& touchPoints)
{
    if (touchPoints.isEmpty()) {
        // Hide all touch indicator items.
        foreach (QQuickItem* item, m_activeMockComponents.values())
            item->setProperty("pressed", false);

        return;
    }

    foreach (const QTouchEvent::TouchPoint& touchPoint, touchPoints) {
        QQuickItem* mockTouchPointItem = m_activeMockComponents.value(touchPoint.id());

        if (!mockTouchPointItem) {
            QQmlComponent touchMockPointComponent(window->engine(), QUrl("qrc:///qml/MockTouchPoint.qml"));
            mockTouchPointItem = qobject_cast<QQuickItem*>(touchMockPointComponent.create());
            Q_ASSERT(mockTouchPointItem);
            m_activeMockComponents.insert(touchPoint.id(), mockTouchPointItem);
            mockTouchPointItem->setProperty("pointId", QVariant(touchPoint.id()));
            mockTouchPointItem->setParent(window->rootObject());
            mockTouchPointItem->setParentItem(window->rootObject());
        }

        mockTouchPointItem->setX(touchPoint.pos().x());
        mockTouchPointItem->setY(touchPoint.pos().y());
        mockTouchPointItem->setWidth(touchPoint.ellipseDiameters().width());
        mockTouchPointItem->setHeight(touchPoint.ellipseDiameters().height());
        mockTouchPointItem->setProperty("pressed", QVariant(touchPoint.state() != Qt::TouchPointReleased));
    }
}
