// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    property var newViewRequest: null
    property var dialog: null
    property string viewType: ""
    property var loadRequestArray: []

    onLoadingChanged: function(load) {
        loadRequestArray.push({
            "status": load.status,
        });
    }

    SignalSpy {
        id: newViewRequestedSpy
        target: webEngineView
        signalName: "newWindowRequested"
    }

    onNewWindowRequested: function(request) {
        newViewRequest = {
            "destination": request.destination,
            "userInitiated": request.userInitiated,
            "requestedUrl": request.requestedUrl
        };

        dialog = Qt.createQmlObject(
            "import QtQuick.Window\n" +
            "Window {\n" +
            "    width: 100; height: 100\n" +
            "    visible: true; flags: Qt.Dialog\n" +
            "    property alias webEngineView: webView\n" +
            "    TestWebEngineView { id: webView; anchors.fill: parent }\n" +
            "}", webEngineView);

        if (viewType === "dialog")
            request.openIn(dialog.webEngineView);
        else if (viewType === "null")
            request.openIn(0);
        else if (viewType === "webEngineView")
            request.openIn(webEngineView);
    }

    TestCase {
        id: testCase
        name: "NewWindowRequest"
        when: windowShown

        function init() {
            webEngineView.url = Qt.resolvedUrl("about:blank");
            verify(webEngineView.waitForLoadSucceeded());

            newViewRequestedSpy.clear();
            newViewRequest = null;
            viewType = "";
            loadRequestArray = [];
        }

        function cleanup() {
            if (dialog)
                dialog.destroy();
        }

        function test_loadNewWindowRequest_data() {
            return [
                   { tag: "dialog", viewType: "dialog" },
                   { tag: "invalid", viewType: "null" },
                   { tag: "unhandled", viewType: "" },
                   { tag: "webEngineView", viewType: "webEngineView" },
            ];
        }

        function test_loadNewWindowRequest(row) {
            viewType = row.viewType;
            var url = 'data:text/html,%3Chtml%3E%3Cbody%3ETest+Page%3C%2Fbody%3E%3C%2Fhtml%3E';

            // Open an empty page in a new tab
            webEngineView.loadHtml(
                "<html><head><script>" +
                "   function popup() { window.open(''); }" +
                "</script></head>" +
                "<body onload='popup()'></body></html>");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(newViewRequestedSpy, "count", 1);

            compare(newViewRequest.destination, WebEngineNewWindowRequest.InNewTab);
            verify(!newViewRequest.userInitiated);

            if (viewType === "dialog") {
                tryVerify(dialog.webEngineView.loadSucceeded)
                compare(dialog.webEngineView.url, Qt.url("about:blank"));
                dialog.destroy();
            }
            // https://chromium-review.googlesource.com/c/chromium/src/+/1300395
            compare(newViewRequest.requestedUrl, 'about:blank#blocked');
            newViewRequestedSpy.clear();

            // Open a page in a new dialog
            webEngineView.loadHtml(
                "<html><head><script>" +
                "   function popup() { window.open('" + url + "', '_blank', 'width=200,height=100'); }" +
                "</script></head>" +
                "<body onload='popup()'></body></html>");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(newViewRequestedSpy, "count", 1);

            compare(newViewRequest.destination, WebEngineNewWindowRequest.InNewDialog);
            compare(newViewRequest.requestedUrl, url);
            verify(!newViewRequest.userInitiated);
            if (viewType === "dialog") {
                tryVerify(dialog.webEngineView.loadSucceeded)
                dialog.destroy();
            }
            newViewRequestedSpy.clear();

            if (viewType !== "webEngineView") {
                // Open a page in a new dialog by user
                webEngineView.loadHtml(
                    "<html><head><script>" +
                    "   function popup() { window.open('" + url + "', '_blank', 'width=200,height=100'); }" +
                    "</script></head>" +
                    "<body onload=\"document.getElementById('popupButton').focus();\">" +
                    "   <button id='popupButton' onclick='popup()'>Pop Up!</button>" +
                    "</body></html>");
                verify(webEngineView.waitForLoadSucceeded());
                webEngineView.verifyElementHasFocus("popupButton");
                keyPress(Qt.Key_Enter);
                tryCompare(newViewRequestedSpy, "count", 1);
                compare(newViewRequest.requestedUrl, url);

                compare(newViewRequest.destination, WebEngineNewWindowRequest.InNewDialog);
                verify(newViewRequest.userInitiated);
                if (viewType === "dialog") {
                    tryVerify(dialog.webEngineView.loadSucceeded)
                    dialog.destroy();
                }
                newViewRequestedSpy.clear();
            }

            loadRequestArray = [];
            compare(loadRequestArray.length, 0);
            webEngineView.url = Qt.resolvedUrl("test2.html");
            verify(webEngineView.waitForLoadSucceeded());
            var center = webEngineView.getElementCenter("link");
            mouseClick(webEngineView, center.x, center.y, Qt.LeftButton, Qt.ControlModifier);
            tryCompare(newViewRequestedSpy, "count", 1);
            compare(newViewRequest.requestedUrl, Qt.resolvedUrl("test1.html"));
            compare(newViewRequest.destination, WebEngineNewWindowRequest.InNewBackgroundTab);
            verify(newViewRequest.userInitiated);
            if (viewType === "" || viewType === "null") {
                compare(loadRequestArray[0].status, WebEngineView.LoadStartedStatus);
                compare(loadRequestArray[1].status, WebEngineView.LoadSucceededStatus);
                compare(loadRequestArray.length, 2);
            }
            newViewRequestedSpy.clear();
        }
    }
}
