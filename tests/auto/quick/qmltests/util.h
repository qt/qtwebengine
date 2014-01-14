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

// Functions and macros that really need to be in QTestLib

#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>

class QQuickWebEngineView;
class QWebEngineLoadRequest;

#if !defined(TESTS_SOURCE_DIR)
#define TESTS_SOURCE_DIR ""
#endif

void addQtWebProcessToPath();
bool waitForSignal(QObject*, const char* signal, int timeout = 10000);
void suppressDebugOutput();

#if defined(HAVE_QTQUICK) && HAVE_QTQUICK
bool waitForLoadSucceeded(QQuickWebEngineView* webEngineView, int timeout = 10000);
bool waitForLoadFailed(QQuickWebEngineView* webEngineView, int timeout = 10000);
bool waitForViewportReady(QQuickWebEngineView* webEngineView, int timeout = 10000);

class LoadSpy : public QEventLoop {
    Q_OBJECT
public:
    LoadSpy(QQuickWebEngineView* webEngineView);
Q_SIGNALS:
    void loadSucceeded();
    void loadFailed();
private Q_SLOTS:
    void onLoadingStateChanged(QWebEngineLoadRequest* loadRequest);
};

class LoadStartedCatcher : public QObject {
    Q_OBJECT
public:
    LoadStartedCatcher(QQuickWebEngineView* webEngineView);
    virtual ~LoadStartedCatcher() { }
public Q_SLOTS:
    void onLoadingStateChanged(QWebEngineLoadRequest* loadRequest);
Q_SIGNALS:
    void finished();
private:
    QQuickWebEngineView* m_webEngineView;
};
#endif
