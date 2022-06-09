// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtWebEngine

//! [0]
QtObject {
    id: windowParent
    // Create the initial browsing windows and open the startup page.
    Component.onCompleted: {
        var firstWindow = windowComponent.createObject(windowParent);
        firstWindow.webView.loadHtml('<input type="button" value="Click!" onclick="window.open(&quot;http://qt.io&quot;)">');
    }

    property Component windowComponent: Window {
        // Destroy on close to release the Window's QML resources.
        // Because it was created with a parent, it won't be garbage-collected.
        onClosing: destroy()
        visible: true

        property WebEngineView webView: webView_
        WebEngineView {
            id: webView_
            anchors.fill: parent

            // Handle the signal. Dynamically create the window and
            // use its WebEngineView as the destination of our request.
            onNewWindowRequested: function(request) {
                var newWindow = windowComponent.createObject(windowParent);
                newWindow.webView.acceptAsNewWindow(request);
            }
        }
    }
}
//! [0]
