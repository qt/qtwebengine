/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
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

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    property bool watchProgress: false
    property int numLoadStarted: 0
    property int numLoadSucceeded: 0

    onLoadProgressChanged: {
        if (watchProgress && webEngineView.loadProgress != 100) {
            watchProgress = false
            url = ''
        }
    }

    onLoadingChanged: {
        if (loadRequest.status == WebEngineView.LoadStartedStatus)
            ++numLoadStarted
        if (loadRequest.status == WebEngineView.LoadSucceededStatus)
            ++numLoadSucceeded
    }

    TestCase {
        name: "WebEngineViewLoadUrl"

        function test_loadIgnoreEmptyUrl() {
            var url = Qt.resolvedUrl("test1.html")

            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())
            compare(numLoadStarted, 1)
            compare(numLoadSucceeded, 1)
            compare(webEngineView.url, url)

            var lastUrl = webEngineView.url
            webEngineView.url = ''
            wait(1000)
            compare(numLoadStarted, 1)
            compare(numLoadSucceeded, 1)
            compare(webEngineView.url, lastUrl)

            webEngineView.url = 'about:blank'
            verify(webEngineView.waitForLoadSucceeded())
            compare(numLoadStarted, 2)
            compare(numLoadSucceeded, 2)
            compare(webEngineView.url, 'about:blank')

            // It shouldn't interrupt any ongoing load when an empty url is used.
            watchProgress = true
            webEngineView.url = url
            webEngineView.waitForLoadSucceeded()
            compare(numLoadStarted, 3)
            compare(numLoadSucceeded, 3)
            verify(!watchProgress)
            compare(webEngineView.url, url)
        }

        function test_stopStatus() {
            var url = Qt.resolvedUrl("test1.html")

            webEngineView.loadingChanged.connect(function(loadRequest) {
                if (loadRequest.status == WebEngineView.LoadStoppedStatus) {
                    compare(webEngineView.url, url)
                    compare(loadRequest.url, url)
                }
            })

            webEngineView.url = url
            compare(webEngineView.url, url)
            webEngineView.stop()
            verify(webEngineView.waitForLoadStopped())
            compare(webEngineView.url, url)
        }
    }
}
