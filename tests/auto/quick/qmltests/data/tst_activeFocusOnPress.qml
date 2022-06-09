// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest

Item {
    id: root
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
        text: "foo"
    }

    TestWebEngineView {
        id: webEngineView
        activeFocusOnPress: false
        anchors {
            top: textInput.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        TestCase {
            id: testCase
            name: "ActiveFocusOnPress"
            when:windowShown

            function test_activeFocusOnPress() {
                textInput.forceActiveFocus()
                verify(textInput.activeFocus)
                mouseClick(root, 150, 300, Qt.LeftButton)
                verify(textInput.activeFocus)
            }
        }
    }
}
