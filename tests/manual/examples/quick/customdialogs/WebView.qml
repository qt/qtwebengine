// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWebEngine

WebEngineView {
    id: view
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
        if (useDefaultDialogs) {
            // do not show proxy error page
            view.url = "qrc:/index.html"
            return;
        }
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

    onTouchSelectionMenuRequested: function(request) {
        if (useDefaultDialogs)
            return;

        request.accepted = true;
        openForm({item: Qt.resolvedUrl("forms/TouchSelectionMenu.qml"),
                     properties: {"request": request}});
    }
}
