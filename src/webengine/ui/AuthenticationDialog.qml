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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQuick 2.5

ApplicationWindow {
    signal accepted(string user, string password);
    signal rejected;
    property alias text: message.text;

    width: 350
    height: 100
    flags: Qt.Dialog

    title: "Authentication Required"

    function open() {
        show();
    }

    ColumnLayout {
        anchors.fill: parent;
        anchors.margins: 4;
        Text {
            id: message;
            Layout.fillWidth: true;
        }
        RowLayout {
            Label {
                text: "Username:"
            }
            TextField {
                id: userField;
                Layout.fillWidth: true;
            }
        }
        RowLayout {
            Label {
                text: "Password:"
            }
            TextField {
                id: passwordField;
                Layout.fillWidth: true;
                echoMode: TextInput.Password;
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8;
            Button {
                text: "Log In"
                onClicked: {
                    accepted(userField.text, passwordField.text);
                    close();
                    destroy();
                }
            }
            Button {
                text: "Cancel"
                onClicked: {
                    rejected();
                    close();
                    destroy();
                }
            }
        }
    }

}
