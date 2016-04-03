/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.2

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    property var lastUrl
    property bool watchProgress: false
    property int numLoadStarted: 0
    property int numLoadSucceeded: 0

    focus: true

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
        when: windowShown

        function test_loadIgnoreEmptyUrl() {
            var url = Qt.resolvedUrl("test1.html")

            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())
            compare(numLoadStarted, 1)
            compare(numLoadSucceeded, 1)
            compare(webEngineView.url, url)

            lastUrl = webEngineView.url
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

        function test_urlProperty() {
            WebEngine.settings.errorPageEnabled = false

            var url = Qt.resolvedUrl("test1.html")

            webEngineView.url = url
            compare(webEngineView.url, url)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, url)

            var bogusSite = "http://www.somesitethatdoesnotexist.abc/"
            webEngineView.url = bogusSite
            compare(webEngineView.url, bogusSite)
            verify(webEngineView.waitForLoadFailed())
            compare(webEngineView.url, url)

            webEngineView.url = "about:blank" // Reset from previous test
            verify(webEngineView.waitForLoadSucceeded())

            var handleLoadFailed = function(loadRequest) {
                if (loadRequest.status == WebEngineView.LoadFailedStatus) {
                    webEngineView.loadHtml("load failed", bogusSite)
                    // Since the load did not succeed the active url is the
                    // url of the previous successful load.
                    compare(webEngineView.url, "about:blank")
                    compare(loadRequest.url, bogusSite)
                }
            }
            webEngineView.loadingChanged.connect(handleLoadFailed)
            webEngineView.url = bogusSite
            compare(webEngineView.url, bogusSite)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, bogusSite)
            webEngineView.loadingChanged.disconnect(handleLoadFailed)

            var dataUrl = "data:text/html,foo"
            webEngineView.url = dataUrl
            compare(webEngineView.url, dataUrl)
            verify(webEngineView.waitForLoadSucceeded()) // data:text/html,foo is loaded
            compare(webEngineView.url, dataUrl)

            var redirectUrl = Qt.resolvedUrl("redirect.html")
            webEngineView.url = redirectUrl
            compare(webEngineView.url, redirectUrl)
            verify(webEngineView.waitForLoadSucceeded()) // redirect.html is loaded
            compare(webEngineView.url, redirectUrl)
            verify(webEngineView.waitForLoadSucceeded()) // test1.html is loaded
            compare(webEngineView.url, url)

            var linkUrl = Qt.resolvedUrl("link.html")
            webEngineView.url = linkUrl
            compare(webEngineView.url, linkUrl)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, linkUrl)

            var handleLoadRequest = function(loadRequest) {
                if (loadRequest.status == WebEngineView.LoadStartedStatus) {
                    compare(webEngineView.url, lastUrl)
                    compare(loadRequest.url, url)
                }
                if (loadRequest.status == WebEngineView.LoadSuceededStatus) {
                    compare(webEngineView.url, loadRequest.url)
                    compare(webEngineView.url, url)
                }
            }
            lastUrl = webEngineView.url
            webEngineView.loadingChanged.connect(handleLoadRequest)
            mouseClick(webEngineView, 10, 10, Qt.LeftButton, Qt.NoModifiers, 50)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, url)
            webEngineView.loadingChanged.disconnect(handleLoadRequest)
        }

        function test_stopStatus() {
            var url = Qt.resolvedUrl("test1.html")

            var handleLoadRequest = function(loadRequest) {
                if (loadRequest.status == WebEngineView.LoadStoppedStatus) {
                    compare(webEngineView.url, url)
                    compare(loadRequest.url, url)
                }
            }
            webEngineView.loadingChanged.connect(handleLoadRequest)
            webEngineView.url = url
            compare(webEngineView.url, url)
            webEngineView.stop()
            verify(webEngineView.waitForLoadStopped())
            compare(webEngineView.url, url)
            webEngineView.loadingChanged.disconnect(handleLoadRequest)
        }
    }
}
