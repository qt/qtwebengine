// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 300
    height: 400

    property var testUrl: Qt.resolvedUrl("test4.html")

    SignalSpy {
        id: scrollPositionSpy
        target: webEngineView
        signalName: "onScrollPositionChanged"
    }

    TestCase {
        name: "ScrollPosition"
        when: windowShown

        function init() {
            webEngineView.url = Qt.resolvedUrl("about:blank");
            verify(webEngineView.waitForLoadSucceeded());
        }

        function test_scrollPosition() {
            webEngineView.url = testUrl;
            verify(webEngineView.waitForLoadSucceeded());

            keyPress(Qt.Key_Return); // Focus is on the scroll button.

            tryCompare(scrollPositionSpy, "count", 1);
            compare(webEngineView.scrollPosition.x, 0);
            compare(webEngineView.scrollPosition.y, 600);
        }

        function test_scrollPositionAfterReload() {
            webEngineView.url = testUrl;
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView.scrollPosition, "y", 0);

            keyPress(Qt.Key_Return); // Focus is on the scroll button.

            // Wait for proper scroll position change otherwise we cannot expect
            // the new y position after reload.
            tryCompare(webEngineView.scrollPosition, "x", 0);
            tryCompare(webEngineView.scrollPosition, "y", 600);

            webEngineView.reload();
            verify(webEngineView.waitForLoadSucceeded());

            tryCompare(webEngineView.scrollPosition, "x", 0);
            tryCompare(webEngineView.scrollPosition, "y", 600);
        }
    }
}
