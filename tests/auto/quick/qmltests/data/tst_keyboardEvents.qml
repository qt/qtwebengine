// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 350
    height: 480

    TestCase {
        id: testCase
        name: "WebEngineViewKeyboardEvents"
        when: windowShown

        function isElementChecked(element) {
            var elementChecked;
            runJavaScript("document.getElementById('" + element + "').checked", function(result) {
                elementChecked = result;
            });
            tryVerify(function() { return elementChecked != undefined; });
            return elementChecked;
        }

        function verifyElementChecked(element, expected) {
            tryVerify(function() { return expected == isElementChecked(element); }, 5000,
                "Element \"" + element + "\" is " + (expected ? "" : "not") + " checked");
        }

        function test_keyboardEvents() {
            webEngineView.url = Qt.resolvedUrl("keyboardEvents.html");
            verify(webEngineView.waitForLoadSucceeded());

            var elements = [
                        "first_div", "second_div",
                        "text_input", "radio1", "checkbox1", "checkbox2",
                        "number_input", "range_input", "search_input",
                        "submit_button", "combobox", "first_hyperlink", "second_hyperlink"
                    ];

            // Iterate over the elements of the test page with the Tab key. This tests whether any
            // element blocks the in-page navigation by Tab.
            for (var i = 0; i < elements.length; ++i) {
                verifyElementHasFocus(elements[i])
                keyPress(Qt.Key_Tab);
            }

            // Move back to the radio buttons with the Shift+Tab key combination
            for (var i = 0; i < 10; ++i)
                keyPress(Qt.Key_Tab, Qt.ShiftModifier);
            verifyElementHasFocus("radio2");

            // Test the Space key by checking a radio button
            verifyElementChecked("radio2", false);
            keyClick(Qt.Key_Space);
            verifyElementChecked("radio2", true);

            // Test the Left key by switching the radio button
            verifyElementChecked("radio1", false);
            keyPress(Qt.Key_Left);
            verifyElementHasFocus("radio1");
            verifyElementChecked("radio1", true);

            // Test the Space key by unchecking a checkbox
            setFocusToElement("checkbox1");
            verifyElementChecked("checkbox1", true);
            keyClick(Qt.Key_Space);
            verifyElementChecked("checkbox1", false);

            // Test the Up and Down keys by changing the value of a spinbox
            setFocusToElement("number_input");
            compareElementValue("number_input", 5);
            keyPress(Qt.Key_Up);
            compareElementValue("number_input", 6);
            keyPress(Qt.Key_Down);
            compareElementValue("number_input", 5);

            // Test the Left, Right, Home, PageUp, End and PageDown keys by changing the value of a slider
            setFocusToElement("range_input");
            compareElementValue("range_input", 5);
            keyPress(Qt.Key_Left);
            compareElementValue("range_input", 4);
            keyPress(Qt.Key_Right);
            compareElementValue("range_input", 5);
            keyPress(Qt.Key_Home);
            compareElementValue("range_input", 0);
            keyPress(Qt.Key_PageUp);
            compareElementValue("range_input", 1);
            keyPress(Qt.Key_End);
            compareElementValue("range_input", 10);
            keyPress(Qt.Key_PageDown);
            compareElementValue("range_input", 9);

            // Test the Escape key by removing the content of a search field
            setFocusToElement("search_input");
            compareElementValue("search_input", "test");
            keyPress(Qt.Key_Escape);
            compareElementValue("search_input", "");

            // Test the alpha keys by changing the values in a combobox
            setFocusToElement("combobox");
            compareElementValue("combobox", "a");
            keyPress(Qt.Key_B);
            compareElementValue("combobox", "b");
            // Must wait with the second key press to simulate selection of another element
            wait(1100); // blink::typeAheadTimeout + 0.1s
            keyPress(Qt.Key_C);
            compareElementValue("combobox", "c");

            // Test the Enter key by loading a page with a hyperlink
            setFocusToElement("first_hyperlink");
            keyPress(Qt.Key_Enter);
            verify(webEngineView.waitForLoadSucceeded());
        }
    }
}
