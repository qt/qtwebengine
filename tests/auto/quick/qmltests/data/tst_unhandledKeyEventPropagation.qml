// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

Item {
    id: parentItem
    width: 400
    height: 300

    property var pressEvents: []
    property var releaseEvents: []
    Keys.onPressed: function(event) {
        pressEvents.push(event.key)
    }
    Keys.onReleased: function(event) {
        releaseEvents.push(event.key)
    }

    TestWebEngineView {
        id: webEngineView
        anchors.fill: parent
        focus: true
    }
    TestCase {
        id: testCase
        name: "WebEngineViewUnhandledKeyEventPropagation"

        when: false
        Timer {
            running: parent.windowShown
            repeat: false
            interval: 1
            onTriggered: parent.when = true
        }

        function test_keyboardModifierMapping() {
            webEngineView.url = Qt.resolvedUrl("keyboardEvents.html");
            verify(webEngineView.waitForLoadSucceeded());

            webEngineView.runJavaScript("document.getElementById('first_div').focus()");
            webEngineView.verifyElementHasFocus("first_div");

            keyPress(Qt.Key_Right);
            keyRelease(Qt.Key_Right);
            // Right arrow key is unhandled thus focus is not changed
            tryCompare(parentItem.releaseEvents, "length", 1);
            webEngineView.verifyElementHasFocus("first_div");

            keyPress(Qt.Key_Tab);
            keyRelease(Qt.Key_Tab);
            // Tab key is handled thus focus is changed
            tryCompare(parentItem.releaseEvents, "length", 2);
            webEngineView.verifyElementHasFocus("second_div");

            keyPress(Qt.Key_Left);
            keyRelease(Qt.Key_Left);
            // Left arrow key is unhandled thus focus is not changed
            tryCompare(parentItem.releaseEvents, "length", 3);
            webEngineView.verifyElementHasFocus("second_div");

            // The page will consume the Tab key to change focus between elements while the arrow
            // keys won't be used.
            compare(parentItem.pressEvents.length, 2);
            compare(parentItem.pressEvents[0], Qt.Key_Right);
            compare(parentItem.pressEvents[1], Qt.Key_Left);

            // Key releases will all come back unconsumed.
            compare(parentItem.releaseEvents[0], Qt.Key_Right);
            compare(parentItem.releaseEvents[1], Qt.Key_Tab);
            compare(parentItem.releaseEvents[2], Qt.Key_Left);
        }
    }
}
