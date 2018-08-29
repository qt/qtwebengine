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

#ifndef TOUCHMOCKINGAPPLICATION_H
#define TOUCHMOCKINGAPPLICATION_H

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QUrl>
#include <QtGui/QGuiApplication>
#include <QtGui/QTouchEvent>

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
    void updateTouchPoint(const QMouseEvent *, QTouchEvent::TouchPoint, Qt::MouseButton);
    bool sendTouchEvent(QQuickView *, QEvent::Type, ulong timestamp);
    void updateVisualMockTouchPoints(QQuickView *,const QList<QTouchEvent::TouchPoint> &touchPoints);

private:
    bool m_realTouchEventReceived;
    int m_pendingFakeTouchEventCount;

    QPointF m_lastPos;
    QPointF m_lastScreenPos;
    QPointF m_startScreenPos;

    QHash<int, QTouchEvent::TouchPoint> m_touchPoints;
    QSet<int> m_heldTouchPoints;
    QHash<int, QQuickItem*> m_activeMockComponents;

    bool m_holdingControl;
};

#endif // TOUCHMOCKINGAPPLICATION_H
