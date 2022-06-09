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
        id: titleSpy
        target: webEngineView
        signalName: "titleChanged"
    }

    TestCase {
        name: "WebEngineViewKeyboardModifierMapping"

        when: false
        Timer {
            running: parent.windowShown
            repeat: false
            interval: 1
            onTriggered: parent.when = true
        }

        function getPressedModifiers() {
            var pressedModifiers;
            runJavaScript("getPressedModifiers()", function(result) {
                pressedModifiers = result;
            });
            tryVerify(function() { return pressedModifiers != undefined });
            return pressedModifiers;
        }

        function test_keyboardModifierMapping() {
            webEngineView.url = Qt.resolvedUrl("keyboardModifierMapping.html")
            waitForLoadSucceeded();
            titleSpy.wait()

            // Alt
            keyPress(Qt.Key_Alt);
            titleSpy.wait()
            compare(getPressedModifiers(), "alt:pressed ctrl:no meta:no");
            keyRelease(Qt.Key_Alt)
            titleSpy.wait()

            // Ctrl
            // On mac Qt automatically translates Meta to Ctrl and vice versa.
            // However, if sending the events manually no mapping is being done,
            // so we have to do this here manually.
            // For testing we assume that the flag Qt::AA_MacDontSwapCtrlAndMeta is NOT set.
            keyPress(Qt.platform.os == "osx" ? Qt.Key_Meta : Qt.Key_Control);
            titleSpy.wait()
            compare(getPressedModifiers(), "alt:released ctrl:pressed meta:no");
            keyRelease(Qt.platform.os == "osx" ? Qt.Key_Meta : Qt.Key_Control);
            titleSpy.wait()

            // Meta (Command on Mac)
            keyPress(Qt.platform.os == "osx" ? Qt.Key_Control : Qt.Key_Meta);
            titleSpy.wait()
            compare(getPressedModifiers(), "alt:released ctrl:released meta:pressed");
            keyRelease(Qt.platform.os == "osx" ? Qt.Key_Control : Qt.Key_Meta);
            titleSpy.wait()

            compare(getPressedModifiers(), "alt:released ctrl:released meta:released");
        }
    }
}
