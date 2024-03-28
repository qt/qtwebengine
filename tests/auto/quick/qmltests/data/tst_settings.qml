// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    TestCase {
        id: testCase
        name: "WebEngineViewSettings"

        function test_javascriptEnabled() {
            webEngineView.settings.javascriptEnabled = true;

            webEngineView.url = Qt.resolvedUrl("javascript.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "New Title");
        }

        function test_javascriptDisabled() {
            webEngineView.settings.javascriptEnabled = false;

            webEngineView.url = Qt.resolvedUrl("javascript.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "Original Title");
        }

        function test_localStorageDisabled() {
            webEngineView.settings.javascriptEnabled = true;
            webEngineView.settings.localStorageEnabled = false;

            webEngineView.url = Qt.resolvedUrl("localStorage.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "Original Title");
        }

        function test_localStorageEnabled() {
            webEngineView.settings.localStorageEnabled = true;
            webEngineView.settings.javascriptEnabled = true;

            webEngineView.url = Qt.resolvedUrl("localStorage.html");
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView.reload();
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "New Title");
        }

        function test_settingsAffectCurrentViewOnly()  {
            var webEngineView2 = Qt.createQmlObject('TestWebEngineView {width: 400; height: 300;}', testCase);

            webEngineView.settings.javascriptEnabled = true;
            webEngineView2.settings.javascriptEnabled = true;

            var testUrl = Qt.resolvedUrl("javascript.html");

            webEngineView.url = testUrl;
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView2.url = testUrl;
            verify(webEngineView2.waitForLoadSucceeded());

            tryCompare(webEngineView, "title", "New Title");
            tryCompare(webEngineView2, "title", "New Title");

            webEngineView.settings.javascriptEnabled = false;

            webEngineView.url = testUrl;
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView2.url = testUrl;
            verify(webEngineView2.waitForLoadSucceeded());

            tryCompare(webEngineView, "title", "Original Title");
            tryCompare(webEngineView2, "title", "New Title");

            webEngineView2.destroy();
        }

        function test_disableReadingFromCanvas_data() {
            return [
                { tag: 'disabled', disableReadingFromCanvas: false, result: true },
                { tag: 'enabled', disableReadingFromCanvas: true, result: false },
            ]
        }

        function test_disableReadingFromCanvas(data) {
            webEngineView.settings.readingFromCanvasEnabled = !data.disableReadingFromCanvas;
            webEngineView.loadHtml("<html><body>" +
                                   "<canvas id='myCanvas' width='200' height='40' style='border:1px solid #000000;'></canvas>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());
            verify(webEngineView.settings.readingFromCanvasEnabled === !data.disableReadingFromCanvas )

            var jsCode = "(function(){" +
                       "   var canvas = document.getElementById(\"myCanvas\");" +
                       "   var ctx = canvas.getContext(\"2d\");" +
                       "   ctx.fillStyle = \"rgb(255,0,255)\";" +
                       "   ctx.fillRect(0, 0, 200, 40);" +
                       "   try {" +
                       "      src = canvas.toDataURL();" +
                       "   }" +
                       "   catch(err) {" +
                       "      src = \"\";" +
                       "   }" +
                       "   return src.length ? true : false;" +
                       "})();";

            var isDataRead = false;
            runJavaScript(jsCode, function(result) {
                isDataRead = result
            });
            tryVerify(function() { return isDataRead === data.result });
        }

        function test_forceDarkMode() {
            // based on: https://developer.chrome.com/blog/auto-dark-theme/#detecting-auto-dark-theme
            webEngineView.loadHtml("<html><body>" +
                                   "<div id=\"detection\", style=\"display: none; background-color: canvas; color-scheme: light\"</div>" +
                                   "</body></html>");
            const script = "(() => {"
                           + "  const detectionDiv = document.querySelector('#detection');"
                           + "  return getComputedStyle(detectionDiv).backgroundColor != 'rgb(255, 255, 255)';"
                           + "})()";
            verify(webEngineView.waitForLoadSucceeded());

            var isAutoDark = true;
            runJavaScript(script, result => isAutoDark = result);
            tryVerify(() => {return !isAutoDark});

            webEngineView.settings.forceDarkMode = true;
            verify(webEngineView.settings.forceDarkMode == true)

            isAutoDark = false;
            // the page is not updated immediately
            tryVerify(function() {
                runJavaScript(script, result => isAutoDark = result);
                return isAutoDark;
            });
        }
    }
}

