// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
import QtWebEngine

WebEngineView {
    id: root
    required property WebToolBar toolBar
    required property TabBar tabs
    required property FindBar searchBar
    required property SslDialog certificateDialog
    required property PermissionDialog permissionPrompt
    required property WebAuthDialog webAuthenticationDialog
    required property int imageAnimationPolicy
    property int index: 0

    signal fullScreenToggled(bool toggleOn)

    focus: true

    settings.localContentCanAccessRemoteUrls: true
    settings.localContentCanAccessFileUrls: false
    settings.autoLoadImages: root.toolBar.loadImagesChecked
    settings.javascriptEnabled: root.toolBar.javaScriptEnabledChecked
    settings.errorPageEnabled: root.toolBar.errorPageEnabledChecked
    settings.pluginsEnabled: root.toolBar.pluginsEnabledChecked
    settings.fullScreenSupportEnabled: root.toolBar.fullScreenSupportEnabledChecked
    settings.autoLoadIconsForPage: root.toolBar.autoLoadIconsForPageChecked
    settings.touchIconsEnabled: root.toolBar.touchIconsEnabledChecked
    settings.webRTCPublicInterfacesOnly: root.toolBar.webRTCPublicInterfacesOnlyChecked
    settings.pdfViewerEnabled: root.toolBar.pdfViewerEnabledChecked
    settings.imageAnimationPolicy: root.imageAnimationPolicy
    settings.screenCaptureEnabled: true
    settings.javascriptCanAccessClipboard: root.toolBar.javascriptCanAccessClipboardChecked
    settings.javascriptCanPaste: root.toolBar.javascriptCanPasteChecked

    onCertificateError: function(error) {
        if (!error.isMainFrame) {
            error.rejectCertificate();
            return;
        }

        error.defer();
        root.certificateDialog.enqueue(error);
    }

    onFullScreenRequested: function(request) {
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
        Qt.callLater(function() { root.reload() })
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
}
