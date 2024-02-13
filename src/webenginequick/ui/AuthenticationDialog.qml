// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    property alias text: message.text
    property bool handled: false
    signal accepted(string user, string password)
    signal rejected()
    title: qsTr("Authentication Required")
    modal: false
    anchors.centerIn: parent
    objectName: "authenticationDialog"

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
        accepted(userField.text, passwordField.text);
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
                onClicked: rejectDialog()
            }
            Button {
                text: qsTr("Log In")
                onClicked: acceptDialog()
            }
        }
    }
}
