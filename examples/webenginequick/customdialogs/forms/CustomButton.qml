// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: root
    width: 200
    height: 30
    radius: 5
    property string btnText: "Name"
    property bool btnEnable: true
    property bool btnBlue: true
    opacity: btnEnable ? 1.0 : 0.5
    signal clicked()
    gradient: btnBlue ? blueButton : greenButton
    Text {
        id: textArea
        x: 54
        y: 5
        color: "#ffffff"
        text: parent.btnText
        font.pointSize: 12
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.bold: false
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            if (btnEnable)
                root.clicked();
        }
    }

    Gradient {
        id: blueButton
        GradientStop {
            position: 0
            color: "#25a6e2"
        }
        GradientStop {
            position: mouseArea.pressed && root.btnEnable ? 0.7 :1
            color: "#188bd0"
        }
    }

    Gradient {
        id: greenButton
        GradientStop {
            position: 0
            color: "#80c342"
        }
        GradientStop {
            position: mouseArea.pressed && root.btnEnable ? 0.7 :1
            color: "#5fac18"
        }
    }
}
