// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: colorDialog
    title: qsTr("Color Picker Dialog")
    modal: false
    anchors.centerIn: parent
    objectName: "colorDialog"

    property bool handled: false
    property color color

    signal selectedColor(var color)
    signal rejected()

    function accept() {
        handled = true;
        selectedColor(colorDialog.color)
        close();
    }

    function reject() {
        handled = true;
        rejected();
        close();
    }

    // Handle the case where users simply closes or clicks out of the dialog.
    onVisibleChanged: {
        if (!visible && !handled) {
            handled = true;
            rejected();
        } else {
            handled = false;
        }
    }

    function selectColorFromPalette(paletteColor) {
        colorDialog.color = paletteColor;
    }

    function zeroPadding(text, length = 2) {
        var textLength = text.length;

        if (textLength >= length) {
            return text;
        }

        for (var i = 0; i < length - textLength; i++) {
            text = "0" + text;
        }

        return text;
    }

    function calculateRGBA() {

        var rgbArray = [colorDialog.color.r, colorDialog.color.g, colorDialog.color.b]
        if (colorDialog.color.a != 1) {
            rgbArray.push(colorDialog.color.a);
        }

        for (var i = 0; i < rgbArray.length; i++) {
            rgbArray[i] = Number(Math.round(rgbArray[i] * 255)).toString(16);
            rgbArray[i] = zeroPadding(rgbArray[i]);
        }

        return "#" + rgbArray.join("");
    }


    function isNaNOrUndefined(value) {
        return value == null || value == undefined || Number.isNaN(value);
    }

    function parseColorText(colorText) {
        if (colorText[0] == '#') {
            colorText = colorText.substring(1);
        }

        if (!(colorText.length == 6 || colorText.length == 8)) {
            return undefined;
        }

        var rgbaValues = [parseInt("0x" + colorText.substring(0,2)),
                          parseInt("0x" + colorText.substring(2,4)),
                          parseInt("0x" + colorText.substring(4,6)),
                          parseInt("0x" + (colorText.length > 6 ? colorText.substring(6,8) : "FF"))]

        for (var i = 0; i < rgbaValues.length; i++) {
            if (isNaNOrUndefined(rgbaValues[i])) {
                return undefined;
            }
            rgbaValues[i] = rgbaValues[i] / 255;
        }

        return Qt.rgba(rgbaValues[0], rgbaValues[1], rgbaValues[2], rgbaValues[3]);
    }

    ListModel {
        id: colorList
        ListElement { rectangleColor: "red" }
        ListElement { rectangleColor: "orangered" }
        ListElement { rectangleColor: "orange" }
        ListElement { rectangleColor: "gold" }
        ListElement { rectangleColor: "yellow" }
        ListElement { rectangleColor: "yellowgreen" }
        ListElement { rectangleColor: "green" }
        ListElement { rectangleColor: "lightskyblue" }
        ListElement { rectangleColor: "blue" }
        ListElement { rectangleColor: "blueviolet" }
        ListElement { rectangleColor: "violet" }
        ListElement { rectangleColor: "mediumvioletred" }
        ListElement { rectangleColor: "black" }
        ListElement { rectangleColor: "white" }
    }

    contentItem: GridLayout {
        id: grid
        columns: 7
        rows: 5

        Repeater {
            model: colorList
            delegate: Rectangle {
                width: 50
                height: 50
                color: rectangleColor
                border.color: "black"
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    onClicked: selectColorFromPalette(parent.color)
                }
            }
        }
        ColumnLayout {
            id: colorTools
            Layout.columnSpan: 4
            Layout.rowSpan: 2
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Rectangle {
                height: 100
                width: 200
                border.color: "black"
                border.width: 1

                Binding on color {
                    when: colorDialog.color
                    value: colorDialog.color
                }
            }
            TextField {
                id: colorTextField
                width: 100
                selectByMouse: true
                Layout.alignment: Qt.AlignHCenter

                Binding on text {
                    when: colorDialog.color
                    value: calculateRGBA()
                    delayed: true
                }

                onTextEdited: {
                    var parsedColor = parseColorText(colorTextField.text);
                    if (parsedColor != undefined) {
                        colorDialog.color = parsedColor;
                    }
                }

                MouseArea {
                    id: colorTextFieldMouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: colorTextFieldContextMenu.open()
                }

                Menu {
                    id: colorTextFieldContextMenu
                    x: colorTextFieldMouseArea.mouseX
                    y: colorTextFieldMouseArea.mouseY
                    MenuItem {
                        text: qsTr("Copy color")
                        onTriggered: {
                            colorTextField.selectAll();
                            colorTextField.copy();
                            colorTextField.deselect();
                        }
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: qsTr("Paste")
                        onTriggered: {
                            colorTextField.selectAll();
                            colorTextField.paste();
                        }
                        enabled: colorTextField.canPaste
                    }
                }
            }
        }
        ListModel {
            id: sliderBoxElements
            ListElement { labelText: "Red value"; colorChannel: 0 }
            ListElement { labelText: "Green value"; colorChannel: 1 }
            ListElement { labelText: "Blue value"; colorChannel: 2 }
            ListElement { labelText: "Alpha value"; colorChannel: 3 }
        }
        ColumnLayout {
            id: sliderBox
            Layout.columnSpan: 3
            Layout.rowSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true

            Repeater {
                model: sliderBoxElements
                delegate: ColumnLayout {
                    Label {
                        text: labelText
                    }
                    Slider {
                        id: colorSlider
                        property int channel: colorChannel
                        from: 0
                        to: 255
                        stepSize: 1
                        value: {
                            if (colorSlider.channel == 0)
                                return colorDialog.color.r * 255;
                            else if (colorSlider.channel == 1)
                                return colorDialog.color.g * 255;
                            else if (colorSlider.channel == 2)
                                return colorDialog.color.b * 255;
                            else if (colorSlider.channel == 3)
                                return colorDialog.color.a * 255;
                        }

                        Connections {
                            function onMoved() {
                                var redChannelValue = colorDialog.color.r;
                                var greenChannelValue = colorDialog.color.g;
                                var blueChannelValue = colorDialog.color.b;
                                var alphaChannelValue = colorDialog.color.a;

                                if (colorSlider.channel == 0)
                                    redChannelValue = colorSlider.value / 255;
                                else if (colorSlider.channel == 1)
                                    greenChannelValue = colorSlider.value / 255;
                                else if (colorSlider.channel == 2)
                                    blueChannelValue = colorSlider.value / 255;
                                else if (colorSlider.channel == 3)
                                    alphaChannelValue = colorSlider.value / 255;

                                colorDialog.color = Qt.rgba(redChannelValue, greenChannelValue, blueChannelValue, alphaChannelValue);
                            }
                        }
                    }
                }
            }
        }
        DialogButtonBox {
            id: dialogButtonBox
            Layout.columnSpan: 7
            Layout.alignment: Qt.AlignRight

            Button {
                text: qsTr("Apply")
                onClicked: accept()
            }
            Button {
                text: qsTr("Cancel")
                onClicked: reject()
            }
        }
    }
}
