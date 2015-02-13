/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
