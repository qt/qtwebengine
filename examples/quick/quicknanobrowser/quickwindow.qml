/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

import QtQuick 2.1
import QtWebEngine 1.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: browserWindow
    height: 600
    width: 800
    visible: true
    title: webEngineView.title

    // Focus and select text in URL bar
    Action {
        id: focus
        shortcut: "Ctrl+L" // How to have Cmd + L on Mac ?
        onTriggered: {
            addressBar.forceActiveFocus();
            addressBar.selectAll();
        }
    }

    toolBar: ToolBar {
        id: navigationBar
            RowLayout {
                anchors.fill: parent;
                ToolButton {
                    id: backButton
                    iconSource: "icons/go-previous.png"
                    onClicked: webEngineView.goBack()
                    enabled: webEngineView.canGoBack
                }
                ToolButton {
                    id: forwardButton
                    iconSource: "icons/go-next.png"
                    onClicked: webEngineView.goForward()
                    enabled: webEngineView.canGoForward
                }
                ToolButton {
                    id: reloadButton
                    iconSource: webEngineView.loading ? "icons/process-stop.png" : "icons/view-refresh.png"
                    onClicked: webEngineView.reload()
                }
                TextField {
                    id: addressBar
                    Image {
                        anchors.verticalCenter: addressBar.verticalCenter;
                        x: 5
                        z: 2
                        id: faviconImage
                        width: 16; height: 16
                    }
                    style: TextFieldStyle {
                        padding {
                            left: 26;
                        }
                    }
                    focus: true
                    Layout.fillWidth: true
                    onAccepted: webEngineView.url = utils.fromUserInput(text)
                }
            }
            ProgressBar {
                id: progressBar
                height: 3
                anchors {
                    left: parent.left
                    top: parent.bottom
                    right: parent.right
                    leftMargin: -parent.leftMargin
                    rightMargin: -parent.rightMargin
                }
                style: ProgressBarStyle {
                    background: Item {}
                }
                z: -2;
                minimumValue: 0
                maximumValue: 100
            }
    }
    WebEngineView {
        id: webEngineView
        focus: true
        anchors.fill: parent
        url: utils.initialUrl()
        contextMenuExtraItems: ContextMenuExtras {}

        onUrlChanged: addressBar.text = url
        onIconChanged: faviconImage.source = icon
        onLoadProgressChanged: progressBar.value = loadProgress
    }
}
