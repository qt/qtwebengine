// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtWebEngine
import QtQuick.Layouts
import QtQuick.Controls

ApplicationWindow {
    width: 1024
    height: 750
    visible: true
    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                property int itemAction: WebEngineView.Back
                text: webEngineView.action(itemAction).text
                enabled: webEngineView.action(itemAction).enabled
                onClicked: webEngineView.action(itemAction).trigger()
                icon.name: webEngineView.action(itemAction).iconName
                display: AbstractButton.TextUnderIcon
            }

            ToolButton {
                property int itemAction: WebEngineView.Forward
                text: webEngineView.action(itemAction).text
                enabled: webEngineView.action(itemAction).enabled
                onClicked: webEngineView.action(itemAction).trigger()
                icon.name: webEngineView.action(itemAction).iconName
                display: AbstractButton.TextUnderIcon
            }

            ToolButton {
                property int itemAction: webEngineView.loading ? WebEngineView.Stop : WebEngineView.Reload
                text: webEngineView.action(itemAction).text
                enabled: webEngineView.action(itemAction).enabled
                onClicked: webEngineView.action(itemAction).trigger()
                icon.name: webEngineView.action(itemAction).iconName
                display: AbstractButton.TextUnderIcon
            }

            TextField {
                Layout.fillWidth: true
                text: webEngineView.url
                selectByMouse: true
                onEditingFinished: webEngineView.url = text
            }

            Label { text: 'Handle: ' }
            ComboBox {
                model: [ 'Default', 'Circle', 'Square' ]

                onCurrentValueChanged: {
                    if (currentValue == 'Circle')
                        webEngineView.touchHandleDelegate = circleTouchHandle
                    else if (currentValue == 'Square')
                        webEngineView.touchHandleDelegate = rectTouchHandle
                    else
                        webEngineView.touchHandleDelegate = null
                }

                Component.onCompleted: currentIndex = indexOfValue('Square')
            }
        }
    }

    Component {
        id: circleTouchHandle
        Rectangle {
            color: "blue"
            border.color: "black"
            border.width: 2
            radius: 50
        }
    }

    Component {
        id: rectTouchHandle
        Rectangle {
            border.color: "black"
            border.width: 2
            radius: 2
            onVisibleChanged: if (visible) { color = 'yellow'; cAnim.restart(); }
            ColorAnimation on color { id: cAnim; to: 'red'; duration: 1000 }
        }
    }

    WebEngineView {
        anchors.fill: parent
        id: webEngineView
        url: "https://www.qt.io"
    }
}
