// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtWebEngine

StackLayout {
    id: root
    required property BrowserTabBar tabs
    required property FindBar searchBar
    required property SslDialog certificateDialog
    required property PermissionDialog permissionPrompt
    required property WebAuthDialog webAuthenticationDialog
    // appSettings from BrowserWindow, which declares the custom properties.
    required property var browserSettings
    readonly property BrowserTab currentTab: root.currentIndex < root.count ? root.children[root.currentIndex] as BrowserTab : null
    readonly property url newTabUrl: "chrome://qt?example=Quick Nano Browser"

    signal linkHovered(url hoveredUrl)
    signal fullScreenToggled(bool toggleOn)
    signal lastTabClosed()

    currentIndex: root.tabs.currentIndex

    Component.onCompleted: createTab(BrowserManager.defaultProfilePrototype.instance())

    function createTab(profile, focusOnNewTab = true, url = undefined) {
        var tab = tabFactory.createObject(root, {profile: profile}) as BrowserTab;
        var webview = tab.webView;
        root.tabs.addTabButton(tab);
        if (focusOnNewTab) {
            root.tabs.setCurrentIndex(root.tabs.count - 1);
        }
        if (url !== undefined) {
            webview.url = url;
        }
        return webview;
    }

    function tryCloseTab(index) {
        (root.children[index] as BrowserTab).tryCloseTab();
    }

    function closeTab(index) {
        if (root.tabs.count > 1) {
            root.tabs.removeTabButton(index);
            root.children[index].destroy();
        } else {
            root.lastTabClosed();
        }
    }

    Component {
        id: tabFactory
        BrowserTab {
            id: tab
            searchBar: root.searchBar
            certificateDialog: root.certificateDialog
            permissionPrompt: root.permissionPrompt
            webAuthenticationDialog: root.webAuthenticationDialog
            imageAnimationPolicy: root.browserSettings.imageAnimationPolicy
            toolBar.loadImagesChecked: root.browserSettings.autoLoadImages
            toolBar.javaScriptEnabledChecked: root.browserSettings.javaScriptEnabled
            toolBar.errorPageEnabledChecked: root.browserSettings.errorPageEnabled
            toolBar.pluginsEnabledChecked: root.browserSettings.pluginsEnabled
            toolBar.fullScreenSupportEnabledChecked: root.browserSettings.fullScreenSupportEnabled
            toolBar.autoLoadIconsForPageChecked: root.browserSettings.autoLoadIconsForPage
            toolBar.touchIconsEnabledChecked: root.browserSettings.touchIconsEnabled
            toolBar.webRTCPublicInterfacesOnlyChecked: root.browserSettings.webRTCPublicInterfacesOnly
            toolBar.devToolsEnabledChecked: root.browserSettings.devToolsEnabled
            toolBar.pdfViewerEnabledChecked: root.browserSettings.pdfViewerEnabled
            toolBar.javascriptCanAccessClipboardChecked: root.browserSettings.javascriptCanAccessClipboard
            toolBar.javascriptCanPasteChecked: root.browserSettings.javascriptCanPaste
            toolBar.disableImageAnimationChecked: root.browserSettings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.Disallow
            toolBar.allowImageAnimationChecked: root.browserSettings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.Allow
            toolBar.animateImageOnceChecked: root.browserSettings.imageAnimationPolicy === WebEngineSettings.ImageAnimationPolicy.AnimateOnce

            onSettingToggled: function(name, value) {
                root.browserSettings[name] = value;
            }

            onLinkHovered: hoveredUrl => root.linkHovered(hoveredUrl)

            onTabViewCloseRequested: function() {
                root.closeTab(tab.index);
            }

            onNewWindowRequested: function(request) {
                if (!request.userInitiated)
                    console.warn("Blocked a popup window.");
                else if (request.destination === WebEngineNewWindowRequest.InNewTab) {
                    let newTab = root.createTab(root.currentTab.webView.profile, true, request.requestedUrl);
                    newTab.acceptAsNewWindow(request);
                } else if (request.destination === WebEngineNewWindowRequest.InNewBackgroundTab) {
                    let backgroundTab = root.createTab(root.currentTab.webView.profile, false);
                    backgroundTab.acceptAsNewWindow(request);
                } else if (request.destination === WebEngineNewWindowRequest.InNewDialog) {
                    let dialog = BrowserManager.createDialog(root.currentTab.webView.profile);
                    dialog.currentWebView.acceptAsNewWindow(request);
                } else {
                    let window = BrowserManager.createWindow(root.currentTab.webView.profile);
                    window.currentWebView.acceptAsNewWindow(request);
                }
            }

            onDevToolsNewWindowRequested: function(request) {
                let newTab = root.createTab(root.currentTab.webView.profile);
                request.openIn(newTab);
            }

            onFullScreenToggled: toggleOn => root.fullScreenToggled(toggleOn)
        }
    }
}
