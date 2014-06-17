/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

import QtQuick 2.1

Rectangle {
    id: errorPage
    property alias heading: errorHeading.text;
    property alias details: errorDetails.text;
    property bool displayingError: false;
    property string errorName;
    anchors.fill: parent
    color: "lightgray"
    visible: displayingError
    z: 3

    Rectangle {
        color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height / 8
        height: Math.max(parent.height / 3, errorHeading.height * 2 + errorCode.height + errorDetails.height + 20)
        width: Math.max(parent.width / 2, errorHeading.width + 20)

        border {
            color: "dimgray"
            width: 0.5
        }

        radius: 20
        Text {
            id: errorHeading
            color: "dimgray"
            font.pixelSize: 20
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height / 4
        }
        Text {
            id: errorDetails
            color: "gray"
            wrapMode: Text.WordWrap
            width: parent.width - 20
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: errorHeading.bottom
            anchors.margins: 10
        }
        Text {
            id: errorCode
            color: "gray"
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: errorDetails.bottom
            text: "Error code: " + errorPage.errorName;
        }
    }

}
