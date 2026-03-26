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

    Action {
        shortcut: "Ctrl+D"
        onTriggered: {
            downloadView.visible = !downloadView.visible;
        }
    }
    Action {
        id: focusAction
        shortcut: "Ctrl+L"
        onTriggered: {
            navigationBar.addressBar.forceActiveFocus();
            navigationBar.addressBar.selectAll();
        }
    }
    Action {
        shortcut: StandardKey.Refresh
        onTriggered: {
            if (win.currentWebView)
                win.currentWebView.reload();
        }
    }
    Action {
        shortcut: StandardKey.AddTab
        onTriggered: {
            tabBar.createTab(tabBar.count !== 0
                             ? win.currentWebView.profile
                             : BrowserManager.defaultProfilePrototype.instance());
            navigationBar.addressBar.forceActiveFocus();
            navigationBar.addressBar.selectAll();
        }
    }
    Action {
        shortcut: StandardKey.Close
        onTriggered: {
            win.currentWebView.triggerWebAction(WebEngineView.RequestClose);
        }
    }
    Action {
        shortcut: StandardKey.Quit
        onTriggered: win.close()
    }
    Action {
        shortcut: "Escape"
        onTriggered: {
            if (win.currentWebView.state === "FullScreen") {
                win.visibility = win.previousVisibility;
                fullScreenNotification.hide();
                win.currentWebView.triggerWebAction(WebEngineView.ExitFullScreen);
            }

            if (findBar.visible)
                findBar.visible = false;
        }
    }
    Action {
        shortcut: "Ctrl+0"
        onTriggered: win.currentWebView.zoomFactor = 1.0
    }
    Action {
        shortcut: StandardKey.ZoomOut
        onTriggered: win.currentWebView.zoomFactor -= 0.1
    }
    Action {
        shortcut: StandardKey.ZoomIn
        onTriggered: win.currentWebView.zoomFactor += 0.1
    }

    Action {
        shortcut: StandardKey.Copy
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.Copy)
    }
    Action {
        shortcut: StandardKey.Cut
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.Cut)
    }
    Action {
        shortcut: StandardKey.Paste
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.Paste)
    }
    Action {
        shortcut: "Shift+"+StandardKey.Paste
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.PasteAndMatchStyle)
    }
    Action {
        shortcut: StandardKey.SelectAll
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.SelectAll)
    }
    Action {
        shortcut: StandardKey.Undo
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.Undo)
    }
    Action {
        shortcut: StandardKey.Redo
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.Redo)
    }
    Action {
        shortcut: StandardKey.Back
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.Back)
    }
    Action {
        shortcut: StandardKey.Forward
        onTriggered: win.currentWebView.triggerWebAction(WebEngineView.Forward)
    }
    Action {
        shortcut: StandardKey.Find
        onTriggered: {
            if (!findBar.visible)
                findBar.visible = true;
        }
    }
    Action {
        shortcut: StandardKey.FindNext
        onTriggered: findBar.findNext()
    }
    Action {
        shortcut: StandardKey.FindPrevious
        onTriggered: findBar.findPrevious()
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
                implicitWidth: Math.max(tabText.width + 30, 80)
                implicitHeight: Math.max(tabText.height + 10, 20)
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

            onClicked: navigationBar.addressBar.text = (tabLayout.itemAt(TabBar.index) as WebEngineView).url;
            function closeTab() {
                tabBar.tryCloseView(TabBar.index);
            }
        }
    }

    TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        Component.onCompleted: createTab(BrowserManager.defaultProfilePrototype.instance())

        function createTab(profile, focusOnNewTab = true, url = undefined) {
            var webview = tabComponent.createObject(tabLayout, {profile: profile});
            var newTabButton = tabButtonComponent.createObject(tabBar, {tabTitle: Qt.binding(function () { return webview.title; })});
            webview.index = Qt.binding(function () { return newTabButton.TabBar.index; })
            tabBar.addItem(newTabButton);
            if (focusOnNewTab) {
                tabBar.setCurrentIndex(tabBar.count - 1);
            }
            if (url !== undefined) {
                webview.url = url;
            }
            return webview;
        }

        function tryCloseView(index) {
            tabLayout.children[index].triggerWebAction(WebEngineView.RequestClose);
        }

        function removeView(index) {
            if (tabBar.count > 1) {
                tabBar.removeItem(tabBar.itemAt(index));
                tabLayout.children[index].destroy();
            } else {
                win.lastTabClosing = true;
                win.close();
            }
        }

        Component {
            id: tabComponent
            WebEngineView {
                id: webEngineView
                property int index: 0
                focus: true

                onLinkHovered: function(hoveredUrl) {
                    if (hoveredUrl === "")
                        hideStatusText.start();
                    else {
                        statusText.text = hoveredUrl;
                        statusBubble.visible = true;
                        hideStatusText.stop();
                    }
                }

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
                settings.localContentCanAccessRemoteUrls: true
                settings.localContentCanAccessFileUrls: false
                settings.autoLoadImages: appSettings.autoLoadImages
                settings.javascriptEnabled: appSettings.javaScriptEnabled
                settings.errorPageEnabled: appSettings.errorPageEnabled
                settings.pluginsEnabled: appSettings.pluginsEnabled
                settings.fullScreenSupportEnabled: appSettings.fullScreenSupportEnabled
                settings.autoLoadIconsForPage: appSettings.autoLoadIconsForPage
                settings.touchIconsEnabled: appSettings.touchIconsEnabled
                settings.webRTCPublicInterfacesOnly: appSettings.webRTCPublicInterfacesOnly
                settings.pdfViewerEnabled: appSettings.pdfViewerEnabled
                settings.imageAnimationPolicy: appSettings.imageAnimationPolicy
                settings.screenCaptureEnabled: true
                settings.javascriptCanAccessClipboard: appSettings.javascriptCanAccessClipboard
                settings.javascriptCanPaste: appSettings.javascriptCanPaste

                onWindowCloseRequested: function() {
                    tabBar.removeView(webEngineView.index);
                }

                onCertificateError: function(error) {
                    if (!error.isMainFrame) {
                        error.rejectCertificate();
                        return;
                    }

                    error.defer();
                    sslDialog.enqueue(error);
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

                onFullScreenRequested: function(request) {
                    if (request.toggleOn) {
                        webEngineView.state = "FullScreen";
                        win.previousVisibility = win.visibility;
                        win.showFullScreen();
                        fullScreenNotification.show();
                    } else {
                        webEngineView.state = "";
                        win.visibility = win.previousVisibility;
                        fullScreenNotification.hide();
                    }
                    request.accept();
                }

                onRegisterProtocolHandlerRequested: function(request) {
                    console.log("accepting registerProtocolHandler request for "
                                + request.scheme + " from " + request.origin);
                    request.accept();
                }

                onDesktopMediaRequested: function(request) {
                    // select the primary screen
                    request.selectScreen(request.screensModel.index(0, 0));
                }

                onRenderProcessTerminated: function(terminationStatus, exitCode) {
                    var status = "";
                    switch (terminationStatus) {
                    case WebEngineView.NormalTerminationStatus:
                        status = "(normal exit)";
                        break;
                    case WebEngineView.AbnormalTerminationStatus:
                        status = "(abnormal exit)";
                        break;
                    case WebEngineView.CrashedTerminationStatus:
                        status = "(crashed)";
                        break;
                    case WebEngineView.KilledTerminationStatus:
                        status = "(killed)";
                        break;
                    }

                    print("Render process exited with code " + exitCode + " " + status);
                    Qt.callLater(function() { win.currentWebView.reload() })
                }

                onSelectClientCertificate: function(selection) {
                    selection.certificates[0].select();
                }

                onFindTextFinished: function(result) {
                    if (!findBar.visible)
                        findBar.visible = true;

                    findBar.numberOfMatches = result.numberOfMatches;
                    findBar.activeMatch = result.activeMatch;
                }

                onLoadingChanged: function(loadRequest) {
                    if (loadRequest.status === WebEngineView.LoadStartedStatus)
                        findBar.reset();
                }

                onPermissionRequested: function(permission) {
                    permissionDialog.permission = permission;
                    permissionDialog.visible = true;
                }
                onWebAuthUxRequested: function(request) {
                    webAuthDialog.init(request);
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
