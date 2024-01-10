// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 400

    TestCase {
        id: testCase
        name: "WebEngineInputTextDirection"
        when: windowShown

        function getInputTextDirection(element) {
            var dir;
            runJavaScript("document.getElementById('" + element + "').dir", function(result) {
                dir = result;
            });
            tryVerify(function() { return dir != undefined; });
            return dir;
        }

        function test_changeInputTextDirection() {
            webEngineView.loadHtml("<html><body><input type='text' id='textfield' value='some text'></body></html>");
            verify(webEngineView.waitForLoadSucceeded());
            setFocusToElement("textfield");

            var rtlAction = webEngineView.action(WebEngineView.ChangeTextDirectionRTL);
            verify(rtlAction);
            rtlAction.trigger();
            compare(getInputTextDirection("textfield"), "rtl");

            var ltrAction = webEngineView.action(WebEngineView.ChangeTextDirectionLTR);
            verify(ltrAction);
            ltrAction.trigger();
            compare(getInputTextDirection("textfield"), "ltr");
        }
    }
}
