// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

Rectangle {
    id: root
    width: 200
    height: 200

    Column {
        anchors.fill: parent
        Rectangle {
            id: placeHolder
            width: parent.width
            height: 100
            color: "red"
        }

        TestWebEngineView {
            id: webEngineView
            width: parent.width
            height: 100

            function getInnerText(element) {
                var innerText;
                runJavaScript("document.getElementById('" + element + "').innerText", function(result) {
                    innerText = result;
                });
                testCase.tryVerify(function() { return innerText != undefined; });
                return innerText;
            }
        }
    }

    TestCase {
        id: testCase
        name: "WebEngineViewMouseMove"
        when: windowShown

        function test_mouseLeave() {
            mouseMove(root, 0, 0);
            webEngineView.loadHtml(
                        "<html>" +
                        "<head><script>" +
                        "function init() {" +
                        " var div = document.getElementById('testDiv');" +
                        " div.onmouseenter = function(e) { div.innerText = 'Mouse IN' };" +
                        " div.onmouseleave = function(e) { div.innerText = 'Mouse OUT' };" +
                        "}" +
                        "</script></head>" +
                        "<body onload='init()' style='margin: 0px; padding: 0px'>" +
                        " <div id='testDiv' style='width: 100%; height: 100%; background-color: green' />" +
                        "</body>" +
                        "</html>");
            verify(webEngineView.waitForLoadSucceeded());
            // Make sure the testDiv text is empty.
            webEngineView.runJavaScript("document.getElementById('testDiv').innerText = ''");
            tryVerify(function() { return !webEngineView.getInnerText("testDiv") });

            for (var i = 90; i < 110; ++i)
                mouseMove(root, 50, i);
            tryVerify(function() { return webEngineView.getInnerText("testDiv") == "Mouse IN" });

            for (var i = 110; i > 90; --i)
                mouseMove(root, 50, i);
            tryVerify(function() { return webEngineView.getInnerText("testDiv") == "Mouse OUT" });
        }
    }
}

