/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// FIXME: authentication missing in Qt Quick Dialogs atm. Make our own for now.
import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2

Window {
    signal accepted(string user, string password);
    signal rejected;
    property alias text: message.text

    title: qsTr("Authentication Required")
    flags: Qt.Dialog
    modality: Qt.WindowModal

    width: minimumWidth
    height: minimumHeight
    minimumWidth: rootLayout.implicitWidth + rootLayout.doubleMargins
    minimumHeight: rootLayout.implicitHeight + rootLayout.doubleMargins

    SystemPalette { id: palette; colorGroup: SystemPalette.Active }
    color: palette.window

    function open() {
        show();
    }

    function acceptDialog() {
        accepted(userField.text, passwordField.text);
        close();
    }

    ColumnLayout {
        id: rootLayout
        anchors.fill: parent
        anchors.margins: 4
        property int doubleMargins: anchors.margins * 2
        Text {
            id: message;
            color: palette.windowText
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
                onAccepted: acceptDialog()
            }
            Label {
                text: qsTr("Password:")
                color: palette.windowText
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                onAccepted: acceptDialog()
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
                text: qsTr("&Cancel")
                onClicked: {
                    rejected();
                    close();
                }
            }
            Button {
                text: qsTr("&Log In")
                isDefault: true
                onClicked: acceptDialog()
            }
        }
    }
}
