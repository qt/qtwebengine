// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import "../../qmltests/data"

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    property var unavailableUrl: Qt.resolvedUrl("file_that_does_not_exist.html")

    SignalSpy {
        id: loadSpy
        target: webEngineView
        signalName: 'loadingChanged'
    }

    TestCase {
        id: test
        name: "WebEngineViewLoadFail"

        function cleanup() {
            loadSpy.clear()
        }

        function test_fail() {
            WebEngine.settings.errorPageEnabled = false
            webEngineView.url = unavailableUrl
            verify(webEngineView.waitForLoadFailed())
        }

        function test_fail_url() {
            WebEngine.settings.errorPageEnabled = false
            var url = Qt.resolvedUrl("test1.html")
            webEngineView.url = url
            compare(webEngineView.url, url)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, url)

            webEngineView.url = unavailableUrl
            compare(webEngineView.url, unavailableUrl)
            verify(webEngineView.waitForLoadFailed())
            // When error page is disabled in case of LoadFail the entry of the unavailable page is not stored.
            // We expect the url of the previously loaded page here.
            compare(webEngineView.url, url)
        }

        function test_error_page() {
            WebEngine.settings.errorPageEnabled = true
            webEngineView.url = unavailableUrl

            // Loading of the error page must be successful
            verify(webEngineView.waitForLoadFailed())

            // Start to load unavailableUrl
            let loadStart = loadSpy.signalArguments[0][0]
            compare(loadStart.status, WebEngineView.LoadStartedStatus)
            compare(loadStart.errorDomain, WebEngineView.NoErrorDomain)
            compare(loadStart.errorDomain, WebEngineLoadingInfo.NoErrorDomain)
            compare(loadStart.url, unavailableUrl)
            verify(!loadStart.isErrorPage)

            // Loading of the unavailableUrl must fail
            let loadFail = loadSpy.signalArguments[1][0]
            compare(loadFail.status, WebEngineView.LoadFailedStatus)
            compare(loadFail.errorDomain, WebEngineView.InternalErrorDomain)
            compare(loadFail.errorDomain, WebEngineLoadingInfo.InternalErrorDomain)
            compare(loadFail.url, unavailableUrl)
            verify(loadFail.isErrorPage)

            compare(webEngineView.url, unavailableUrl)
            compare(webEngineView.title, unavailableUrl)
        }
    }
}
