/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import QtWebEngine 1.10

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
