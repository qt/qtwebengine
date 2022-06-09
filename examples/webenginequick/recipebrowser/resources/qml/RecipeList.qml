// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

FocusScope {
    id: root
    signal recipeSelected(url url)

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        ToolBar {
            id: headerBackground
            Layout.fillWidth: true
            implicitHeight: headerText.height + 20

            Label {
                id: headerText
                width: parent.width
                text: qsTr("Favorite recipes")
                padding: 10
                anchors.centerIn: parent
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            keyNavigationWraps: true
            clip: true
            focus: true
            ScrollBar.vertical: ScrollBar { }

            model: recipeModel

            delegate: ItemDelegate {
                width: parent.width
                text: model.name
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: parent.enabled ? parent.Material.primaryTextColor
                                          : parent.Material.hintTextColor
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }

                property url url: model.url
                highlighted: ListView.isCurrentItem

                onClicked: {
                    listView.forceActiveFocus()
                    listView.currentIndex = model.index
                }
            }

            onCurrentItemChanged: {
                root.recipeSelected(currentItem.url)
            }

            ListModel {
                id: recipeModel

                ListElement {
                    name: "Pizza Diavola"
                    url: "qrc:///pages/pizza.html"
                }
                ListElement {
                    name: "Steak"
                    url: "qrc:///pages/steak.html"
                }
                ListElement {
                    name: "Burger"
                    url: "qrc:///pages/burger.html"
                }
                ListElement {
                    name: "Soup"
                    url: "qrc:///pages/soup.html"
                }
                ListElement {
                    name: "Pasta"
                    url: "qrc:///pages/pasta.html"
                }
                ListElement {
                    name: "Grilled Skewers"
                    url: "qrc:///pages/skewers.html"
                }
                ListElement {
                    name: "Cupcakes"
                    url: "qrc:///pages/cupcakes.html"
                }
            }

            ToolTip {
                id: help
                implicitWidth: root.width - padding * 3
                y: root.y + root.height
                delay: 1000
                timeout: 5000
                text: qsTr("Use keyboard, mouse, or touch controls to navigate through the\
                            recipes.")

                contentItem: Text {
                    text: help.text
                    font: help.font
                    color: help.Material.primaryTextColor
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    function showHelp() {
        help.open()
    }
}

