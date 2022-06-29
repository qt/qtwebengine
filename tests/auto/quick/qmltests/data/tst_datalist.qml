// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtQuick.Controls
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    property string html: "<html><body>" +
                          "<input id='browserInput' list='browserDatalist'>" +
                          "<datalist id='browserDatalist'>" +
                          "  <option value='Internet Explorer'>" +
                          "  <option value='Firefox'>" +
                          "  <option value='Chrome'>" +
                          "  <option value='Opera'>" +
                          "  <option value='Safari'>" +
                          "</datalist>" +
                          "</body></html>"

    function listView() {
        if (webEngineView.parent.visibleChildren.length == 1) {
            // No popup case.
            return null;
        }

        let overlay = null;
        for (let i = 0; i < webEngineView.parent.visibleChildren.length; ++i) {
            let child = webEngineView.parent.visibleChildren[i];
            if (child instanceof Overlay) {
                overlay = child;
                break;
            }
        }

        if (!overlay)
            return null;

        let popupItem = null;
        for (let i = 0; i < overlay.visibleChildren[0].visibleChildren.length; ++i) {
            let child = overlay.visibleChildren[0].visibleChildren[i];
            if (child.objectName == "QQuickPopupItem") {
                popupItem = child;
            }
        }

        if (!popupItem)
            return null;

        for (let i = 0; i < popupItem.visibleChildren.length; ++i) {
            let child = popupItem.visibleChildren[i];
            if (child instanceof ListView)
                return child;
        }

        return null;
    }

    TestCase {
        id: testCase
        name: "WebEngineDatalist"
        when: windowShown

        function test_showAndHide() {
            webEngineView.loadHtml(webEngineView.html);
            verify(webEngineView.waitForLoadSucceeded());

            var values = "";
            webEngineView.runJavaScript(
                        "(function() {" +
                        "  var browserDatalist = document.getElementById('browserDatalist');" +
                        "  var options = browserDatalist.options;" +
                        "  var result = [];" +
                        "  for (let i = 0; i < options.length; ++i) {" +
                        "    result.push(options[i].value);" +
                        "  }" +
                        "  return result;" +
                        "})();", function(result) { values = result; });
            tryVerify(function() { return values.length != 0; });
            compare(values, ["Internet Explorer", "Firefox", "Chrome", "Opera", "Safari"]);
            compareElementValue("browserInput", "");

            // Make sure there is no open popup yet.
            verify(!listView());
            // Click in the input field.
            var browserInputCenter = getElementCenter("browserInput");
            mouseClick(webEngineView, browserInputCenter.x, browserInputCenter.y, Qt.LeftButton);
            // Wait for the popup.
            tryVerify(function() { return listView() != null; });

            // No suggestion is selected.
            verify(!listView().currentItem);
            compare(listView().count, 5);

            // Accepting suggestion does nothing.
            keyClick(Qt.Key_Enter);
            tryVerify(function() { return listView() != null; });
            verify(!listView().currentItem);

            // Escape should close popup.
            keyClick(Qt.Key_Escape);
            tryVerify(function() { return listView() == null; });

            // Key Down should open the popup and select the first suggestion.
            keyClick(Qt.Key_Down);
            tryVerify(function() { return listView() != null; });
            compare(listView().currentIndex, 0);
            verify(listView().currentItem);
        }

        function test_keyboardNavigationAndAccept() {
            webEngineView.loadHtml(html);
            verify(webEngineView.waitForLoadSucceeded());
            setFocusToElement("browserInput");

            // Make sure there is no open popup yet.
            verify(!listView());

            // Key Down should open the popup and select the first suggestion.
            keyClick(Qt.Key_Down);
            tryVerify(function() { return listView() != null; });
            compare(listView().currentIndex, 0);

            // Test keyboard navigation in list.
            keyClick(Qt.Key_Up);
            compare(listView().currentIndex, 4);
            keyClick(Qt.Key_Up);
            compare(listView().currentIndex, 3);
            keyClick(Qt.Key_PageDown);
            compare(listView().currentIndex, 4);
            keyClick(Qt.Key_PageUp);
            compare(listView().currentIndex, 0);
            keyClick(Qt.Key_Down);
            compare(listView().currentIndex, 1);
            keyClick(Qt.Key_Down);
            compare(listView().currentIndex, 2);

            // Test accepting suggestion.
            compare(listView().currentItem.text, "Chrome");
            keyClick(Qt.Key_Enter);
            compareElementValue("browserInput", "Chrome");
            // Accept closes popup.
            tryVerify(function() { return listView() == null; });

            // Clear input field, should not trigger popup.
            webEngineView.runJavaScript("document.getElementById('browserInput').value = ''");
            compareElementValue("browserInput", "");
            verify(listView() == null);
        }

        function test_filterSuggestion() {
            webEngineView.loadHtml(html);
            verify(webEngineView.waitForLoadSucceeded());
            setFocusToElement("browserInput");

            // Make sure there is no open popup yet.
            verify(!listView());

            // Filter suggestions.
            keyClick(Qt.Key_F);
            tryVerify(function() { return listView() != null; });
            compare(listView().count, 2);
            verify(!listView().currentItem);
            compare(listView().itemAtIndex(0).text, "Firefox");
            compare(listView().itemAtIndex(1).text, "Safari");
            keyClick(Qt.Key_I);
            tryVerify(function() { return listView().count == 1; });
            verify(!listView().currentItem);
            compare(listView().itemAtIndex(0).text, "Firefox");
            keyClick(Qt.Key_L);
            // Mismatch should close popup.
            tryVerify(function() { return listView() == null; });
            compareElementValue("browserInput", "fil");
        }
    }
}
