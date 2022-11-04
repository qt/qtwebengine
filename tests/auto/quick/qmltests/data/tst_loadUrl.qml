// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    property var loadRequestArray: []

    onLoadingChanged: function(load) {
        loadRequestArray.push({
            "status": load.status,
            "url": load.url,
            "activeUrl": webEngineView.url
        });
    }

    function clear() {
        // Reset loadStatus for waitForLoadSucceded
        webEngineView.loadStatus = null;
        loadRequestArray = [];
    }

    TestCase {
        id: testCase
        name: "WebEngineViewLoadUrl"
        when: windowShown

        function init() {
            webEngineView.clear();
        }

        function test_loadIgnoreEmptyUrl() {
            var url = Qt.resolvedUrl("test1.html");
            webEngineView.url = url;
            verify(webEngineView.waitForLoadSucceeded());
            compare(loadRequestArray[0].status, WebEngineView.LoadStartedStatus);
            compare(loadRequestArray[1].status, WebEngineView.LoadSucceededStatus);
            compare(loadRequestArray.length, 2);
            compare(webEngineView.url, url);
            webEngineView.clear();

            var lastUrl = webEngineView.url;
            webEngineView.url = "";
            wait(1000);
            compare(loadRequestArray.length, 0);
            compare(webEngineView.url, lastUrl);
            webEngineView.clear();

            var aboutBlank = "about:blank";
            webEngineView.url = aboutBlank;
            verify(webEngineView.waitForLoadSucceeded());
            compare(loadRequestArray[0].status, WebEngineLoadingInfo.LoadStartedStatus);
            compare(loadRequestArray[1].status, WebEngineLoadingInfo.LoadSucceededStatus);
            compare(loadRequestArray.length, 2);
            compare(webEngineView.url, aboutBlank);
            webEngineView.clear();

            // It shouldn't interrupt any ongoing load when an empty url is used.
            var watchProgress = true;
            var handleLoadProgress = function() {
                if (webEngineView.loadProgress != 100) {
                    webEngineView.url = "";
                    watchProgress = false;
                }
            }
            webEngineView.loadProgressChanged.connect(handleLoadProgress);
            webEngineView.url = url;
            verify(webEngineView.waitForLoadSucceeded());
            compare(loadRequestArray[0].status, WebEngineView.LoadStartedStatus);
            compare(loadRequestArray[1].status, WebEngineView.LoadSucceededStatus);
            compare(loadRequestArray.length, 2);
            verify(!watchProgress);
            compare(webEngineView.url, url);
            webEngineView.loadProgressChanged.disconnect(handleLoadProgress);
            webEngineView.clear();
        }

        function test_urlProperty() {
            WebEngine.settings.errorPageEnabled = false;

            var loadRequest = null;

            // Test succeeded load
            var url = Qt.resolvedUrl("test1.html");
            webEngineView.url = url;
            tryCompare(loadRequestArray, "length", 2);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineView.LoadStartedStatus);
            compare(loadRequest.activeUrl, url);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineView.LoadSucceededStatus);
            compare(loadRequest.activeUrl, url);
            webEngineView.clear();

            // Test failed load
            var bogusSite = "http://www.somesitethatdoesnotexist.abc/";
            webEngineView.url = bogusSite;
            tryCompare(loadRequestArray, "length", 2, 12000);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineView.LoadStartedStatus);
            compare(loadRequest.activeUrl, bogusSite);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineView.LoadFailedStatus);
            compare(loadRequest.status, WebEngineLoadingInfo.LoadFailedStatus);
            compare(loadRequest.activeUrl, url);
            webEngineView.clear();

            // Test page redirection
            var redirectUrl = Qt.resolvedUrl("redirect.html");
            webEngineView.url = redirectUrl;
            tryCompare(loadRequestArray, "length", 4);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineView.LoadStartedStatus);
            compare(loadRequest.activeUrl, redirectUrl);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineView.LoadSucceededStatus);
            compare(loadRequest.activeUrl, redirectUrl);
            loadRequest = loadRequestArray[2];
            compare(loadRequest.status, WebEngineLoadingInfo.LoadStartedStatus);
            compare(loadRequest.activeUrl, redirectUrl);
            loadRequest = loadRequestArray[3];
            compare(loadRequest.status, WebEngineLoadingInfo.LoadSucceededStatus);
            compare(loadRequest.activeUrl, url);
            webEngineView.clear();

            // Test clicking on a hyperlink
            var linkUrl = Qt.resolvedUrl("link.html");
            webEngineView.url = linkUrl;
            tryCompare(loadRequestArray, "length", 2);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineView.LoadStartedStatus);
            compare(loadRequest.activeUrl, linkUrl);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineView.LoadSucceededStatus);
            compare(loadRequest.activeUrl, linkUrl);
            webEngineView.clear();

            var lastUrl = webEngineView.url;
            mouseClick(webEngineView, 10, 10, Qt.LeftButton, Qt.NoModifiers, 50);
            tryCompare(loadRequestArray, "length", 2);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineLoadingInfo.LoadStartedStatus);
            compare(loadRequest.url, url);
            compare(loadRequest.activeUrl, lastUrl);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineLoadingInfo.LoadSucceededStatus);
            compare(loadRequest.url, url);
            compare(loadRequest.activeUrl, url);
            webEngineView.clear();
        }

        function test_loadDataUrl() {
            WebEngine.settings.errorPageEnabled = false;

            var loadRequest = null;

            // Test load of a data URL
            var dataUrl = "data:text/html,foo";
            webEngineView.url = dataUrl;
            tryCompare(loadRequestArray, "length", 2);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineView.LoadStartedStatus);
            compare(loadRequest.activeUrl, dataUrl);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineView.LoadSucceededStatus);
            compare(loadRequest.activeUrl, dataUrl);
            webEngineView.clear();

            // Test loadHtml after a failed load
            var aboutBlank = "about:blank";
            webEngineView.url = aboutBlank; // Reset from previous test
            tryCompare(loadRequestArray, "length", 2);
            webEngineView.clear();

            var bogusSite = "http://www.somesitethatdoesnotexist.abc/";
            var handleLoadFailed = function(loadRequest) {
                if (loadRequest.status === WebEngineView.LoadFailedStatus) {
                    // loadHtml constructs data URL
                    webEngineView.loadHtml("load failed", bogusSite);
                    compare(loadRequest.url, bogusSite);
                }
            }
            webEngineView.loadingChanged.connect(handleLoadFailed);
            webEngineView.url = bogusSite
            tryCompare(loadRequestArray, "length", 4, 30000);
            webEngineView.loadingChanged.disconnect(handleLoadFailed);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineView.LoadStartedStatus);
            compare(loadRequest.activeUrl, bogusSite);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineView.LoadFailedStatus);
            compare(loadRequest.status, WebEngineLoadingInfo.LoadFailedStatus);
            // Since the load did not succeed the active url is the
            // URL of the previous successful load.
            compare(loadRequest.activeUrl, aboutBlank);
            loadRequest = loadRequestArray[2];
            compare(loadRequest.status, WebEngineLoadingInfo.LoadStartedStatus);
            compare(loadRequest.activeUrl, bogusSite);
            compare(loadRequest.url, "data:text/html;charset=UTF-8,load failed")
            loadRequest = loadRequestArray[3];
            compare(loadRequest.status, WebEngineLoadingInfo.LoadSucceededStatus);
            compare(loadRequest.activeUrl, bogusSite);
            compare(loadRequest.url, bogusSite)
            webEngineView.clear();
        }

        function test_QTBUG_56661() {
            var url = Qt.resolvedUrl("test1.html");

            // Warm up phase
            webEngineView.url = url;
            verify(webEngineView.waitForLoadSucceeded());

            // Load data URL
            var dataUrl = "data:text/html,foo";
            webEngineView.url = dataUrl;
            verify(webEngineView.waitForLoadSucceeded());

            // WebEngine should not try to execute user scripts in the
            // render frame of the warm up phase otherwise the renderer
            // crashes.
            webEngineView.url = url;
            verify(webEngineView.waitForLoadSucceeded());
        }

        function test_stopStatus() {
            var loadRequest = null;
            var initialUrl = Qt.resolvedUrl("test1.html");
            var stoppedUrl = Qt.resolvedUrl("test2.html");

            // Warm up phase
            webEngineView.url = initialUrl;
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView.loadStatus = null;
            loadRequestArray = [];

            // Stop load
            var handleLoadStarted = function(loadRequest) {
                if (loadRequest.status === WebEngineView.LoadStartedStatus)
                    webEngineView.stop();
            }
            webEngineView.loadingChanged.connect(handleLoadStarted);
            webEngineView.url = stoppedUrl;
            tryCompare(loadRequestArray, "length", 2);
            webEngineView.loadingChanged.disconnect(handleLoadStarted);

            loadRequest = loadRequestArray[0];
            compare(loadRequest.status, WebEngineView.LoadStartedStatus);
            compare(loadRequest.url, stoppedUrl);
            compare(loadRequest.activeUrl, stoppedUrl);
            loadRequest = loadRequestArray[1];
            compare(loadRequest.status, WebEngineView.LoadStoppedStatus);
            compare(loadRequest.status, WebEngineLoadingInfo.LoadStoppedStatus);
            compare(loadRequest.url, stoppedUrl);
            compare(loadRequest.activeUrl, initialUrl);
            webEngineView.clear();
        }

        function test_loadStartedAfterInPageNavigation() {
            webEngineView.url = Qt.resolvedUrl("test4.html");
            verify(webEngineView.waitForLoadSucceeded());
            compare(webEngineView.loadProgress, 100);
            compare(loadRequestArray.length, 2);
            compare(loadRequestArray[0].status, WebEngineView.LoadStartedStatus);
            compare(loadRequestArray[1].status, WebEngineView.LoadSucceededStatus);

            // In-page navigation shouldn't trigger load
            let anchorUrl = Qt.resolvedUrl("test4.html#anchor");
            let c = webEngineView.getElementCenter('anchor')
            mouseClick(webEngineView, c.x, c.y)
            tryCompare(webEngineView, 'url', anchorUrl)

            // Load after in-page navigation.
            webEngineView.url = Qt.resolvedUrl("test4.html");
            verify(webEngineView.waitForLoadSucceeded());
            compare(webEngineView.loadProgress, 100);
            compare(loadRequestArray.length, 4);
            compare(loadRequestArray[2].status, WebEngineView.LoadStartedStatus);
            compare(loadRequestArray[3].status, WebEngineView.LoadSucceededStatus);

            webEngineView.clear();
        }
    }
}
