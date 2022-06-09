// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import Test.util
import "../../qmltests/data"

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    TestInputContext { id: testInputContext }

    TestCase {
        id: testCase
        name: "WebEngineViewInputMethod"
        when: windowShown

        function init() {
            testInputContext.create();
        }

        function cleanup() {
            testInputContext.release();
        }

        function test_softwareInputPanel() {
            verify(!Qt.inputMethod.visible);
            webEngineView.loadHtml(
                        "<html><body>" +
                        "   <form><input id='textInput' type='text' /></form>" +
                        "</body></html");
            verify(webEngineView.waitForLoadSucceeded());

            verify(!getActiveElementId());
            verify(!Qt.inputMethod.visible);

            // Show input panel
            webEngineView.runJavaScript("document.getElementById('textInput').focus()");
            webEngineView.verifyElementHasFocus("textInput");
            tryVerify(function() { return Qt.inputMethod.visible; });

            // Hide input panel
            webEngineView.runJavaScript("document.getElementById('textInput').blur()");
            verify(!getActiveElementId());
            tryVerify(function() { return !Qt.inputMethod.visible; });
        }
    }
}

