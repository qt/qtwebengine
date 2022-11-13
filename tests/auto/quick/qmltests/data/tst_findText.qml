// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    property int matchCount: 0
    property bool findFailed: false

    SignalSpy {
        id: findTextSpy
        target: webEngineView
        signalName: "findTextFinished"
    }

    function clear() {
        findFailed = false
        matchCount = -1
        findTextSpy.clear()
    }

    function findCallbackCalled() { return matchCount != -1 }

    function findTextCallback(matchCount) {
        // If this starts to fail then either clear was not called before findText
        // or unexpected callback was triggered from some search.
        // On c++ side callback id can be checked to verify
        testCase.verify(!findCallbackCalled(), 'Unexpected callback call or uncleared state before findText call!')

        webEngineView.matchCount = matchCount
        findFailed = matchCount == 0
    }


    TestCase {
        id: testCase
        name: "WebViewFindText"

        function getBodyInnerHTML() {
            var bodyInnerHTML;
            runJavaScript("document.body.innerHTML", function(result) {
                bodyInnerHTML = result;
            });
            tryVerify(function() { return bodyInnerHTML != undefined; }, 20000);
            return bodyInnerHTML;
        }

        function getListItemText(index) {
            var listItemText;
            runJavaScript("document.getElementById('list').getElementsByTagName('li')[" + index + "].innerText;", function(result) {
                listItemText = result;
            });
            tryVerify(function() { return listItemText != undefined; }, 20000);
            return listItemText;
        }

        function appendListItem(text) {
            var script =
                    "(function () {" +
                    "   var list = document.getElementById('list');" +
                    "   var item = document.createElement('li');" +
                    "   item.appendChild(document.createTextNode('" + text + "'));" +
                    "   list.appendChild(item);" +
                    "   return list.getElementsByTagName('li').length - 1;" +
                    "})();";
            var itemIndex;

            runJavaScript(script, function(result) { itemIndex = result; });
            tryVerify(function() { return itemIndex != undefined; }, 20000);
            // Make sure the DOM is up-to-date.
            tryVerify(function() { return getListItemText(itemIndex).length == text.length; }, 20000);
        }

        function test_findText() {
            var findFlags = WebEngineView.FindCaseSensitively
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("Hello", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 1)
            verify(!findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 1)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 1)
        }

        function test_findTextCaseInsensitive() {
            var findFlags = 0
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("heLLo", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 1)
            verify(!findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 1)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 1)
        }

        function test_findTextManyMatches() {
            var findFlags = 0
            webEngineView.url = Qt.resolvedUrl("test4.html")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("bla", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 100, 20000)
            verify(!findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 100)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 1)
        }


        function test_findTextFailCaseSensitive() {
            var findFlags = WebEngineView.FindCaseSensitively
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("heLLo", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 0)
            verify(findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 0)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 0)
        }

        function test_findTextNotFound() {
            var findFlags = 0
            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("string-that-is-not-threre", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 0)
            verify(findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 0)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 0)
        }

        function test_findTextAfterNotFound() {
            var findFlags = 0
            webEngineView.url = Qt.resolvedUrl("about:blank")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("hello", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 0)
            verify(findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 0)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 0)

            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("hello", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 1)
            verify(!findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 1)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 1)
        }

        function test_findTextInModifiedDOMAfterNotFound() {
            var findFlags = 0
            webEngineView.loadHtml(
                        "<html><body>" +
                        "bla" +
                        "</body></html>");
            verify(webEngineView.waitForLoadSucceeded())

            webEngineView.clear()
            webEngineView.findText("hello", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 0, 20000)
            verify(findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 0)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 0)

            runJavaScript("document.body.innerHTML = 'blahellobla'");
            tryVerify(function() { return getBodyInnerHTML() == "blahellobla"; }, 20000);

            webEngineView.clear()
            webEngineView.findText("hello", findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, "matchCount", 1)
            verify(!findFailed)
            tryCompare(findTextSpy, "count", 1)
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 1)
            compare(findTextSpy.signalArguments[0][0].activeMatch, 1)
        }

        function test_findTextInterruptedByLoad() {
            var findFlags = 0;

            var listItemText = '';
            for (var i = 0; i < 100000; ++i)
                listItemText += "bla ";
            listItemText = listItemText.trim();

            webEngineView.loadHtml(
                        "<html><body>" +
                        "<ol id='list' />" +
                        "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            // Generating a huge list is a workaround to avoid timeout while loading the test page.
            for (var i = 0; i < 10; ++i)
                appendListItem(listItemText);
            appendListItem("hello");

            webEngineView.clear();
            webEngineView.findText("hello", findFlags, webEngineView.findTextCallback);

            // This should not crash.
            webEngineView.loadHtml("<html><body>New page with same hello text</body></html>")
            verify(webEngineView.waitForLoadSucceeded())

            // The callback is not supposed to be called, see QTBUG-61506.
            expectFailContinue('', 'No unexpected findText callback calls occurred.')
            tryVerify(function() { return webEngineView.findCallbackCalled() })
            verify(!webEngineView.findCallbackCalled())

            webEngineView.clear();
            webEngineView.findText('New page', findFlags, webEngineView.findTextCallback)
            tryCompare(webEngineView, 'matchCount', 1)
        }

        function test_findTextActiveMatchOrdinal() {
            webEngineView.loadHtml(
                        "<html><body>" +
                        "foo bar foo bar foo" +
                        "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            // Iterate over all "foo" matches.
            webEngineView.clear();
            for (var i = 1; i <= 3; ++i) {
                webEngineView.findText("foo");
                findTextSpy.wait();
                compare(findTextSpy.count, i);
                compare(findTextSpy.signalArguments[i-1][0].numberOfMatches, 3);
                compare(findTextSpy.signalArguments[i-1][0].activeMatch, i);
            }

            // The last match is followed by the fist one.
            webEngineView.clear();
            webEngineView.findText("foo");
            findTextSpy.wait();
            compare(findTextSpy.count, 1);
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 3);
            compare(findTextSpy.signalArguments[0][0].activeMatch, 1);

            // The first match is preceded by the last one.
            webEngineView.clear();
            webEngineView.findText("foo", WebEngineView.FindBackward);
            findTextSpy.wait();
            compare(findTextSpy.count, 1);
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 3);
            compare(findTextSpy.signalArguments[0][0].activeMatch, 3);

            // Finding another word resets the activeMatch.
            webEngineView.clear();
            webEngineView.findText("bar");
            findTextSpy.wait();
            compare(findTextSpy.count, 1);
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 2);
            compare(findTextSpy.signalArguments[0][0].activeMatch, 1);

            // If no match activeMatch is 0.
            webEngineView.clear();
            webEngineView.findText("bla");
            findTextSpy.wait();
            compare(findTextSpy.count, 1);
            compare(findTextSpy.signalArguments[0][0].numberOfMatches, 0);
            compare(findTextSpy.signalArguments[0][0].activeMatch, 0);
        }
    }
}
