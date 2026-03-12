// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQuick
import QtWebEngine

QtObject {
    id: manager

    property WebEngineProfilePrototype defaultProfilePrototype : WebEngineProfilePrototype {
        storageName: "Profile"
    }

    property WebEngineProfilePrototype otrPrototype : WebEngineProfilePrototype {}

    function createWindow(profile) {
        let browserWindowComponent = Qt.createComponent("BrowserUtils", "BrowserWindow");
        let newWindow = browserWindowComponent.createObject(manager) as BrowserWindow;
        newWindow.currentWebView.profile = profile;
        profile.downloadRequested.connect(newWindow.onDownloadRequested);
        return newWindow;
    }
    function createDialog(profile) {
        let browserDialogComponent = Qt.createComponent("BrowserUtils", "BrowserDialog");
        let newDialog = browserDialogComponent.createObject(manager) as BrowserDialog;
        newDialog.closing.connect(function(){destroy()})
        newDialog.currentWebView.profile = profile;
        return newDialog;
    }
    function load(url) {
        let browserWindow = createWindow(manager.defaultProfilePrototype.instance());
        browserWindow.currentWebView.url = url;
    }

    Component.onCompleted: {
        let fullVersionList = manager.defaultProfilePrototype.instance().clientHints.fullVersionList;
        fullVersionList["QuickNanoBrowser"] = "1.0";
        manager.defaultProfilePrototype.instance().clientHints.fullVersionList = fullVersionList;
    }
}
