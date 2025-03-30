// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    property alias text: message.text
    property alias prompt: field.text
    property bool handled: false
    signal input(string text)
    title: qsTr("Prompt Dialog")
    modal: false
    anchors.centerIn: parent
    objectName: "promptDialog"


    function acceptDialog() {
        input(field.text);
        accept();
    }

    ColumnLayout {
        id: rootLayout
        anchors.fill: parent
        anchors.margins: 4
        property int minimumWidth: rootLayout.implicitWidth + rootLayout.doubleMargins
        property int minimumHeight: rootLayout.implicitHeight + rootLayout.doubleMargins
        property int doubleMargins: anchors.margins * 2
        SystemPalette { id: palette; colorGroup: SystemPalette.Active }
        Text {
            id: message
            Layout.fillWidth: true
            color: palette.windowText
            textFormat: Text.PlainText
        }
        TextField {
            id:field
            focus: true
            Layout.fillWidth: true
            onAccepted: acceptDialog()
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
                onClicked: reject()
            }
        }
    }
}
