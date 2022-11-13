// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick 2.1
import QtWebEngine 1.1

QtObject {
    id: root

    property bool thirdPartyCookiesEnabled: true

    property QtObject testProfile: WebEngineProfile {
        storageName: "Test"
    }

    property QtObject otrProfile: WebEngineProfile {
        offTheRecord: true
    }

    property Component browserWindowComponent: BrowserWindow {
        applicationRoot: root
        onClosing: destroy()
    }
    property Component browserDialogComponent: BrowserDialog {
        onClosing: destroy()
    }
    function createWindow(profile) {
        var newWindow = browserWindowComponent.createObject(root)
        newWindow.currentWebView.profile = profile
        profile.downloadRequested.connect(newWindow.onDownloadRequested)
        profile.presentNotification.connect(newWindow.onPresentNotification)
        return newWindow
    }
    function createDialog(profile) {
        var newDialog = browserDialogComponent.createObject(root)
        newDialog.currentWebView.profile = profile
        return newDialog
    }
    function load(url) {
        var browserWindow = createWindow(testProfile)
        browserWindow.currentWebView.url = url
    }
}
