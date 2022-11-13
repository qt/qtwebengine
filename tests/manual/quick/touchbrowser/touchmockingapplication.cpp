// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "touchmockingapplication.h"

#include <qpa/qwindowsysteminterface.h>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QInputDevice>

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
                QMutableEventPoint::setState(m_touchPoints[id], QEventPoint::Released);
                m_heldTouchPoints.remove(id);
            } else
                QMutableEventPoint::setState(m_touchPoints[id], QEventPoint::Stationary);

        sendTouchEvent(window, m_heldTouchPoints.isEmpty() ? QEvent::TouchEnd : QEvent::TouchUpdate, static_cast<QKeyEvent*>(event)->timestamp());
    }

    if (isMouseEvent(event)) {
        const QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);

        QEventPoint touchPoint;
        QMutableEventPoint::setPressure(touchPoint, 1);

        QEvent::Type touchType = QEvent::None;

        switch (mouseEvent->type()) {
        case QEvent::MouseButtonPress:
            QMutableEventPoint::setId(touchPoint, mouseEvent->button());
            if (m_touchPoints.contains(touchPoint.id())) {
                QMutableEventPoint::setState(touchPoint, QEventPoint::Updated);
                touchType = QEvent::TouchUpdate;
            } else {
                QMutableEventPoint::setState(touchPoint, QEventPoint::Pressed);
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
            QMutableEventPoint::setId(touchPoint, mouseEvent->buttons());
            QMutableEventPoint::setState(touchPoint, QEventPoint::Updated);
            break;
        case QEvent::MouseButtonRelease:
            // Check if any buttons are still held down after this event.
            if (mouseEvent->buttons())
                touchType = QEvent::TouchUpdate;
            else
                touchType = QEvent::TouchEnd;
            QMutableEventPoint::setId(touchPoint, mouseEvent->button());
            QMutableEventPoint::setState(touchPoint, QEventPoint::Released);
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

        if (m_holdingControl && touchPoint.state() == QEventPoint::Released) {
            // We avoid sending the release event because the Flickable is
            // listening to mouse events and would start a bounce-back
            // animation if it received a mouse release.
            event->accept();
            return true;
        }

        // Update states for all other touch-points
        for (QHash<int, QEventPoint>::iterator it = m_touchPoints.begin(), end = m_touchPoints.end(); it != end; ++it) {
            if (!(it.value().id() & touchPoint.id()))
                QMutableEventPoint::setState(it.value(), QEventPoint::Stationary);
        }

        Q_ASSERT(touchType != QEvent::None);

        if (!sendTouchEvent(window, touchType, mouseEvent->timestamp()))
            return QGuiApplication::notify(target, event);

        event->accept();
        return true;
    }

    return QGuiApplication::notify(target, event);
}

void TouchMockingApplication::updateTouchPoint(const QMouseEvent* mouseEvent, QEventPoint touchPoint, Qt::MouseButton mouseButton)
{
    // Ignore inserting additional touch points if Ctrl isn't held because it produces
    // inconsistent touch events and results in assers in the gesture recognizers.
    if (!m_holdingControl && m_touchPoints.size() && !m_touchPoints.contains(mouseButton))
        return;

    if (m_holdingControl && touchPoint.state() == QEventPoint::Released) {
        m_heldTouchPoints.insert(mouseButton);
        return;
    }

    // Gesture recognition uses the screen position for the initial threshold
    // but since the canvas translates touch events we actually need to pass
    // the screen position as the scene position to deliver the appropriate
    // coordinates to the target.
    QMutableEventPoint::setPosition(touchPoint, mouseEvent->position());
    QMutableEventPoint::setScenePosition(touchPoint, mouseEvent->globalPosition());

    if (touchPoint.state() == QEventPoint::Pressed)
        QMutableEventPoint::setScenePosition(touchPoint, mouseEvent->scenePosition());
    else {
        const QEventPoint& oldTouchPoint = m_touchPoints[mouseButton];
        QMutableEventPoint::setGlobalLastPosition(touchPoint, oldTouchPoint.globalPosition());
    }

    // Update current touch-point.
    QMutableEventPoint::setId(touchPoint, mouseButton);
    m_touchPoints.insert(mouseButton, touchPoint);
}

bool TouchMockingApplication::sendTouchEvent(QQuickView* window, QEvent::Type type, ulong timestamp)
{
    static QPointingDevice* device = 0;
    if (!device) {
        device = new QPointingDevice(QStringLiteral("MockTouchDevice"), 1,
                                     QPointingDevice::DeviceType::TouchScreen,
                                     QPointingDevice::PointerType::AllPointerTypes,
                                     QInputDevice::Capability::All, 3, 3,
                                     QString(), QPointingDeviceUniqueId(), window->rootObject());

        QWindowSystemInterface::registerInputDevice(device);
    }

    m_pendingFakeTouchEventCount++;

    const QList<QEventPoint>& currentTouchPoints = m_touchPoints.values();
    QEventPoint::States touchPointStates = QEventPoint::States();
    foreach (const QEventPoint& touchPoint, currentTouchPoints)
        touchPointStates |= touchPoint.state();

    QTouchEvent event(type, device, Qt::NoModifier, currentTouchPoints);
    event.setTimestamp(timestamp);
    event.setAccepted(false);

    QGuiApplication::notify(window, &event);

    updateVisualMockTouchPoints(window, m_holdingControl ? currentTouchPoints : QList<QEventPoint>());

    // Get rid of touch-points that are no longer valid
    foreach (const QEventPoint& touchPoint, currentTouchPoints) {
        if (touchPoint.state() == QEventPoint::Released)
            m_touchPoints.remove(touchPoint.id());
    }

    return event.isAccepted();
}

void TouchMockingApplication::updateVisualMockTouchPoints(QQuickView* window,const QList<QEventPoint>& touchPoints)
{
    if (touchPoints.isEmpty()) {
        // Hide all touch indicator items.
        foreach (QQuickItem* item, m_activeMockComponents.values())
            item->setProperty("pressed", false);

        return;
    }

    foreach (const QEventPoint& touchPoint, touchPoints) {
        QQuickItem* mockTouchPointItem = m_activeMockComponents.value(touchPoint.id());

        if (!mockTouchPointItem) {
            QQmlComponent touchMockPointComponent(window->engine(), QUrl("qrc:///MockTouchPoint.qml"));
            mockTouchPointItem = qobject_cast<QQuickItem*>(touchMockPointComponent.create());
            Q_ASSERT(mockTouchPointItem);
            m_activeMockComponents.insert(touchPoint.id(), mockTouchPointItem);
            mockTouchPointItem->setProperty("pointId", QVariant(touchPoint.id()));
            mockTouchPointItem->setParent(window->rootObject());
            mockTouchPointItem->setParentItem(window->rootObject());
        }

        mockTouchPointItem->setX(touchPoint.position().x());
        mockTouchPointItem->setY(touchPoint.position().y());
        mockTouchPointItem->setWidth(touchPoint.ellipseDiameters().width());
        mockTouchPointItem->setHeight(touchPoint.ellipseDiameters().height());
        mockTouchPointItem->setProperty("pressed", QVariant(touchPoint.state() != QEventPoint::Released));
    }
}
