// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtCore
import QtQml
import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Window
import QtWebEngine
import BrowserUtils

ApplicationWindow {
    id: win
    property WebEngineView currentWebView: tabBar.currentIndex < tabBar.count ? tabLayout.children[tabBar.currentIndex] : null
    property int previousVisibility: Window.Windowed
    property bool lastTabClosing: false

    width: 1300
    height: 900
    visible: true
    title: win.currentWebView?.title ?? ""

    // Make sure the Qt.WindowFullscreenButtonHint is set on OS X.
    Component.onCompleted: flags = flags | Qt.WindowFullscreenButtonHint

    onCurrentWebViewChanged: {
        findBar.reset();
    }

    Settings {
        id : appSettings
        property alias autoLoadImages: navigationBar.loadImagesChecked
        property alias javaScriptEnabled: navigationBar.javaScriptEnabledChecked
        property alias errorPageEnabled: navigationBar.errorPageEnabledChecked
        property alias pluginsEnabled: navigationBar.pluginsEnabledChecked
        property alias fullScreenSupportEnabled: navigationBar.fullScreenSupportEnabledChecked
        property alias autoLoadIconsForPage: navigationBar.autoLoadIconsForPageChecked
        property alias touchIconsEnabled: navigationBar.touchIconsEnabledChecked
        property alias webRTCPublicInterfacesOnly : navigationBar.webRTCPublicInterfacesOnlyChecked
        property alias devToolsEnabled: navigationBar.devToolsEnabledChecked
        property alias pdfViewerEnabled: navigationBar.pdfViewerEnabledChecked
        property alias javascriptCanAccessClipboard: navigationBar.javascriptCanAccessClipboardChecked
        property alias javascriptCanPaste: navigationBar.javascriptCanPasteChecked
        property int imageAnimationPolicy: {
           return navigationBar.animateImageOnceChecked ? WebEngineSettings.ImageAnimationPolicy.AnimateOnce :
                  navigationBar.allowImageAnimationChecked ? WebEngineSettings.ImageAnimationPolicy.Allow :
                  navigationBar.disableImageAnimationChecked ? WebEngineSettings.ImageAnimationPolicy.Disallow :
                  WebEngineSettings.ImageAnimationPolicy.AnimateOnce
          }
        }

    ActionManager {
        browserWindow: win
        webView: win.currentWebView
        downloads: downloadView
        toolBar: navigationBar
        searchBar: findBar
        fullScreenNotifications: fullScreenNotification
        onNewTabRequested: tabBar.createTab(BrowserManager.defaultProfilePrototype.instance())
    }

    menuBar: WebToolBar {
        id: navigationBar
        currentWebView: win.currentWebView
    }

    StackLayout {
        id: tabLayout
        currentIndex: tabBar.currentIndex

        anchors.top: tabBar.bottom
        anchors.bottom: devToolsWebEngineView.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    BrowserTabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        viewStack: tabLayout
        toolBar: navigationBar
        webViewComponent: tabComponent
        onLastTabClosed: {
            win.lastTabClosing = true;
            win.close();
        }
    }

    Component {
        id: tabComponent
        BrowserTab {
            id: webEngineView
            toolBar: navigationBar
            tabs: tabBar
            searchBar: findBar
            certificateDialog: sslDialog
            permissionPrompt: permissionDialog
            webAuthenticationDialog: webAuthDialog
            imageAnimationPolicy: appSettings.imageAnimationPolicy
            states: [
                State {
                    name: "FullScreen"
                    PropertyChanges {
                        tabBar.visible: false
                        tabBar.height: 0
                        navigationBar.visible: false
                    }
                 }
            ]

            onLinkHovered: function(hoveredUrl) {
                if (hoveredUrl === "")
                    hideStatusText.start();
                else {
                    statusText.text = hoveredUrl;
                    statusBubble.visible = true;
                    hideStatusText.stop();
                }
            }

            onWindowCloseRequested: function() {
                tabBar.removeView(webEngineView.index);
            }

            onNewWindowRequested: function(request) {
                if (!request.userInitiated)
                    console.warn("Blocked a popup window.");
                else if (request.destination === WebEngineNewWindowRequest.InNewTab) {
                    let tab = tabBar.createTab(win.currentWebView.profile, true, request.requestedUrl);
                    tab.acceptAsNewWindow(request);
                } else if (request.destination === WebEngineNewWindowRequest.InNewBackgroundTab) {
                    let backgroundTab = tabBar.createTab(win.currentWebView.profile, false);
                    backgroundTab.acceptAsNewWindow(request);
                } else if (request.destination === WebEngineNewWindowRequest.InNewDialog) {
                    let dialog = BrowserManager.createDialog(win.currentWebView.profile);
                    dialog.currentWebView.acceptAsNewWindow(request);
                } else {
                    let window = BrowserManager.createWindow(win.currentWebView.profile);
                    window.currentWebView.acceptAsNewWindow(request);
                }
            }


            onFullScreenToggled: function(toggleOn) {
                webEngineView.state = toggleOn ? "FullScreen" : "";
                if (toggleOn) {
                    win.previousVisibility = win.visibility;
                    win.showFullScreen();
                    fullScreenNotification.show();
                } else {
                    win.visibility = win.previousVisibility;
                    fullScreenNotification.hide();
                }
            }
        }
    }
    WebEngineView {
        id: devToolsWebEngineView
        visible: appSettings.devToolsEnabled
        height: visible ? 400 : 0
        inspectedView: visible && tabBar.currentIndex < tabBar.count ? tabLayout.children[tabBar.currentIndex] : null
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        onNewWindowRequested: function(request) {
            var tab = tabBar.createTab(win.currentWebView.profile);
            request.openIn(tab);
        }

        onWindowCloseRequested: function() {
            // Delay hiding for keep the inspectedView set to receive the ACK message of close.
            Qt.callLater(function() { appSettings.devToolsEnabled = false })
        }
    }
    SslDialog {
        id: sslDialog
        anchors.centerIn: parent
    }
    PermissionDialog {
        id: permissionDialog
        anchors.centerIn: parent
        width: Math.min(win.width, win.height) / 3 * 2
    }

    FullScreenNotification {
        id: fullScreenNotification
    }

    DownloadView {
        id: downloadView
        visible: false
        anchors.fill: parent
    }

    WebAuthDialog {
        id: webAuthDialog
        visible: false
        width: Math.min(win.width, win.height) / 3 * 2
    }

    MessageDialog {
        id: downloadAcceptDialog
        property var downloadRequest: downloadView.pendingDownloadRequest
        title: "Download requested"
        text: downloadRequest ? downloadRequest.suggestedFileName : ""
        buttons: Dialog.No | Dialog.Yes
        onAccepted: {
            downloadView.visible = true;
            downloadView.append(downloadRequest);
            downloadRequest.accept();
        }
        onRejected: {
            downloadRequest.cancel();
        }
        onButtonClicked: {
            visible = false;
        }
        visible: false
    }

    function onDownloadRequested(download) {
        downloadView.pendingDownloadRequest = download;
        downloadAcceptDialog.visible = true;
    }

    FindBar {
        id: findBar
        visible: false
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top

        onFindNext: {
            if (text)
                win.currentWebView?.findText(text);
            else if (!visible)
                visible = true;
        }
        onFindPrevious: {
            if (text)
                win.currentWebView?.findText(text, WebEngineView.FindBackward);
            else if (!visible)
                visible = true;
        }
    }


    Rectangle {
        id: statusBubble
        color: "oldlace"
        property int padding: 8
        visible: false

        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: statusText.paintedWidth + padding
        height: statusText.paintedHeight + padding

        Text {
            id: statusText
            anchors.centerIn: statusBubble
            elide: Qt.ElideMiddle

            Timer {
                id: hideStatusText
                interval: 750
                onTriggered: {
                    statusText.text = "";
                    statusBubble.visible = false;
                }
            }
        }
    }

    onClosing: function(closeEvent) {
       if (lastTabClosing) {
           return;
       }
       closeEvent.accepted = false
       for (let i = 0; i < tabBar.count; i++)  {
           tabBar.tryCloseView(i);
       }
    }
}
