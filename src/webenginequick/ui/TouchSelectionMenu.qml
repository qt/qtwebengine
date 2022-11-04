// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: menu

    signal cutTriggered
    signal copyTriggered
    signal pasteTriggered
    signal contextMenuTriggered

    property bool isCutEnabled: false
    property bool isCopyEnabled: false
    property bool isPasteEnabled: false

    property color borderColor: "darkGray"
    property color bgColor: "white"

    radius: 4
    border.color: borderColor
    color: borderColor
    antialiasing: true

    RowLayout {
        anchors.fill: parent
        spacing: parent.border.width
        anchors.margins: parent.border.width

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            radius: menu.radius
            color: bgColor
            visible: isCutEnabled

            Text {
                id: cutText
                anchors.centerIn: parent
                text: "Cut"
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    parent.color = borderColor;
                    cutText.color = "white";
                }
                onReleased: {
                    parent.color = bgColor;
                    cutText.color = "black";
                    cutTriggered();
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            radius: menu.radius
            color: bgColor
            visible: isCopyEnabled

            Text {
                id: copyText
                anchors.centerIn: parent
                text: "Copy"
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    parent.color = borderColor;
                    copyText.color = "white";
                }
                onReleased: {
                    parent.color = bgColor;
                    copyText.color = "black";
                    copyTriggered();
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            radius: menu.radius
            color: bgColor
            visible: isPasteEnabled

            Text {
                id: pasteText
                anchors.centerIn: parent
                text: "Paste"
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    parent.color = borderColor;
                    pasteText.color = "white";
                }
                onReleased: {
                    parent.color = bgColor;
                    pasteText.color = "black";
                    pasteTriggered();
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            radius: menu.radius
            color: bgColor

            Text {
                id: contextMenuText
                anchors.centerIn: parent
                text: "..."
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    parent.color = borderColor;
                    contextMenuText.color = "white";
                }
                onReleased: {
                    parent.color = bgColor;
                    contextMenuText.color = "black";
                    contextMenuTriggered();
                }
            }
        }
    }
}
