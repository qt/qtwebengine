// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Rectangle {
    property alias zoomFactor: slider.value ;
    function zoomIn() {
        visible = true
        visibilityTimer.restart()
        zoomFactor = zoomFactor + 0.25;
    }
    function zoomOut() {
        visible = true
        visibilityTimer.restart()
        zoomFactor = zoomFactor - 0.25;
    }
    function reset() { zoomFactor = 1.0 }

    width: 220
    height: 30
    color: palette.window
    visible: false
    radius: 4

    SystemPalette {
        id: palette
    }
    Timer {
        id: visibilityTimer
        interval: 3000
        repeat: false
        onTriggered: zoomController.visible = false
    }

    RowLayout {
        anchors.margins: 4
        anchors.fill: parent
        ToolButton {
            id: plusButton
            text: '+'
            onClicked: zoomIn()
        }
        ToolButton {
            text: '\u2014'
            id: minusButton
            onClicked: zoomOut()
        }
        Slider {
            id: slider
            maximumValue: 5.0
            minimumValue: 0.25
            Layout.fillWidth: true;
            stepSize: 0.05
            value: 1
            onValueChanged: visibilityTimer.restart()
        }
        Button {
            text: "Reset"
            onClicked: reset()
        }
    }
}
