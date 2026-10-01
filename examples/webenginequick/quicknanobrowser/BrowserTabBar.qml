// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {
    id: root
    property alias currentIndex: tabBar.currentIndex
    readonly property alias count: tabBar.count
    padding: 0

    signal newTabRequested()
    signal tabCloseRequested(int index)

    function addTabButton(tab) {
        var newTabButton = tabButtonFactory.createObject(tabBar, {tabTitle: Qt.binding(function () { return tab.webView.title; })});
        tab.index = Qt.binding(function () { return newTabButton.TabBar.index; })
        tabBar.addItem(newTabButton);
        return newTabButton;
    }

    function removeTabButton(index) {
        tabBar.removeItem(tabBar.itemAt(index));
    }

    function setCurrentIndex(index) {
        tabBar.setCurrentIndex(index);
    }

    RowLayout {
        spacing: 0
        anchors.fill: parent

        ToolButton {
            Layout.bottomMargin: 2
            font.pointSize: 12
            font.bold: true
            implicitWidth: 32
            implicitHeight: 32
            text: "+"
            onClicked: root.newTabRequested()
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }

    Component {
        id: tabButtonFactory

        TabButton {
            id: tabButton
            property string tabTitle: "New Tab"

            contentItem: RowLayout {
                Label {
                    id: tabText
                    Layout.fillWidth: true
                    Layout.leftMargin: 4
                    Layout.rightMargin: 4
                    text: tabButton.tabTitle
                    elide: Text.ElideRight
                }
                ToolButton {
                    id: button
                    font.pointSize: 12
                    font.bold: true
                    implicitWidth: 32
                    implicitHeight: 32
                    text: "x"
                    onClicked: tabButton.closeTab()
                }
            }

            function closeTab() {
                root.tabCloseRequested(TabBar.index);
            }
        }
    }
}
