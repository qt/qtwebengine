// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    SignalSpy {
        id: urlChangedSpy
        target: webEngineView
        signalName: "urlChanged"
    }

    TestCase {
        id: testCase
        name: "WebEngineViewLoadHtml"
        when: windowShown

        function test_loadProgressAfterLoadHtml() {
            var loadProgressChangedCount = 0;

            var handleLoadProgressChanged = function() {
                loadProgressChangedCount++;
            }

            webEngineView.loadProgressChanged.connect(handleLoadProgressChanged);
            webEngineView.loadHtml("<html><head><title>Test page 1</title></head><body>Hello.</body></html>")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.loadProgress, 100)
            verify(loadProgressChangedCount);
            webEngineView.loadProgressChanged.disconnect(handleLoadProgressChanged);
        }

        function test_dataURLFragment() {
            webEngineView.loadHtml("<html><body>" +
                                   "<a id='link' href='#anchor'>anchor</a>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            urlChangedSpy.clear();
            var center = getElementCenter("link");
            mouseClick(webEngineView, center.x, center.y);
            urlChangedSpy.wait();
            compare(webEngineView.url.toString().split("#")[1], "anchor");
        }
    }
}
