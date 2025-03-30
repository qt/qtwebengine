// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    property alias text: message.text
    property bool handled: false
    signal credentials(string user, string password)
    title: qsTr("Authentication Required")
    modal: false
    anchors.centerIn: parent
    objectName: "authenticationDialog"

    function acceptDialog() {
        credentials(userField.text, passwordField.text);
        accept()
    }

    ColumnLayout {
        id: rootLayout
        anchors.fill: parent
        anchors.margins: 4
        property int minimumWidth: rootLayout.implicitWidth + rootLayout.doubleMargins
        property int minimumHeight: rootLayout.implicitHeight + rootLayout.doubleMargins

        property int doubleMargins: anchors.margins * 2

        SystemPalette { id: palette; colorGroup: SystemPalette.Active }
        Label {
            id: message
            color: palette.windowText
            textFormat: Text.PlainText
        }
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Username:")
                color: palette.windowText
            }
            TextField {
                id: userField
                focus: true
                Layout.fillWidth: true
                onAccepted: {
                    if (userField.text && passwordField.text)
                        acceptDialog();
                }
            }
            Label {
                text: qsTr("Password:")
                color: palette.windowText
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                onAccepted: {
                    if (userField.text && passwordField.text)
                        acceptDialog();
                }
            }
        }
        Item {
            Layout.fillHeight: true
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8
            Button {
                id: cancelButton
                text: qsTr("Cancel")
                onClicked: reject()
            }
            Button {
                text: qsTr("Log In")
                onClicked: acceptDialog()
            }
        }
    }
}
