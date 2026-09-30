// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtCore
import QtQml
import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Dialogs
import QtQuick.Window
import QtWebEngine
import BrowserUtils

ApplicationWindow {
    id: win
    property BrowserTab currentTab: tabLayout.currentTab
    property WebEngineView currentWebView: currentTab?.webView ?? null
    property int previousVisibility: Window.Windowed
    property bool lastTabClosing: false

    width: 1300
    height: 900
    visible: true
    title: win.currentWebView?.title ?? ""


    header: BrowserTabBar {
        id: tabBar
        onTabCloseRequested: index => tabLayout.tryCloseTab(index)
    }

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
        onNewTabRequested: tabLayout.createTab(BrowserManager.defaultProfilePrototype.instance())
    }

    BrowserStackLayout {
        id: tabLayout
        anchors.fill: parent
        tabs: tabBar
        searchBar: findBar
        certificateDialog: sslDialog
        permissionPrompt: permissionDialog
        webAuthenticationDialog: webAuthDialog
        browserSettings: appSettings

        onLinkHovered: function(hoveredUrl) {
            if (hoveredUrl === "")
                hideStatusText.start();
            else {
                statusText.text = hoveredUrl;
                statusBubble.visible = true;
                hideStatusText.stop();
            }
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

        onLastTabClosed: {
            win.lastTabClosing = true;
            win.close();
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
           tabLayout.tryCloseTab(i);
       }
    }
}
