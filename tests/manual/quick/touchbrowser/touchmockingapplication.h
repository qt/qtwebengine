// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TOUCHMOCKINGAPPLICATION_H
#define TOUCHMOCKINGAPPLICATION_H

#include <QtGui/QGuiApplication>
#include <QtGui/private/qeventpoint_p.h>
#include <QtGui/QEventPoint>

#include <private/qevent_p.h>

QT_BEGIN_NAMESPACE
class QQuickView;
class QQuickItem;
QT_END_NAMESPACE

class TouchMockingApplication : public QGuiApplication
{
    Q_OBJECT

public:
    TouchMockingApplication(int &argc, char **argv);

    virtual bool notify(QObject *, QEvent *) override;

private:
    void updateTouchPoint(const QMouseEvent *, QEventPoint, Qt::MouseButton);
    bool sendTouchEvent(QQuickView *, QEvent::Type, ulong timestamp);
    void updateVisualMockTouchPoints(QQuickView *,const QList<QEventPoint> &touchPoints);

private:
    bool m_realTouchEventReceived;
    int m_pendingFakeTouchEventCount;

    QHash<int, QEventPoint> m_touchPoints;
    QSet<int> m_heldTouchPoints;
    QHash<int, QQuickItem*> m_activeMockComponents;

    bool m_holdingControl;
};

#endif // TOUCHMOCKINGAPPLICATION_H
