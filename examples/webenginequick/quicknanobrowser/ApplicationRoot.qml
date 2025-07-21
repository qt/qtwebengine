// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtWebEngine

QtObject {
    id: root

    property WebEngineProfilePrototype defaultProfilePrototype : WebEngineProfilePrototype {
        storageName: "Profile"
        Component.onCompleted: {
            let fullVersionList = root.defaultProfilePrototype.instance().clientHints.fullVersionList;
            fullVersionList["QuickNanoBrowser"] = "1.0";
            root.defaultProfilePrototype.instance().clientHints.fullVersionList = fullVersionList;
        }
    }

    property WebEngineProfilePrototype otrPrototype : WebEngineProfilePrototype {
    }

    property Component browserWindowComponent: BrowserWindow {
        applicationRoot: root
    }
    property Component browserDialogComponent: BrowserDialog {
        onClosing: destroy()
    }
    function createWindow(profile) {
        var newWindow = browserWindowComponent.createObject(root) as BrowserWindow;
        newWindow.currentWebView.profile = profile;
        profile.downloadRequested.connect(newWindow.onDownloadRequested);
        return newWindow;
    }
    function createDialog(profile) {
        var newDialog = browserDialogComponent.createObject(root) as BrowserDialog;
        newDialog.currentWebView.profile = profile;
        return newDialog;
    }
    function load(url) {
        var browserWindow = createWindow(root.defaultProfilePrototype.instance());
        browserWindow.currentWebView.url = url;
    }
}
