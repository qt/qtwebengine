// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtWebEngine

ColumnLayout {
    id: root
    required property TabBar tabs
    required property FindBar searchBar
    required property SslDialog certificateDialog
    required property PermissionDialog permissionPrompt
    required property WebAuthDialog webAuthenticationDialog
    property alias profile: tabWebView.profile
    readonly property alias toolBar: tabToolBar
    readonly property alias webView: tabWebView
    required property int imageAnimationPolicy
    property int index: 0

    signal fullScreenToggled(bool toggleOn)
    signal linkHovered(url hoveredUrl)
    signal tabViewCloseRequested()
    signal newWindowRequested(WebEngineNewWindowRequest request)
    signal devToolsNewWindowRequested(WebEngineNewWindowRequest request)
    signal settingToggled(string name, var value)

    spacing: 0

    function tryCloseTab() {
        tabWebView.triggerWebAction(WebEngineView.RequestClose);
    }

    WebToolBar {
        id: tabToolBar
        Layout.fillWidth: true
        currentWebView: tabWebView
        onSettingToggled: (name, value) => root.settingToggled(name, value)
    }

    WebEngineView {
        id: tabWebView
        Layout.fillWidth: true
        Layout.fillHeight: true
        focus: true

        states: [
            State {
                name: "FullScreen"
                PropertyChanges {
                    root.tabs.visible: false
                    root.tabs.height: 0
                    tabToolBar.visible: false
                }
            }
        ]
        settings.localContentCanAccessRemoteUrls: true
        settings.localContentCanAccessFileUrls: false
        settings.autoLoadImages: tabToolBar.loadImagesChecked
        settings.javascriptEnabled: tabToolBar.javaScriptEnabledChecked
        settings.errorPageEnabled: tabToolBar.errorPageEnabledChecked
        settings.pluginsEnabled: tabToolBar.pluginsEnabledChecked
        settings.fullScreenSupportEnabled: tabToolBar.fullScreenSupportEnabledChecked
        settings.autoLoadIconsForPage: tabToolBar.autoLoadIconsForPageChecked
        settings.touchIconsEnabled: tabToolBar.touchIconsEnabledChecked
        settings.webRTCPublicInterfacesOnly: tabToolBar.webRTCPublicInterfacesOnlyChecked
        settings.pdfViewerEnabled: tabToolBar.pdfViewerEnabledChecked
        settings.imageAnimationPolicy: root.imageAnimationPolicy
        settings.screenCaptureEnabled: true
        settings.javascriptCanAccessClipboard: tabToolBar.javascriptCanAccessClipboardChecked
        settings.javascriptCanPaste: tabToolBar.javascriptCanPasteChecked

        onCertificateError: function(error) {
            if (!error.isMainFrame) {
                error.rejectCertificate();
                return;
            }

            error.defer();
            root.certificateDialog.enqueue(error);
        }

        onFullScreenRequested: function(request) {
            tabWebView.state = request.toggleOn ? "FullScreen" : "";
            root.fullScreenToggled(request.toggleOn);
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
            Qt.callLater(function() { tabWebView.reload() })
        }

        onSelectClientCertificate: function(selection) {
            selection.certificates[0].select();
        }

        onFindTextFinished: function(result) {
            if (!root.searchBar.visible)
                root.searchBar.visible = true;

            root.searchBar.numberOfMatches = result.numberOfMatches;
            root.searchBar.activeMatch = result.activeMatch;
        }

        onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebEngineView.LoadStartedStatus)
                root.searchBar.reset();
        }

        onPermissionRequested: function(permission) {
            root.permissionPrompt.permission = permission;
            root.permissionPrompt.visible = true;
        }
        onWebAuthUxRequested: function(request) {
            root.webAuthenticationDialog.init(request);
        }

        onLinkHovered: hoveredUrl => root.linkHovered(hoveredUrl)
        onWindowCloseRequested: root.tabViewCloseRequested()
        onNewWindowRequested: request => root.newWindowRequested(request)
    }

    WebEngineView {
        id: devToolsWebEngineView
        visible: tabToolBar.devToolsEnabledChecked
        Layout.fillWidth: true
        Layout.preferredHeight: 400
        inspectedView: visible ? tabWebView : null
        onNewWindowRequested: request => root.devToolsNewWindowRequested(request)

        onWindowCloseRequested: function() {
            // Delay hiding for keep the inspectedView set to receive the ACK message of close.
            Qt.callLater(function() { root.settingToggled("devToolsEnabled", false) })
        }
    }
}
