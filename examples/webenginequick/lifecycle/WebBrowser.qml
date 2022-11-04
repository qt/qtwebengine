// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: root

    readonly property Action newTabAction: Action {
        text: qsTr("New tab")
        shortcut: StandardKey.AddTab
        onTriggered: root.createNewTab({url: "about:blank"})
    }

    visible: true
    width: Screen.width * 0.5
    height: Screen.height * 0.5
    title: tabStack.currentTab ? tabStack.currentTab.title : ""

    header: WebTabBar {
        id: tabBar

        z: 1

        newTabAction: root.newTabAction
    }

    WebTabStack {
        id: tabStack

        z: 0
        anchors.fill: parent

        currentIndex: tabBar.currentIndex
        freezeDelay: freezeSpin.enabled && freezeSpin.value
        discardDelay: discardSpin.enabled && discardSpin.value

        onCloseRequested: function(index) {
            root.closeTab(index)
        }

        onDrawerRequested: drawer.toggle()
    }

    Drawer {
        id: drawer

        edge: Qt.RightEdge
        height: root.height

        Control {
            padding: 16
            contentItem: ColumnLayout {
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Settings")
                    font.capitalization: Font.AllUppercase
                }
                MenuSeparator {}
                CheckBox {
                    id: lifecycleCheck
                    text: qsTr("Automatic lifecycle control")
                    checked: true
                }
                CheckBox {
                    id: freezeCheck
                    text: qsTr("Freeze after delay (seconds)")
                    enabled: lifecycleCheck.checked
                    checked: true
                }
                SpinBox {
                    id: freezeSpin
                    editable: true
                    enabled: freezeCheck.checked
                    value: 60
                    from: 1
                    to: 60*60
                }
                CheckBox {
                    id: discardCheck
                    text: qsTr("Discard after delay (seconds)")
                    enabled: lifecycleCheck.checked
                    checked: true
                }
                SpinBox {
                    id: discardSpin
                    editable: true
                    enabled: discardCheck.checked
                    value: 60*60
                    from: 1
                    to: 60*60
                }
            }
        }

        function toggle() {
            if (drawer.visible)
                drawer.close()
            else
                drawer.open()
        }
    }

    Component.onCompleted: {
        createNewTab({url: "https://www.qt.io"})
    }

    function createNewTab(properties) {
        const tab = tabStack.createNewTab(properties)
        tabBar.createNewTab({tab: tab})
        tabBar.currentIndex = tab.index
        return tab
    }

    function closeTab(index) {
        if (tabStack.count == 1)
            Qt.quit()
        tabBar.closeTab(index)
        tabStack.closeTab(index)
    }
}
