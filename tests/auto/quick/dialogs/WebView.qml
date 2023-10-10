// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtWebEngine
import QtQuick.Window
import QtTest
import io.qt.tester

Window {
    width: 50
    height: 50
    visible: true

    TestHandler {
        id: handler
        onJavaScript: function(script) {
            view.runJavaScript(script , function(result) {
                handler.ready = true
            })
        }
        onLoadPage: function(url) {
            if (view.url === url) {
                handler.ready = true
                return
            }
            view.url = url
        }
    }

    WebEngineView {
        id: view
        anchors.fill: parent
        onLoadingChanged: function(request) {
            if (request.status === WebEngineView.LoadSucceededStatus) {
                handler.ready = true
            } else if (request.status === WebEngineView.LoadFailedStatus) {
                console.log("Page was not successfully loaded from qrc! Status: " + request.status
                    + ", error [code: " + request.errorCode + "]: '" + request.errorString + "'")
            }
        }

        onContextMenuRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onAuthenticationDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onJavaScriptDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onColorDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onFileDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }
    }
}
