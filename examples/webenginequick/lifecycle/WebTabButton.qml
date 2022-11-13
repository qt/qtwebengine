// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtWebEngine

TabButton {
    id: root

    property WebTab tab

    text: root.tab.title

    ToolTip.delay: 1000
    ToolTip.visible: root.hovered
    ToolTip.text: root.text

    padding: 6

    contentItem: RowLayout {
        Item {
            implicitWidth: 16
            implicitHeight: 16
            BusyIndicator {
                visible: root.tab.loading
                anchors.fill: parent
                leftInset: 0
                topInset: 0
                rightInset: 0
                bottomInset: 0
                padding: 0
            }
            Image {
                visible: !root.tab.loading
                source: root.tab.icon
                anchors.fill: parent
            }
        }
        Label {
            Layout.fillWidth: true
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            text: root.text
            elide: Text.ElideRight
            color: {
                switch (root.tab.lifecycleState) {
                case WebEngineView.LifecycleState.Active:
                    return Material.color(Material.Grey, Material.Shade100)
                case WebEngineView.LifecycleState.Frozen:
                    return Material.color(Material.Blue, Material.Shade400)
                case WebEngineView.LifecycleState.Discarded:
                    return Material.color(Material.Red, Material.Shade400)
                }
            }

        }
        WebToolButton {
            action: root.tab.closeAction
            text: "✕"
            ToolTip.text: action.text
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true
        onClicked: contextMenu.popup()
    }

    Menu {
        id: contextMenu
        Control {
            contentItem: Label {
                text: qsTr("Manual lifecycle control")
            }
            verticalPadding: 9
            horizontalPadding: 14
        }
        Repeater {
            model: [root.tab.activateAction, root.tab.freezeAction, root.tab.discardAction]
            RadioButton {
                action: modelData
                verticalPadding: 9
                horizontalPadding: 14
            }
        }
        Control {
            contentItem: Label {
                text: qsTr("Recommended: %1").arg(recommendedStateText)
                property string recommendedStateText: {
                    switch (root.tab.recommendedState) {
                    case WebEngineView.LifecycleState.Active:
                        return root.tab.activateAction.text
                    case WebEngineView.LifecycleState.Frozen:
                        return root.tab.freezeAction.text
                    case WebEngineView.LifecycleState.Discarded:
                        return root.tab.discardAction.text
                    }
                }
                color: Material.hintTextColor
            }
            font.pointSize: 8
            verticalPadding: 9
            horizontalPadding: 14
        }
    }
}
