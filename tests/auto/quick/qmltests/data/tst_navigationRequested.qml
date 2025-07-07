// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    property bool shouldIgnoreLinkClicks: false
    property bool shouldIgnoreSubFrameRequests: false
    property var navigationRequests: []

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
        navigationRequests.push({
            "url": request.url,
            "navigationType": request.navigationType,
            "userInitiated": request.userInitiated,
            "isMainFrame": request.isMainFrame
        })

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
            navigationRequests = []
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

        // Programmatic URL set is browser-initiated, hence user-initiated
        function test_userInitiatedProgrammaticLoad() {
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationSpy.count, 1)
            // Browser-initiated navigations (e.g. setting url from C++/QML)
            // are treated as user-initiated
            const nav = navigationRequests[0]
            compare(nav.userInitiated, true)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.TypedNavigation)
        }

        // User clicking a link is user-initiated
        function test_userInitiatedLinkClick() {
            webEngineView.url = Qt.resolvedUrl("nav-user-initiated.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            const center = getElementCenter("link")
            mouseClick(webEngineView, center.x, center.y)
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, true)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.LinkClickedNavigation)
        }

        // Programmatic .click() on a link triggered by a real user gesture
        // is still user-initiated (user activation propagates)
        function test_userInitiatedJsClickLink() {
            webEngineView.url = Qt.resolvedUrl("nav-user-initiated.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            const center = getElementCenter("jsClick")
            mouseClick(webEngineView, center.x, center.y)
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, true)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.LinkClickedNavigation)
        }

        // Programmatic .click() without user gesture is not user-initiated
        function test_userInitiatedJsClickLinkNoGesture() {
            webEngineView.url = Qt.resolvedUrl("nav-user-initiated.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            webEngineView.runJavaScript("document.getElementById('link').click()")
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, false)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.LinkClickedNavigation)
        }

        // JavaScript-driven navigation (window.location) without user gesture
        function test_userInitiatedJsNavigation() {
            webEngineView.url = Qt.resolvedUrl("nav-user-initiated.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            webEngineView.runJavaScript("window.location = 'test1.html'")
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, false)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.LinkClickedNavigation)
        }

        // Browser-initiated loadHtml with meta-refresh redirect
        function test_userInitiatedRedirect() {
            const redirectUrl = Qt.resolvedUrl("test1.html")
            webEngineView.loadHtml(
                "<html><head><meta http-equiv='refresh' content='0;url="
                + redirectUrl + "'></head><body>Redirecting...</body></html>")
            tryVerify(function() {
                return webEngineView.url.toString().includes("test1.html")
            }, 10000)

            // The initial loadHtml is browser-initiated (TypedNavigation)
            const initial = navigationRequests[0]
            compare(initial.userInitiated, true)
            compare(initial.navigationType,
                    WebEngineNavigationRequest.TypedNavigation)

            // On some platforms Chromium emits a separate redirect navigation
            if (navigationRequests.length > 1) {
                const redirect = navigationRequests[navigationRequests.length - 1]
                compare(redirect.url.toString().includes("test1.html"), true)
                compare(redirect.userInitiated, false)
                compare(redirect.navigationType,
                        WebEngineNavigationRequest.RedirectNavigation)
            }
        }

        // Meta-refresh redirect from a file load
        function test_userInitiatedFileRedirect() {
            webEngineView.url = Qt.resolvedUrl("redirect.html")
            tryVerify(function() {
                return webEngineView.url.toString().includes("test1.html")
            }, 10000)

            // Expect 2 navigations: the initial file load and the redirect
            compare(navigationRequests.length, 2)

            // The initial file load is browser-initiated, hence user-initiated
            const initial = navigationRequests[0]
            compare(initial.url.toString().includes("redirect.html"), true)
            compare(initial.userInitiated, true)
            compare(initial.navigationType,
                    WebEngineNavigationRequest.TypedNavigation)

            // The redirect navigation to test1.html
            const redirect = navigationRequests[navigationRequests.length - 1]
            compare(redirect.url.toString().includes("test1.html"), true)
            compare(redirect.userInitiated, false)
            compare(redirect.navigationType,
                    WebEngineNavigationRequest.RedirectNavigation)
        }

        // Form submission is user-initiated when triggered by user click
        function test_userInitiatedFormSubmit() {
            webEngineView.url = Qt.resolvedUrl("nav-user-initiated.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            const center = getElementCenter("formSubmit")
            mouseClick(webEngineView, center.x, center.y)
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, true)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.FormSubmittedNavigation)
        }

        // Form submission via JS without user gesture is not user-initiated
        function test_userInitiatedFormSubmitJs() {
            webEngineView.url = Qt.resolvedUrl("nav-user-initiated.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            webEngineView.runJavaScript("document.getElementById('form').submit()")
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, false)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.FormSubmittedNavigation)
        }

        // Back/forward navigation is user-initiated (browser-initiated)
        function test_userInitiatedBackForward() {
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            webEngineView.goBack()
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, true)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.BackForwardNavigation)
        }

        // history.back() from JS without user gesture is not user-initiated
        function test_userInitiatedBackForwardJs() {
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            webEngineView.runJavaScript("history.back()")
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, false)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.BackForwardNavigation)
        }

        // Reload is user-initiated (browser-initiated)
        function test_userInitiatedReload() {
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            webEngineView.reload()
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, true)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.ReloadNavigation)
        }

        // window.location.reload() from JS without user gesture is not user-initiated
        function test_userInitiatedReloadJs() {
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            navigationRequests = []

            webEngineView.runJavaScript("window.location.reload()")
            verify(webEngineView.waitForLoadSucceeded())
            compare(navigationRequests.length, 1)
            const nav = navigationRequests[0]
            compare(nav.userInitiated, false)
            compare(nav.navigationType,
                    WebEngineNavigationRequest.RedirectNavigation)
        }
    }
}
