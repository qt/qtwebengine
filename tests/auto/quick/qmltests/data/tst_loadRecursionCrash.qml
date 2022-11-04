// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

Item {
width: 300
height: 400
    TextInput {
        id: textInput
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        focus: true
        text: Qt.resolvedUrl("test1.html")
        onEditingFinished: webEngineView.url = text
    }

    TestWebEngineView {
        id: webEngineView
        anchors {
            top: textInput.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        TestCase {
            name: "WebEngineViewLoadRecursionCrash"
            when:windowShown

            function test_QTBUG_42929() {
                textInput.forceActiveFocus()
                keyClick(Qt.Key_Return)
                verify(webEngineView.waitForLoadSucceeded())
                textInput.text = "about:blank"
                textInput.forceActiveFocus()
                keyClick(Qt.Key_Return)
                verify(webEngineView.waitForLoadSucceeded())
                textInput.text = Qt.resolvedUrl("test4.html")
                textInput.forceActiveFocus()
                // Don't crash now
                keyClick(Qt.Key_Return)
                verify(webEngineView.waitForLoadSucceeded())
            }
        }
    }
}
