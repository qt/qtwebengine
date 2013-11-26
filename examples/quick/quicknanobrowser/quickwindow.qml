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
import QtWebEngine.experimental 1.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1

ApplicationWindow {
    id: browserWindow
    function load(url) { tabs.currentView.url = url }
    function adoptHandle(viewHandle) { tabs.currentView.adoptHandle(viewHandle) }

    property bool isFullScreen: visibility == Window.FullScreen
    onIsFullScreenChanged: {
        // This is for the case where the system forces us to leave fullscreen.
        if (!isFullScreen && tabs.currentView.state == "FullScreen")
            tabs.currentView.state = ""
    }

    height: 600
    width: 800
    visible: true
    title: tabs.currentView && tabs.currentView.title

    // Make sure the Qt.WindowFullscreenButtonHint is set on Mac.
    Component.onCompleted: flags = flags | Qt.WindowFullscreenButtonHint

    Action {
        id: focus
        shortcut: "Ctrl+L"
        onTriggered: {
            addressBar.forceActiveFocus();
            addressBar.selectAll();
        }
    }
    Action {
        shortcut: "Ctrl+T"
        onTriggered: {
            tabs.createEmptyTab()
            addressBar.forceActiveFocus();
            addressBar.selectAll();
        }
    }
    Action {
        shortcut: "Ctrl+W"
        onTriggered: {
            if (tabs.count == 1)
                browserWindow.close()
            else
                tabs.removeTab(tabs.currentIndex)
        }
    }

    Action {
        shortcut: "Escape"
        onTriggered: {
            if (browserWindow.isFullScreen)
                browserWindow.showNormal()
        }
    }

    toolBar: ToolBar {
        id: navigationBar
            RowLayout {
                anchors.fill: parent;
                ToolButton {
                    id: backButton
                    iconSource: "icons/go-previous.png"
                    onClicked: tabs.currentView.goBack()
                    enabled: tabs.currentView && tabs.currentView.canGoBack
                }
                ToolButton {
                    id: forwardButton
                    iconSource: "icons/go-next.png"
                    onClicked: tabs.currentView.goForward()
                    enabled: tabs.currentView && tabs.currentView.canGoForward
                }
                ToolButton {
                    id: reloadButton
                    iconSource: tabs.currentView && tabs.currentView.loading ? "icons/process-stop.png" : "icons/view-refresh.png"
                    onClicked: tabs.currentView.reload()
                }
                TextField {
                    id: addressBar
                    Image {
                        anchors.verticalCenter: addressBar.verticalCenter;
                        x: 5
                        z: 2
                        id: faviconImage
                        width: 16; height: 16
                        source: tabs.currentView && tabs.currentView.icon
                    }
                    style: TextFieldStyle {
                        padding {
                            left: 26;
                        }
                    }
                    focus: true
                    Layout.fillWidth: true
                    text: tabs.currentView && tabs.currentView.url
                    onAccepted: tabs.currentView.url = utils.fromUserInput(text)
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
                value: (tabs.currentView && tabs.currentView.loadProgress < 100) ? tabs.currentView.loadProgress : 0
            }
    }

    TabView {
        id: tabs
        property Item currentView: currentIndex < count ? getTab(currentIndex).item : null
        function createEmptyTab() {
            var tab = addTab("", tabComponent)
            // We must do this first to make sure that tab.active gets set so that tab.item gets instantiated immediately.
            tabs.currentIndex = tabs.count - 1
            tab.title = Qt.binding(function() { return tab.item.title })
            return tab
        }

        anchors.fill: parent
        Component.onCompleted: createEmptyTab()

        Component {
            id: tabComponent
            WebEngineView {
                id: webEngineView
                function adoptHandle(viewHandle) { experimental.adoptHandle(viewHandle) }

                focus: true

                states: [
                    State {
                        name: "FullScreen"
                        PropertyChanges {
                            target: tabs
                            frameVisible: false
                            tabsVisible: false
                        }
                        PropertyChanges {
                            target: navigationBar
                            visible: false
                        }
                    }
                ]

                experimental {
                    isFullScreen: webEngineView.state == "FullScreen" && browserWindow.isFullScreen
                    onFullScreenRequested: {
                        if (fullScreen) {
                            webEngineView.state = "FullScreen"
                            browserWindow.showFullScreen();
                        } else {
                            webEngineView.state = ""
                            browserWindow.showNormal();
                        }
                    }

                    onCreateWindow: {
                        if (newViewDisposition == "popup")
                            print("Warning: Ignored a popup window.")
                        else if (newViewDisposition == "tab") {
                            var tab = tabs.createEmptyTab()
                            tab.item.adoptHandle(newViewHandle)
                        } else {
                            var component = Qt.createComponent("quickwindow.qml")
                            var window = component.createObject()
                            window.adoptHandle(newViewHandle)
                        }
                    }
                    extraContextMenuEntriesComponent: ContextMenuExtras {}
                }
            }
        }
    }
}
