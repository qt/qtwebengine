// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtWebEngine

TabBar {
    id: root
    required property StackLayout viewStack
    required property Component webViewComponent

    signal lastTabClosed()

    Component.onCompleted: createTab(BrowserManager.defaultProfilePrototype.instance())

    function createTab(profile, focusOnNewTab = true, url = undefined) {
        var tab = root.webViewComponent.createObject(root.viewStack, {profile: profile}) as BrowserTab;
        var webview = tab.webView;
        var newTabButton = tabButtonComponent.createObject(root, {tabTitle: Qt.binding(function () { return webview.title; })});
        tab.index = Qt.binding(function () { return newTabButton.TabBar.index; })
        root.addItem(newTabButton);
        if (focusOnNewTab) {
            root.setCurrentIndex(root.count - 1);
        }
        if (url !== undefined) {
            webview.url = url;
        }
        return webview;
    }

    function tryCloseView(index) {
        (root.viewStack.children[index] as BrowserTab).webView.triggerWebAction(WebEngineView.RequestClose);
    }

    function removeView(index) {
        if (root.count > 1) {
            root.removeItem(root.itemAt(index));
            root.viewStack.children[index].destroy();
        } else {
            root.lastTabClosed();
        }
    }

    Component {
        id: tabButtonComponent

        TabButton {
            id: tabButton
            property color frameColor: "#999999"
            property color fillColor: "#eeeeee"
            property color nonSelectedColor: "#dddddd"
            property string tabTitle: "New Tab"

            contentItem: Rectangle {
                id: tabRectangle
                color: tabButton.down ? tabButton.fillColor : tabButton.nonSelectedColor
                border.width: 1
                border.color: tabButton.frameColor
                implicitWidth: Math.max(tabText.implicitWidth + 30, 80)
                implicitHeight: Math.max(tabText.implicitHeight + 10, 20)
                Rectangle { height: 1 ; width: parent.width ; color: tabButton.frameColor}
                Rectangle { height: parent.height ; width: 1; color: tabButton.frameColor}
                Rectangle { x: parent.width - 2; height: parent.height ; width: 1; color: tabButton.frameColor}
                Text {
                    id: tabText
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 6
                    text: tabButton.tabTitle
                    elide: Text.ElideRight
                    color: tabButton.down ? "black" : tabButton.frameColor
                    width: parent.width - button.background.width
                }
                Button {
                    id: button
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 4
                    height: 12
                    background: Rectangle {
                        implicitWidth: 12
                        implicitHeight: 12
                        color: button.hovered ? "#cccccc" : tabRectangle.color
                        Text {text: "x"; anchors.centerIn: parent; color: "gray"}
                    }
                    onClicked: tabButton.closeTab()
                }
            }

            function closeTab() {
                root.tryCloseView(TabBar.index);
            }
        }
    }
}
