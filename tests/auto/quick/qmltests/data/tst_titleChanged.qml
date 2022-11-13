// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300


    SignalSpy {
        id: spyTitle
        target: webEngineView
        signalName: "titleChanged"
    }


    TestCase {
        name: "WebEngineViewTitleChangedSignal"

        function test_titleFirstLoad() {
            compare(spyTitle.count, 0)

            var testUrl = Qt.resolvedUrl("test3.html")
            webEngineView.url = testUrl
            spyTitle.wait()
            if (webEngineView.title == "test3.html") {
                // This title may be emitted during loading
                spyTitle.clear()
                spyTitle.wait()
            }
            compare(webEngineView.title, "Test page 3")
            spyTitle.clear()
            spyTitle.wait()
            compare(webEngineView.title, "New Title")
        }
    }
}
