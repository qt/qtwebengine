// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    property alias followLink: followLink
    property alias back: back
    property alias forward: forward
    property alias reload: reload
    property alias copyLinkUrl: copyLinkUrl
    property alias saveLink: saveLink
    property alias close: close

    ColumnLayout {
        id: columnLayout
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        Image {
            id: image
            width: 100
            height: 100
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            source: "qrc:/icon.svg"
        }

        CustomButton {
            id: followLink
            btnText: qsTr("Follow")
        }

        CustomButton {
            id: back
            btnText: qsTr("Back")
        }

        CustomButton {
            id: forward
            btnText: qsTr("Forward")
        }

        CustomButton {
            id: reload
            btnText: qsTr("Reload")
        }

        CustomButton {
            id: copyLinkUrl
            btnText: qsTr("Copy Link URL")
        }

        CustomButton {
            id: saveLink
            btnText: qsTr("Save Link")
        }

        CustomButton {
            id: close
            btnBlue: false
            btnText: qsTr("Close")
        }
    }
}
