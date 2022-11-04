// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import "../../qmltests/data"

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400
    focus: true

    property string lastUrl

    SignalSpy {
        id: linkHoveredSpy
        target: webEngineView
        signalName: "linkHovered"
    }

    onLinkHovered: function(hoveredUrl) {
        webEngineView.lastUrl = hoveredUrl
    }

    function isViewRendered() {
        var pixel = getItemPixel(webEngineView);

        // The center pixel is expected to be red.
        if (pixel[0] !== 255) return false;
        if (pixel[1] !== 0) return false;
        if (pixel[2] !== 0) return false;

        return true;
    }

    TestCase {
        id: testCase
        name: "DesktopWebEngineViewLinkHovered"

        // Delayed windowShown to workaround problems with Qt5 in debug mode.
        when: false
        Timer {
            running: parent.windowShown
            repeat: false
            interval: 1
            onTriggered: parent.when = true
        }

        function init() {
            webEngineView.lastUrl = "";
            linkHoveredSpy.clear();
        }

        function test_linkHovered() {
            compare(linkHoveredSpy.count, 0);
            mouseMove(webEngineView, 100, 300)
            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())

            // We get a linkHovered signal with empty hoveredUrl after page load
            linkHoveredSpy.wait();
            compare(linkHoveredSpy.count, 1);
            compare(webEngineView.lastUrl, "")

            // Wait for the page to be rendered before trying to test based on input events
            tryVerify(isViewRendered);

            mouseMove(webEngineView, 100, 100)
            linkHoveredSpy.wait(12000);
            compare(linkHoveredSpy.count, 2);
            compare(webEngineView.lastUrl, Qt.resolvedUrl("test1.html"))
            mouseMove(webEngineView, 100, 300)
            linkHoveredSpy.wait(12000);
            compare(linkHoveredSpy.count, 3);
            compare(webEngineView.lastUrl, "")
        }

        function test_linkHoveredDoesntEmitRepeated() {
            compare(linkHoveredSpy.count, 0);
            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())

            // We get a linkHovered signal with empty hoveredUrl after page load
            linkHoveredSpy.wait();
            compare(linkHoveredSpy.count, 1);
            compare(webEngineView.lastUrl, "")

            // Wait for the page to be rendered before trying to test based on input events
            tryVerify(isViewRendered);

            for (var i = 0; i < 100; i += 10)
                mouseMove(webEngineView, 100, 100 + i)

            linkHoveredSpy.wait(12000);
            compare(linkHoveredSpy.count, 2);
            compare(webEngineView.lastUrl, Qt.resolvedUrl("test1.html"))

            for (var i = 0; i < 100; i += 10)
                mouseMove(webEngineView, 100, 300 + i)

            linkHoveredSpy.wait(12000);
            compare(linkHoveredSpy.count, 3);
            compare(webEngineView.lastUrl, "")
        }
    }
}
