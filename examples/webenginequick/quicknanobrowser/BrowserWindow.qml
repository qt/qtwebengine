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
    property BrowserTab currentTab: tabBar.currentIndex < tabBar.count ? tabLayout.children[tabBar.currentIndex] as BrowserTab : null
    property WebEngineView currentWebView: currentTab?.webView ?? null
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
        id: appSettings
        property bool autoLoadImages: WebEngine.settings.autoLoadImages
        property bool javaScriptEnabled: WebEngine.settings.javascriptEnabled
        property bool errorPageEnabled: WebEngine.settings.errorPageEnabled
        property bool pluginsEnabled: true
        property bool fullScreenSupportEnabled: WebEngine.settings.fullScreenSupportEnabled
        property bool autoLoadIconsForPage: WebEngine.settings.autoLoadIconsForPage
        property bool touchIconsEnabled: WebEngine.settings.touchIconsEnabled
        property bool webRTCPublicInterfacesOnly: WebEngine.settings.webRTCPublicInterfacesOnly
        property bool devToolsEnabled: false
        property bool pdfViewerEnabled: WebEngine.settings.pdfViewerEnabled
        property bool javascriptCanAccessClipboard: WebEngine.settings.javascriptCanAccessClipboard
        property bool javascriptCanPaste: WebEngine.settings.javascriptCanPaste
        property int imageAnimationPolicy: WebEngine.settings.imageAnimationPolicy
    }

    ActionManager {
        browserWindow: win
        webView: win.currentWebView
        downloads: downloadView
        toolBar: win.currentTab?.toolBar ?? null
        searchBar: findBar
        fullScreenNotifications: fullScreenNotification
        onNewTabRequested: tabBar.createTab(BrowserManager.defaultProfilePrototype.instance())
    }

    StackLayout {
        id: tabLayout
        currentIndex: tabBar.currentIndex

        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    BrowserTabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        viewStack: tabLayout
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
            tabs: tabBar
            searchBar: findBar
            certificateDialog: sslDialog
            permissionPrompt: permissionDialog
            webAuthenticationDialog: webAuthDialog
            imageAnimationPolicy: appSettings.imageAnimationPolicy
            toolBar.loadImagesChecked: appSettings.autoLoadImages
            toolBar.javaScriptEnabledChecked: appSettings.javaScriptEnabled
            toolBar.errorPageEnabledChecked: appSettings.errorPageEnabled
            toolBar.pluginsEnabledChecked: appSettings.pluginsEnabled
            toolBar.fullScreenSupportEnabledChecked: appSettings.fullScreenSupportEnabled
            toolBar.autoLoadIconsForPageChecked: appSettings.autoLoadIconsForPage
            toolBar.touchIconsEnabledChecked: appSettings.touchIconsEnabled
            toolBar.webRTCPublicInterfacesOnlyChecked: appSettings.webRTCPublicInterfacesOnly
            toolBar.devToolsEnabledChecked: appSettings.devToolsEnabled
            toolBar.pdfViewerEnabledChecked: appSettings.pdfViewerEnabled
            toolBar.javascriptCanAccessClipboardChecked: appSettings.javascriptCanAccessClipboard
            toolBar.javascriptCanPasteChecked: appSettings.javascriptCanPaste
            toolBar.disableImageAnimationChecked: appSettings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.Disallow
            toolBar.allowImageAnimationChecked: appSettings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.Allow
            toolBar.animateImageOnceChecked: appSettings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.AnimateOnce

            onSettingToggled: function(name, value) {
                appSettings[name] = value;
            }

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

            onDevToolsNewWindowRequested: function(request) {
                let tab = tabBar.createTab(win.currentWebView.profile);
                request.openIn(tab);
            }

            onFullScreenToggled: function(toggleOn) {
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
