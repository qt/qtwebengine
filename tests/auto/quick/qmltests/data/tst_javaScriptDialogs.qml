// Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import "../../qmltests/data"
import "../mock-delegates/TestParams"

TestWebEngineView {
    id: webEngineView
    anchors.fill: parent

    property bool windowCloseRejectedCalled: false

    // Called by QQuickWebEngineViewPrivate::windowCloseRejected()
    function windowCloseRejected() {
        windowCloseRejectedCalled = true;
    }

    TestCase {
        id: test
        name: "WebEngineViewJavaScriptDialogs"
        when: windowShown

        function init() {
            JSDialogParams.dialogMessage = "";
            JSDialogParams.dialogTitle = "";
            JSDialogParams.dialogCount = 0;
            JSDialogParams.shouldAcceptDialog = true;
        }

        function test_alert() {
            webEngineView.url = Qt.resolvedUrl("alert.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 1)
            compare(JSDialogParams.dialogMessage, "Hello Qt")
            verify(JSDialogParams.dialogTitle.indexOf("Javascript Alert -") === 0)
        }

        function test_confirm() {
            webEngineView.url = Qt.resolvedUrl("confirm.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogMessage, "Confirm test")
            compare(JSDialogParams.dialogCount, 1)
            compare(webEngineView.title, "ACCEPTED")
            JSDialogParams.shouldAcceptDialog = false
            webEngineView.reload()
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 2)
            compare(webEngineView.title, "REJECTED")

        }
        function readMousePressRecieved() {
            var mousePressReceived;
            runJavaScript("window.mousePressReceived", function(result) {
                mousePressReceived = result;
            });

            _waitFor(function() { return mousePressReceived != undefined; });
            return mousePressReceived;
        }

        function simulateUserGesture() {
            // A user gesture after page load is required since Chromium 60 to allow showing
            // an onbeforeunload dialog.
            // See https://www.chromestatus.com/feature/5082396709879808
            mouseClick(webEngineView, 10, 10, Qt.LeftButton)

            tryVerify(readMousePressRecieved)
        }

        function test_confirmClose() {
            webEngineView.url = Qt.resolvedUrl("confirmclose.html");
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView.windowCloseRequestedSignalEmitted = false;
            JSDialogParams.shouldAcceptDialog = true;

            simulateUserGesture()
            webEngineView.triggerWebAction(WebEngineView.RequestClose);
            verify(webEngineView.waitForWindowCloseRequested());

            // Navigate away from page with onbeforeunload handler,
            // otherwise it would trigger an extra dialog request when
            // navigating in the subsequent test.
            webEngineView.url = Qt.resolvedUrl("about:blank");
            verify(webEngineView.waitForLoadSucceeded());
            compare(JSDialogParams.dialogCount, 2)
        }

        function test_rejectClose() {
            webEngineView.url = Qt.resolvedUrl("confirmclose.html");
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView.windowCloseRejectedCalled = false;
            JSDialogParams.shouldAcceptDialog = false;

            simulateUserGesture()
            webEngineView.triggerWebAction(WebEngineView.RequestClose);
            tryVerify(function() { return webEngineView.windowCloseRejectedCalled; });

            // Navigate away from page with onbeforeunload handler,
            // otherwise it would trigger an extra dialog request when
            // navigating in the subsequent test.
            JSDialogParams.shouldAcceptDialog = true;
            webEngineView.url = Qt.resolvedUrl("about:blank");
            verify(webEngineView.waitForLoadSucceeded());
            compare(JSDialogParams.dialogCount, 2)
        }

        function test_prompt() {
            JSDialogParams.inputForPrompt = "tQ olleH"
            webEngineView.url = Qt.resolvedUrl("prompt.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 1)
            compare(webEngineView.title, "tQ olleH")
            JSDialogParams.shouldAcceptDialog = false
            webEngineView.reload()
            verify(webEngineView.waitForLoadSucceeded())
            compare(JSDialogParams.dialogCount, 2)
            compare(webEngineView.title, "null")
        }
    }
}
