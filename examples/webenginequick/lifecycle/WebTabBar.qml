// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {
    id: root

    property Action newTabAction

    property alias currentIndex: tabBar.currentIndex

    signal closeRequested(int index)

    Material.background: Material.color(Material.Grey, Material.Shade900)
    Material.elevation: 4
    padding: 0

    RowLayout {
        spacing: 0
        anchors.fill: parent

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        WebToolButton {
            Layout.bottomMargin: 2

            action: root.newTabAction
            text: "+"
            ToolTip.text: root.newTabAction.text
        }
    }

    Component {
        id: factory
        WebTabButton {}
    }

    function createNewTab(properties) {
        return factory.createObject(tabBar, properties)
    }

    function closeTab(index) {
        tabBar.takeItem(index).destroy()
    }
}
