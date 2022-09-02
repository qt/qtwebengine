// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    SignalSpy {
        id: spy
        target: webEngineView
        signalName: "titleChanged"
    }

    TestCase {
        name: "WebEngineViewRunJavaScript"
        function test_runJavaScript() {
            var testTitle = "Title to test runJavaScript";
            runJavaScript("document.title = \"" + testTitle +"\"");
            _waitFor(function() { spy.count > 0; });
            compare(spy.count, 1);
            compare(webEngineView.title, testTitle);

            var testTitle2 = "Foobar"
            var testHtml = "<html><head><title>" + testTitle2 + "</title></head><body></body></html>";
            loadHtml(testHtml);
            waitForLoadSucceeded();
            var callbackCalled = false;
            runJavaScript("document.title", function(result) {
                    compare(result, testTitle2);
                    callbackCalled = true;
                });
            tryVerify(function() { return callbackCalled; });
        }
    }
}
