// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtWebEngine

ColumnLayout {
    id: root

    signal closeRequested
    signal drawerRequested

    property int freezeDelay
    property int discardDelay

    property alias icon : view.icon
    property alias loading : view.loading
    property alias url : view.url
    property alias lifecycleState : view.lifecycleState
    property alias recommendedState : view.recommendedState

    readonly property string title : {
        if (view.url == "about:blank")
            return qsTr("New tab")
        if (view.title)
            return view.title
        return view.url
    }

    readonly property Action backAction: Action {
        property WebEngineAction webAction: view.action(WebEngineView.Back)
        enabled: webAction.enabled
        text: qsTr("Back")
        shortcut: root.visible && StandardKey.Back
        onTriggered: webAction.trigger()
    }

    readonly property Action forwardAction: Action {
        property WebEngineAction webAction: view.action(WebEngineView.Forward)
        enabled: webAction.enabled
        text: qsTr("Forward")
        shortcut: root.visible && StandardKey.Forward
        onTriggered: webAction.trigger()
    }

    readonly property Action reloadAction: Action {
        property WebEngineAction webAction: view.action(WebEngineView.Reload)
        enabled: webAction.enabled
        text: qsTr("Reload")
        shortcut: root.visible && StandardKey.Refresh
        onTriggered: webAction.trigger()
    }

    readonly property Action stopAction: Action {
        property WebEngineAction webAction: view.action(WebEngineView.Stop)
        enabled: webAction.enabled
        text: qsTr("Stop")
        shortcut: root.visible && StandardKey.Cancel
        onTriggered: webAction.trigger()
    }

    readonly property Action closeAction: Action {
        text: qsTr("Close")
        shortcut: root.visible && StandardKey.Close
        onTriggered: root.closeRequested()
    }

    readonly property Action activateAction: Action {
        text: qsTr("Active")
        checkable: true
        checked: view.lifecycleState == WebEngineView.LifecycleState.Active
        enabled: checked || (view.lifecycleState != WebEngineView.LifecycleState.Active)
        onTriggered: view.lifecycleState = WebEngineView.LifecycleState.Active
    }

    readonly property Action freezeAction: Action {
        text: qsTr("Frozen")
        checkable: true
        checked: view.lifecycleState == WebEngineView.LifecycleState.Frozen
        enabled: checked || (!view.visible && view.lifecycleState == WebEngineView.LifecycleState.Active)
        onTriggered: view.lifecycleState = WebEngineView.LifecycleState.Frozen
    }

    readonly property Action discardAction: Action {
        text: qsTr("Discarded")
        checkable: true
        checked: view.lifecycleState == WebEngineView.LifecycleState.Discarded
        enabled: checked || (!view.visible && view.lifecycleState == WebEngineView.LifecycleState.Frozen)
        onTriggered: view.lifecycleState = WebEngineView.LifecycleState.Discarded
    }

    spacing: 0

    ToolBar {
        Layout.fillWidth: true
        Material.elevation: 0
        Material.background: Material.color(Material.Grey, Material.Shade800)

        RowLayout {
            anchors.fill: parent
            WebToolButton {
                action: root.backAction
                text: "←"
                ToolTip.text: root.backAction.text
            }
            WebToolButton {
                action: root.forwardAction
                text: "→"
                ToolTip.text: root.forwardAction.text
            }
            WebToolButton {
                action: root.reloadAction
                visible: root.reloadAction.enabled
                text: "↻"
                ToolTip.text: root.reloadAction.text
            }
            WebToolButton {
                action: root.stopAction
                visible: root.stopAction.enabled
                text: "✕"
                ToolTip.text: root.stopAction.text
            }
            TextField {
                Layout.fillWidth: true
                Layout.topMargin: 6

                placeholderText: qsTr("Type a URL")
                text: view.url == "about:blank" ? "" : view.url
                selectByMouse: true

                onAccepted: { view.url = utils.fromUserInput(text) }
            }
            WebToolButton {
                text: "⋮"
                ToolTip.text: qsTr("Settings")
                onClicked: root.drawerRequested()
            }
        }
    }

    WebEngineView {
        id: view
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    Timer {
        interval: {
            switch (view.recommendedState) {
            case WebEngineView.LifecycleState.Active:
                return 1
            case WebEngineView.LifecycleState.Frozen:
                return root.freezeDelay * 1000
            case WebEngineView.LifecycleState.Discarded:
                return root.discardDelay * 1000
            }
        }
        running: interval && view.lifecycleState != view.recommendedState
        onTriggered: view.lifecycleState = view.recommendedState
    }
}
