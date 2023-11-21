// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginemessagepumpscheduler_p.h"

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QTimerEvent>

QWebEngineMessagePumpScheduler::QWebEngineMessagePumpScheduler(std::function<void()> callback)
    : m_callback(std::move(callback))
{}

void QWebEngineMessagePumpScheduler::scheduleImmediateWork()
{
    QCoreApplication::postEvent(this, new QTimerEvent(0), Qt::NormalEventPriority);
}

void QWebEngineMessagePumpScheduler::scheduleIdleWork()
{
    QCoreApplication::postEvent(this, new QTimerEvent(0), Qt::LowEventPriority);
}

void QWebEngineMessagePumpScheduler::scheduleDelayedWork(int delay)
{
    if (delay < 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    } else if (!m_timerId || delay < QAbstractEventDispatcher::instance()->remainingTime(m_timerId)) {
        killTimer(m_timerId);
        m_timerId = startTimer(delay);
    }
}

void QWebEngineMessagePumpScheduler::timerEvent(QTimerEvent *ev)
{
    Q_ASSERT(!ev->timerId() || m_timerId == ev->timerId());
    killTimer(m_timerId);
    m_timerId = 0;
    m_callback();
}

#include "moc_qwebenginemessagepumpscheduler_p.cpp"
