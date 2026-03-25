// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

Item {
    id: root

    // Defines the target object that will be used
    // to listen for javaScriptConsoleMessage signal
    property var target: null

    // List of javaScriptConsoleMessage arguments stored as objects
    readonly property alias messages: root.qtest_messages;

    // Number of messages
    readonly property alias count: root.qtest_count;


    property int qtest_count: 0;
    property int qtest_expectedCount: 0;
    property var qtest_messages: [];

    Connections {
        id: connection
        target: root.target

        function onJavaScriptConsoleMessage(level, message, lineNumber, source) {
            qtest_messages.push({ level, message, lineNumber, source });
            ++qtest_count;
        }
    }

    Component.onDestruction: clear();
    onTargetChanged: clear();

    // Returns the last console message received
    function popMessage() {
        --qtest_count;
        --qtest_expectedCount;
        return qtest_messages.pop().message;
    }

    // Clears count and messages
    // Throws a warning for each unresolved JavaScript message
    function clear() {
        while (qtest_messages.length) {
            var jsMessage = qtest_messages.pop();
            console.warn("Unhandled message: " + jsMessage.message);
        }
        qtest_count = 0;
        qtest_expectedCount = 0;
    }

    TestResult { id: qtest_results }
    TestUtil { id: util }

    // Waits for the next message
    function wait(timeout) {
        var expected = ++qtest_expectedCount;
        var result = _waitFor(function () { return qtest_count >= expected }, timeout);
        if (!qtest_results.verify(result, "wait for signal WebEngineView.javaScriptConsoleMessage",
            util.callerFile(), util.callerLine())) {
            throw new Error("QtQuickTest::fail");
        }
    }

    // waits for the first signal, that has the defined message
    // also handles the expected message
    function waitForMessage(expectedMessage, timeout) {
        if (expectedMessage == undefined)
            return false;
        var found = false;
        var index = -1;
        //filters previous messages
        if (qtest_count) {
            index = qtest_messages.findIndex((element) => element.message == expectedMessage);
            if (index > -1) {
                qtest_messages.splice(index, 1);
                found = true;
                --qtest_count;
            }
        }
        if (found)
            return true;
        index = qtest_count -1;

        //continously filters incoming messages
        _waitFor(function () {
            if (index < qtest_count - 1 && !found)
                found = (qtest_messages[++index].message == expectedMessage)
            return found
        }, timeout);
        if (found) {
            qtest_messages.splice(index, 1);
            --qtest_count;
        }
        return found;
    }
}
