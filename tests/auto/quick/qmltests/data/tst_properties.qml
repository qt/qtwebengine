// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    TestCase {
        name: "WebEngineViewProperties"

        function test_title() {
            webEngineView.url =  Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.title, "Test page 1")
        }

        function test_url() {
            var testUrl = Qt.resolvedUrl("test1.html")
            webEngineView.url = testUrl
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, testUrl)
        }
    }
}
