// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEMESSAGEPUMPSCHEDULER_P_H
#define QWEBENGINEMESSAGEPUMPSCHEDULER_P_H

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

#include "qtwebenginecoreglobal_p.h"

#include <QtCore/qobject.h>

#include <functional>

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineMessagePumpScheduler : public QObject
{
    Q_OBJECT
public:
    QWebEngineMessagePumpScheduler(std::function<void()> callback);
    void scheduleImmediateWork();
    void scheduleIdleWork();
    void scheduleDelayedWork(int delay);

protected:
    void timerEvent(QTimerEvent *ev) override;

private:
    int m_timerId = 0;
    std::function<void()> m_callback;
};

QT_END_NAMESPACE

#endif // !QWEBENGINEMESSAGEPUMPSCHEDULER_P_H
