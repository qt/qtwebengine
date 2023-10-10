// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    property alias cancelButton: cancelButton
    property alias okButton: okButton
    property string message: "Message"
    property string title: "Title"
    property alias files: files

    ColumnLayout {
        id: columnLayout
        anchors.topMargin: 20
        anchors.top: parent.top
        anchors.bottomMargin: 20
        anchors.bottom: parent.bottom
        anchors.rightMargin: 20
        anchors.right: parent.right
        anchors.leftMargin: 20
        anchors.left: parent.left

        Image {
            id: image
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            source: "qrc:/icon.svg"
        }

        Rectangle {
            id: rectangle
            width: parent.width
            height: 30
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#25a6e2"
                }

                GradientStop {
                    color: "#188bd0"
                }
            }

            Text {
                id: title
                x: 54
                y: 5
                color: "#ffffff"
                text: qsTr("Select File")
                font.pointSize: 12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Item {
            width: 40
            height: 40
        }

        ColumnLayout {
            id: files

            FileRow {
                id: filename1
                text: "example.qdoc"
            }

            FileRow {
                id: filename2
                text: "factory.cpp"
            }

            FileRow {
                id: filename3
                text: "index.html"
            }

            FileRow {
                id: filename4
                text: "main.qml"
            }

            FileRow {
                id: filename5
                text: "qt-logo.png"
            }

            FileRow {
                id: filename6
                text: "window.h"
            }
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            id: rowLayout
            width: 20
            height: 100

            Item {
                Layout.fillWidth: true
            }

            CustomButton {
                id: cancelButton
                width: 90
                height: 30
                btnText: qsTr("Cancel")
                btnBlue: false
            }

            CustomButton {
                id: okButton
                width: 90
                height: 30
                btnText: qsTr("OK")
                btnBlue: false
            }
        }
    }
}
