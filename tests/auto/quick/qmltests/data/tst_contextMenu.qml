// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import "../mock-delegates/TestParams"

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 400

    property string linkText: ""
    property var mediaType: null
    property string selectedText: ""

    onContextMenuRequested: function (request) {
        linkText = request.linkText;
        mediaType = request.mediaType;
        selectedText = request.selectedText;
    }

    SignalSpy {
        id: contextMenuRequestedSpy
        target: webEngineView
        signalName: "contextMenuRequested"
    }

    TestCase {
        id: testCase
        name: "WebEngineViewContextMenu"
        when: windowShown

        function init() {
            MenuParams.isMenuOpened = false;
        }

        function cleanup() {
            contextMenuRequestedSpy.clear();
        }

        function test_contextMenuRequest_data() {
            return [
                   { tag: "defaultContextMenu", userHandled: false, accepted: false },
                   { tag: "defaultContextMenuWithConnect", userHandled: true, accepted: false },
                   { tag: "dontShowDefaultContextMenu", userHandled: true, accepted: true },
            ];
        }

        function test_contextMenuRequest(row) {
            function contextMenuHandler(request) {
                request.accepted = row.accepted;
            }

            if (row.userHandled) {
                webEngineView.contextMenuRequested.connect(contextMenuHandler);
            }
            webEngineView.loadHtml("<html></html>");
            verify(webEngineView.waitForLoadSucceeded());

            mouseClick(webEngineView, 20, 20, Qt.RightButton);
            contextMenuRequestedSpy.wait();
            compare(contextMenuRequestedSpy.count, 1);
            tryCompare(MenuParams, "isMenuOpened", !row.accepted);

            webEngineView.contextMenuRequested.disconnect(contextMenuHandler);
        }

        function test_contextMenuLinkAndSelectedText() {
            webEngineView.loadHtml("<html><body>" +
                                   "<span id='text'>Text </span>" +
                                   "<a id='link' href='test1.html'>Link</a>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            // 1. Nothing is selected, right click on the link
            var linkCenter = getElementCenter("link");
            mouseClick(webEngineView, linkCenter.x, linkCenter.y, Qt.RightButton);
            contextMenuRequestedSpy.wait();
            compare(contextMenuRequestedSpy.count, 1);

            compare(linkText, "Link");
            compare(mediaType, ContextMenuRequest.MediaTypeNone);
            compare(selectedText, "");

            verify(webEngineView.action(WebEngineView.OpenLinkInNewTab).enabled);
            verify(webEngineView.action(WebEngineView.OpenLinkInNewWindow).enabled);
            verify(webEngineView.action(WebEngineView.DownloadLinkToDisk).enabled);
            verify(webEngineView.action(WebEngineView.CopyLinkToClipboard).enabled);

            contextMenuRequestedSpy.clear();

            // 2. Everything is selected, right click on the link
            webEngineView.triggerWebAction(WebEngineView.SelectAll);
            tryVerify(function() { return getTextSelection() == "Text Link" });

            mouseClick(webEngineView, linkCenter.x, linkCenter.y, Qt.RightButton);
            contextMenuRequestedSpy.wait();
            compare(contextMenuRequestedSpy.count, 1);

            compare(linkText, "Link");
            compare(mediaType, ContextMenuRequest.MediaTypeNone);
            compare(selectedText, "Text Link");

            contextMenuRequestedSpy.clear();

            // 3. Everything is selected, right click on the text
            var textCenter = getElementCenter("text");
            mouseClick(webEngineView, textCenter.x, textCenter.y, Qt.RightButton);
            contextMenuRequestedSpy.wait();
            compare(contextMenuRequestedSpy.count, 1);

            compare(linkText, "");
            compare(mediaType, ContextMenuRequest.MediaTypeNone);
            compare(selectedText, "Text Link");
        }

        function test_contextMenuMediaType() {
            webEngineView.url = Qt.resolvedUrl("favicon.html");
            verify(webEngineView.waitForLoadSucceeded());
            // 1. Right click on the image
            var center = getElementCenter("image");
            mouseClick(webEngineView, center.x, center.y, Qt.RightButton);
            contextMenuRequestedSpy.wait();
            compare(contextMenuRequestedSpy.count, 1);

            compare(linkText, "");
            compare(mediaType, ContextMenuRequest.MediaTypeImage);
            compare(selectedText, "");
            contextMenuRequestedSpy.clear();

            // 2. Right click out of the image
            mouseClick(webEngineView, center.x + 30, center.y, Qt.RightButton);
            contextMenuRequestedSpy.wait();
            compare(contextMenuRequestedSpy.count, 1);

            compare(linkText, "");
            compare(mediaType, ContextMenuRequest.MediaTypeNone);
            compare(selectedText, "");
        }
    }
}
