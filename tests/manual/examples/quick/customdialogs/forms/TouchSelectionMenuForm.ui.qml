// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    property alias cut: cut
    property alias copy: copy
    property alias paste: paste
    property alias contextMenu: contextMenu

    ColumnLayout {
        id: columnLayout
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        CustomButton {
            id: cut
            btnText: qsTr("Cut")
        }

        CustomButton {
            id: copy
            btnText: qsTr("Copy")
        }

        CustomButton {
            id: paste
            btnText: qsTr("Paste")
        }

        CustomButton {
            id: contextMenu
            btnText: qsTr("...")
        }

    }
}
