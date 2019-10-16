/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
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

import QtQuick 2.0
import QtWebEngine 1.10

WebEngineView {

    url: "qrc:/index.html"
    property bool useDefaultDialogs: true
    signal openForm(var form)

    Rectangle {
        id: tooltip
        width: 200
        height: 30
        z: 50
        visible: false
        color: "gray"
        border.color: "black"
        border.width: 2
        radius: 3

        property string text: ""

        Text {
            x: 0
            y: 0
            color: "#ffffff"
            text: parent.text
            font.pointSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.bold: false
        }

    }

    onContextMenuRequested: function(request) {
        // we only show menu for links with #openMenu
        if (!request.linkUrl.toString().endsWith("#openMenu")) {
            request.accepted = true;
            return;
        }
        // return early to show default menu
        if (useDefaultDialogs)
            return;

        request.accepted = true;
        openForm({item: Qt.resolvedUrl("forms/Menu.qml"),
                     properties: {"request": request}});
    }

    onTooltipRequested: function(request) {
        if (useDefaultDialogs)
            return;

        if (request.type == TooltipRequest.Show) {
            tooltip.visible = true;
            tooltip.x = request.x;
            tooltip.y = request.y;
            tooltip.text = request.text;
        } else {
            tooltip.visible = false;
        }

        request.accepted = true;
    }

    onAuthenticationDialogRequested: function(request) {
        if (useDefaultDialogs)
            return;

        request.accepted = true;
        openForm({item: Qt.resolvedUrl("forms/Authentication.qml"),
                     properties: {"request": request}});
    }

    onJavaScriptDialogRequested: function(request) {
        if (useDefaultDialogs)
            return;

        request.accepted = true;
        openForm({item: Qt.resolvedUrl("forms/JavaScript.qml"),
                     properties: {"request": request}});
    }

    onColorDialogRequested: function(request) {
        if (useDefaultDialogs)
            return;

        request.accepted = true;
        openForm({item: Qt.resolvedUrl("forms/ColorPicker.qml"),
                     properties: {"request": request}});
    }

    onFileDialogRequested: function(request) {
        if (useDefaultDialogs)
            return;

        request.accepted = true;
        openForm({item: Qt.resolvedUrl("forms/FilePicker.qml"),
                     properties: {"request": request}});

    }
}
