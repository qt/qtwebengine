// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property int progress: 0
    property url iconUrl: ""
    property url pageUrl: ""

    signal accepted(url addressUrl)

    clip: true

    onActiveFocusChanged: {
        if (activeFocus)
            addressField.forceActiveFocus();
    }

    Rectangle {
        width: addressField.width / 100 * root.progress
        height: root.height

        visible: root.progress < 100

        color: "#b6dca6"
        radius: root.radius
    }

    TextField {
        id: addressField
        anchors.fill: parent
        leftPadding: 30

        background: Rectangle {
            color: "transparent"
            border.color: "black"
            border.width: 1
            radius: root.radius
        }

        Image {
            anchors.verticalCenter: addressField.verticalCenter
            x: 5; z: parent.z + 1
            width: 16; height: 16
            sourceSize: Qt.size(width, height)
            source: root.iconUrl
            visible: root.progress == 100
        }

        Text {
            text: root.progress < 0 ? "" : root.progress + "%"
            x: 5; z: parent.z + 1
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter

            visible: root.progress < 100
        }

        onActiveFocusChanged: {
            if (activeFocus)
                selectAll();
            else
                deselect();
        }

        text: root.pageUrl
        onAccepted: root.accepted(utils.fromUserInput(text))
    }
}
