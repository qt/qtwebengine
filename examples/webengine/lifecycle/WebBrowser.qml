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
import QtQuick.Window 2.12

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
