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
    height: 200

    TestInputEvent {
        id: testInputEvent

        function __mouseMultiClick(item, x, y, clickCount) {
            if (!item)
                qtest_fail("No item given to mouseMultiClick", 1);

            if (x === undefined)
                x = item.width / 2;
            if (y === undefined)
                y = item.height / 2;
            if (!mouseMultiClick(item, x, y, clickCount))
                qtest_fail("window not shown", 2);
        }

        function mouseDoubleClick(item, x, y) {
            __mouseMultiClick(item, x, y, 2);
        }

        function mouseTripleClick(item, x, y) {
            __mouseMultiClick(item, x, y, 3);
        }

        function mouseQuadraClick(item, x, y) {
            __mouseMultiClick(item, x, y, 4);
        }
    }


    TestCase {
        id: testCase
        name: "WebEngineViewMouseClick"
        when: windowShown

        function test_singleClick() {
            webEngineView.settings.focusOnNavigationEnabled = false;
            webEngineView.loadHtml("<html><body>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());
            verify(!getActiveElementId());

            var center = getElementCenter("input");
            mouseClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            compare(getTextSelection(), "");
        }

        function test_doubleClick() {
            webEngineView.settings.focusOnNavigationEnabled = true;
            webEngineView.loadHtml("<html><body onload='document.getElementById(\"input\").focus()'>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            var center = getElementCenter("input");
            testInputEvent.mouseDoubleClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            tryVerify(function() { return getTextSelection() == "Company" });

            mouseClick(webEngineView, center.x, center.y);
            tryVerify(function() { return getTextSelection() == "" });
        }

        function test_tripleClick() {
            webEngineView.settings.focusOnNavigationEnabled = true;
            webEngineView.loadHtml("<html><body onload='document.getElementById(\"input\").focus()'>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            var center = getElementCenter("input");
            testInputEvent.mouseTripleClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            tryVerify(function() { return getTextSelection() == "The Qt Company" });

            mouseClick(webEngineView, center.x, center.y);
            tryVerify(function() { return getTextSelection() == "" });
        }

        function test_quadraClick() {
            webEngineView.settings.focusOnNavigationEnabled = true;
            webEngineView.loadHtml("<html><body onload='document.getElementById(\"input\").focus()'>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            var center = getElementCenter("input");
            testInputEvent.mouseQuadraClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            tryVerify(function() { return getTextSelection() == "" });
        }
    }
}
