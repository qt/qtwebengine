// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

TabBar {
    id: root

    signal tabCloseRequested(int index)

    function addTabButton(tab) {
        var newTabButton = tabButtonComponent.createObject(root, {tabTitle: Qt.binding(function () { return tab.webView.title; })});
        tab.index = Qt.binding(function () { return newTabButton.TabBar.index; })
        root.addItem(newTabButton);
        return newTabButton;
    }

    Component {
        id: tabButtonComponent

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
