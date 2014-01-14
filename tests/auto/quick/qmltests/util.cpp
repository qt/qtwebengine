/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "util.h"

#include <QtTest/QtTest>
#include <stdio.h>

#if defined(HAVE_QTQUICK) && HAVE_QTQUICK
#include "private/qquickwebengineview_p.h"
#include "private/qwebengineloadrequest_p.h"
#endif

void addQtWebProcessToPath()
{
    // Since tests won't find ./QtWebEngineProcess, add it to PATH (at the end to prevent surprises).
    // QWP_PATH should be defined by qmake.
    qputenv("PATH", qgetenv("PATH") + ":" + QWP_PATH);
}

/**
 * Starts an event loop that runs until the given signal is received.
 * Optionally the event loop
 * can return earlier on a timeout.
 *
 * \return \p true if the requested signal was received
 *         \p false on timeout
 */
bool waitForSignal(QObject* obj, const char* signal, int timeout)
{
    QEventLoop loop;
    QObject::connect(obj, signal, &loop, SLOT(quit()));
    QTimer timer;
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
    if (timeout > 0) {
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(timeout);
    }
    loop.exec();
    return timeoutSpy.isEmpty();
}

static void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& message)
{
    if (type == QtCriticalMsg) {
        fprintf(stderr, "%s\n", qPrintable(message));
        return;
    }
    // Do nothing
}

void suppressDebugOutput()
{
    qInstallMessageHandler(messageHandler); \
    if (qgetenv("QT_WEBENGINE_SUPPRESS_WEB_PROCESS_OUTPUT").isEmpty()) \
        qputenv("QT_WEBENGINE_SUPPRESS_WEB_PROCESS_OUTPUT", "1");
}

#if defined(HAVE_QTQUICK) && HAVE_QTQUICK
bool waitForLoadSucceeded(QQuickWebEngineView* webEngineView, int timeout)
{
    QEventLoop loop;
    LoadSpy loadSpy(webEngineView);
    QObject::connect(&loadSpy, SIGNAL(loadSucceeded()), &loop, SLOT(quit()));
    QTimer timer;
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
    if (timeout > 0) {
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(timeout);
    }
    loop.exec();
    return timeoutSpy.isEmpty();
}

bool waitForLoadFailed(QQuickWebEngineView* webEngineView, int timeout)
{
    QEventLoop loop;
    LoadSpy loadSpy(webEngineView);
    QObject::connect(&loadSpy, SIGNAL(loadFailed()), &loop, SLOT(quit()));
    QTimer timer;
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
    if (timeout > 0) {
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(timeout);
    }
    loop.exec();
    return timeoutSpy.isEmpty();
}

bool waitForViewportReady(QQuickWebEngineView* webEngineView, int timeout)
{
    // The viewport is locked until the first frame of a page load is rendered.
    // The QQuickView needs to be shown for this to succeed.
    return waitForSignal(webEngineView->experimental(), SIGNAL(loadVisuallyCommitted()), timeout);
}

LoadSpy::LoadSpy(QQuickWebEngineView* webEngineView)
{
    qDebug("LoadSpy::LoadSpy");
    connect(webEngineView, SIGNAL(loadingStateChanged(QWebEngineLoadRequest*)), this, SLOT(onLoadingStateChanged(QWebEngineLoadRequest*)));
}

void LoadSpy::onLoadingStateChanged(QWebEngineLoadRequest* loadRequest)
{
    qDebug("LoadSpy::onLoadingChanged");
    if (loadRequest->status() == QQuickWebEngineView::LoadSucceededStatus)
        emit loadSucceeded();
    else if (loadRequest->status() == QQuickWebEngineView::LoadFailedStatus)
        emit loadFailed();
}

LoadStartedCatcher::LoadStartedCatcher(QQuickWebEngineView* webEngineView)
    : m_webEngineView(webEngineView)
{
    connect(m_webEngineView, SIGNAL(loadingChanged(QWebEngineLoadRequest*)), this, SLOT(onLoadingChanged(QWebEngineLoadRequest*)));
}

void LoadStartedCatcher::onLoadingChanged(QWebEngineLoadRequest* loadRequest)
{
    if (loadRequest->status() == QQuickWebEngineView::LoadStartedStatus) {
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);

        QCOMPARE(m_webEngineView->loading(), true);
    }
}
#endif
