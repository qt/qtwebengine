// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    property alias text: message.text
    property bool handled: false
    signal accepted()
    signal rejected()
    title: qsTr("Confirm Dialog")
    modal: false
    anchors.centerIn: parent
    objectName: "confirmDialog"

    //handle the case where users simply closes the dialog
    onVisibleChanged: {
        if (visible == false && handled == false) {
            handled = true;
            rejected();
        } else {
            handled = false;
        }
    }

    function acceptDialog() {
        accepted();
        handled = true;
        close();
    }

    function rejectDialog() {
        rejected();
        handled = true;
        close();
    }

    ColumnLayout {
        id: rootLayout
        anchors.fill: parent
        anchors.margins: 4
        property int minimumWidth: rootLayout.implicitWidth + rootLayout.doubleMargins
        property int minimumHeight: rootLayout.implicitHeight + rootLayout.doubleMargins
        property int doubleMargins: anchors.margins * 2
        SystemPalette { id: palette; colorGroup: SystemPalette.Active }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8
            Image {
                source: "qrc:/qt-project.org/imports/QtWebEngine/ControlsDelegates/question.png"
            }
            Text {
                id: message
                Layout.fillWidth: true
                color: palette.windowText
                textFormat: Text.PlainText
            }
        }
        Item {
            Layout.fillHeight: true
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8
            Button {
                text: qsTr("OK")
                onClicked: acceptDialog()
            }
            Button {
                text: qsTr("Cancel")
                onClicked: rejectDialog()
            }
        }
    }
}
