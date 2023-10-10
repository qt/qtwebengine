// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick 2.3
import QtQuick.Controls 1.2

// The QtQuick controls guys are slackers, so we need to make our own stuff

ToolButton {
    id: root
    property Menu longPressMenu
    function showMenu() {
        longPressMenu.__popup(Qt.rect(0, root.height, 0, 0), 0)
    }

    Binding {
        target: longPressMenu
        property: "__visualItem"
        value: root
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button == Qt.RightButton)
                showMenu()
            else
                root.clicked()
        }
        onPressAndHold: showMenu()
    }
}
