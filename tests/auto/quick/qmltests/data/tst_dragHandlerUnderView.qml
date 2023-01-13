// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

Item {
    id: parentItem
    width: 400
    height: 300

    Rectangle {
        id: draggableDownUnder
        color: "wheat"
        width: 350
        height: 250

        DragHandler { id: dragHandler }
    }

    TestWebEngineView {
        id: webEngineView
        width: 300
        height: 250

        property var testUrl: Qt.resolvedUrl("test4.html")

        SignalSpy {
            id: scrollPositionSpy
            target: webEngineView
            signalName: "onScrollPositionChanged"
        }

        SignalSpy {
            id: dragActiveSpy
            target: dragHandler
            signalName: "activeChanged"
        }

        TestCase {
            id: testCase
            name: "KeepMouseGrabDuringScrolling"
            when: windowShown

            function test_scroll() {
                webEngineView.url = Qt.resolvedUrl("test4.html");
                verify(webEngineView.waitForLoadSucceeded());

                mousePress(webEngineView, 295, 20);
                mouseMove(webEngineView, 295, 200);
                mouseRelease(webEngineView, 295, 200);

                // WebEngineView scrolled if the scrollbar was visible.
                // But on macOS, the scrollbar is hidden, so text gets selected.
                tryVerify(function() {
                    return (scrollPositionSpy.count === 1 && webEngineView.scrollPosition.y > 100)
                            || webEngineView.getTextSelection().length > 0;
                });

                // DragHandler didn't take over and drag
                compare(dragActiveSpy.count, 0);
                compare(draggableDownUnder.y, 0);
            }
        }
    }
}
