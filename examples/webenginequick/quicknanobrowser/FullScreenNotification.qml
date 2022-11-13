// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: fullScreenNotification
    width: 500
    height: 40
    color: "white"
    radius: 7

    visible: false
    opacity: 0

    function show() {
        visible = true;
        opacity = 1;
        reset.start();
    }

    function hide() {
        reset.stop();
        opacity = 0;
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 750
            onStopped: {
                if (opacity == 0)
                    visible = false;
            }
        }
    }

    Timer {
        id: reset
        interval: 5000
        onTriggered: hide()
    }

    anchors.horizontalCenter: parent.horizontalCenter
    y: 125

    Text {
        id: message
        width: parent.width

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        wrapMode: Text.WordWrap
        elide: Text.ElideNone
        clip: true

        text: qsTr("You are now in fullscreen mode. Press ESC to quit!")
    }
}
