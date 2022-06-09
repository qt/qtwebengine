// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    property bool shouldIgnoreLinkClicks: false
    property bool shouldIgnoreSubFrameRequests: false

    QtObject {
        id: attributes
        property url mainUrl: ""
        property url iframeUrl: ""
        property bool linkClickedNavigationRequested: false
        property bool linkClickedNavigationIgnored: false

        function clear() {
            mainUrl = ""
            iframeUrl = ""
            linkClickedNavigationRequested = false
            linkClickedNavigationIgnored = false
        }
    }

    SignalSpy {
        id: navigationSpy
        target: webEngineView
        signalName: "navigationRequested"
    }

    onNavigationRequested: function(request) {
        if (request.isMainFrame) {
            attributes.mainUrl = request.url
        } else {
            attributes.iframeUrl = request.url
            if (shouldIgnoreSubFrameRequests) {
                request.reject()
            }
        }

        if (request.navigationType === WebEngineNavigationRequest.LinkClickedNavigation) {
            attributes.linkClickedNavigationRequested = true
            if (shouldIgnoreLinkClicks) {
                request.reject()
                attributes.linkClickedNavigationIgnored = true
            }
        }
    }

    TestCase {
        id: testCase
        name: "WebEngineViewNavigationRequested"
        when: windowShown

        function init() {
            attributes.clear()
            navigationSpy.clear()
            shouldIgnoreLinkClicks = false
            shouldIgnoreSubFrameRequests = false
        }

        function test_navigationRequested() {
            // Test if we get notified about main frame and iframe loads
            compare(navigationSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("test-iframe.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(attributes.mainUrl, Qt.resolvedUrl("test-iframe.html"))
            compare(attributes.iframeUrl, Qt.resolvedUrl("test1.html"))
            compare(navigationSpy.count, 2)

            // Test if we get notified about clicked links
            mouseClick(webEngineView, 100, 100)
            verify(webEngineView.waitForLoadSucceeded())
            compare(attributes.mainUrl, Qt.resolvedUrl("test1.html"))
            verify(attributes.linkClickedNavigationRequested)
            compare(navigationSpy.count, 3)
        }

        function test_ignoreLinkClickedRequest() {
            // Test if we can ignore clicked link requests
            compare(navigationSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("test-iframe.html")
            verify(webEngineView.waitForLoadSucceeded())

            shouldIgnoreLinkClicks = true
            mouseClick(webEngineView, 100, 100)
            // We ignored the main frame request, so we should
            // get notified that the load has been stopped.
            verify(webEngineView.waitForLoadStopped())
            verify(!webEngineView.loading)

            compare(navigationSpy.count, 3)
            compare(attributes.mainUrl, Qt.resolvedUrl("test1.html"))
            verify(attributes.linkClickedNavigationRequested)
            verify(attributes.linkClickedNavigationIgnored)
        }

        function test_ignoreSubFrameRequest() {
            // Test if we can ignore sub frame requests
            shouldIgnoreSubFrameRequests = true
            webEngineView.url = Qt.resolvedUrl("test-iframe.html")
            // We ignored the sub frame request, so
            // the main frame load should still succeed.
            verify(webEngineView.waitForLoadSucceeded())

            compare(navigationSpy.count, 2)
            compare(attributes.mainUrl, Qt.resolvedUrl("test-iframe.html"))
            compare(attributes.iframeUrl, Qt.resolvedUrl("test1.html"))
        }
    }
}
