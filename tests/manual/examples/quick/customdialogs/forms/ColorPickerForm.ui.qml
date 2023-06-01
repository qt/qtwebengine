// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    property alias cancelButton: cancelButton
    property alias okButton: okButton
    property string message: "Message"
    property string title: "Title"
    property alias blue1: blue1
    property alias grid: grid
    property alias colorPicker: colorPicker

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
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            source: "qrc:/icon.svg"
        }

        Rectangle {
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
                text: qsTr("Select Color")
                font.pointSize: 12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Item {
            width: 40
            height: 40
        }

        GridLayout {
            id: grid
            columns: 5
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            ColorCell {
                id: blue1
                color: "#26d5f8"
            }
            ColorCell {
                id: green1
                color: "#25f93d"
            }
            ColorCell {
                id: red1
                color: "#f71111"
            }
            ColorCell {
                id: yellow1
                color: "#faf23c"
            }
            ColorCell {
                id: orange1
                color: "#ec8505"
            }
            ColorCell {
                id: blue2
                color: "#037eaa"
            }
            ColorCell {
                id: green2
                color: "#389a13"
            }
            ColorCell {
                id: red2
                color: "#b2001b"
            }
            ColorCell {
                id: yellow2
                color: "#caca03"
            }
            ColorCell {
                id: orange2
                color: "#bb4900"
            }
            ColorCell {
                id: blue3
                color: "#01506c"
            }
            ColorCell {
                id: green3
                color: "#37592b"
            }
            ColorCell {
                id: red3
                color: "#700113"
            }
            ColorCell {
                id: yellow3
                color: "#848404"
            }

            ColorCell {
                id: orange3
                color: "#563100"
            }
        }

        Item {
            width: 10
            height: 10
        }

        Rectangle {
            width: 90
            height: 90
            color: "#000000"
            radius: 4
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Rectangle {
                id: colorPicker
                height: 80
                color: "#ffffff"
                anchors.rightMargin: 5
                anchors.leftMargin: 5
                anchors.bottomMargin: 5
                anchors.topMargin: 5
                anchors.fill: parent
            }
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            id: rowLayout
            width: 100
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
