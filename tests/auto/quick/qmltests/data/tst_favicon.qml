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
import QtWebEngine 1.3
import QtWebEngine.testsupport 1.0

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    testSupport: WebEngineTestSupport {
        property var errorPageLoadStatus: null

        function waitForErrorPageLoadSucceeded() {
            var success = _waitFor(function() { return testSupport.errorPageLoadStatus == WebEngineView.LoadSucceededStatus })
            testSupport.errorPageLoadStatus = null
            return success
        }

        errorPage.onLoadingChanged: {
            errorPageLoadStatus = loadRequest.status
        }
    }

    SignalSpy {
        id: iconChangedSpy
        target: webEngineView
        signalName: "iconChanged"
    }

    TestCase {
        id: test
        name: "WebEngineFavicon"
        when: windowShown

        function init() {
            if (webEngineView.icon != '') {
                // If this is not the first test, then load a blank page without favicon, restoring the initial state.
                webEngineView.url = 'about:blank'
                verify(webEngineView.waitForLoadSucceeded())
                iconChangedSpy.wait()
            }

            iconChangedSpy.clear()
        }

        function test_noFavicon() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("test1.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_aboutBlank() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("about:blank")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_unavailableFavicon() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-unavailable.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl("icons/unavailable.ico"))
        }

        function test_errorPageEnabled() {
            skip("Error page does not work properly: QTBUG-48995")
            WebEngine.settings.errorPageEnabled = true

            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("http://non.existent/url")
            webEngineView.url = url
            verify(webEngineView.testSupport.waitForErrorPageLoadSucceeded())

            iconChangedSpy.wait()
            // Icon is reseted at load start.
            // Load is started twice: once for unavailale page then error page
            compare(iconChangedSpy.count, 2)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_errorPageDisabled() {
            WebEngine.settings.errorPageEnabled = false

            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("http://non.existent/url")
            webEngineView.url = url
            verify(webEngineView.waitForLoadFailed())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_touchIcon() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-touch.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }
    }
}
