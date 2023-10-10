// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

WebEngineView {
    property var loadStatus: null
    property bool windowCloseRequestedSignalEmitted: false
    settings.focusOnNavigationEnabled: true

    function loadSucceeded() { return loadStatus == WebEngineView.LoadSucceededStatus }
    function loadFailed() { return loadStatus == WebEngineView.LoadFailedStatus }
    function loadStopped() { return loadStatus == WebEngineView.LoadStoppedStatus }

    function waitForLoadResult(timeout) {
        loadStatus = null
        var r = _waitFor(function() { return loadStatus != null && loadStatus != WebEngineView.LoadStartedStatus }, timeout)
        return r
    }

    function waitForLoadSucceeded(timeout) {
        loadStatus = null
        var success = _waitFor(function() { return loadStatus == WebEngineView.LoadSucceededStatus }, timeout)
        return success
    }
    function waitForLoadFailed(timeout) {
        loadStatus = null
        var failure = _waitFor(function() { return loadStatus == WebEngineView.LoadFailedStatus }, timeout)
        return failure
    }
    function waitForLoadStopped(timeout) {
        loadStatus = null
        var stop = _waitFor(function() { return loadStatus == WebEngineView.LoadStoppedStatus }, timeout)
        return stop
    }
    function waitForWindowCloseRequested() {
        return _waitFor(function() { return windowCloseRequestedSignalEmitted; });
    }
    function _waitFor(predicate, timeout) {
        if (timeout === undefined)
            timeout = 30000;
        var i = 0
        while (i < timeout && !predicate()) {
            testResult.wait(50)
            i += 50
        }
        return predicate()
    }

    function getActiveElementId() {
        var activeElementId;
        runJavaScript("document.activeElement.id", function(result) {
            activeElementId = result;
        });
        testCase.tryVerify(function() { return activeElementId != undefined });
        return activeElementId;
    }

    function verifyElementHasFocus(element) {
        testCase.tryVerify(function() { return getActiveElementId() == element; }, 5000,
            "Element \"" + element + "\" has focus");
    }

    function setFocusToElement(element) {
        runJavaScript("document.getElementById('" + element + "').focus()");
        verifyElementHasFocus(element);
    }

    function getElementCenter(element) {
            var center;
            testCase.tryVerify(function() {
                runJavaScript("(function() {" +
                          "   var elem = document.getElementById('" + element + "');" +
                          "   var rect = elem.getBoundingClientRect();" +
                          "   return { 'x': (rect.left + rect.right) / 2, 'y': (rect.top + rect.bottom) / 2 };" +
                          "})();", function(result) { center = result } );
                return center !== undefined;
            });
            return center;
    }

    function getTextSelection() {
        var textSelection;
        runJavaScript("window.getSelection().toString()", function(result) { textSelection = result });
        testCase.tryVerify(function() { return textSelection !== undefined; });
        return textSelection;
    }

    function getElementValue(element) {
        var elementValue;
        runJavaScript("document.getElementById('" + element + "').value", function(result) {
            elementValue = result;
        });
        testCase.tryVerify(function() { return elementValue != undefined; });
        return elementValue;
    }

    function compareElementValue(element, expected) {
        testCase.tryVerify(function() { return expected == getElementValue(element); }, 5000,
            "Value of element \"" + element + "\" is \"" + expected + "\"");
    }


    TestResult { id: testResult }

    onLoadingChanged: function(load) {
        loadStatus = load.status
    }

    onWindowCloseRequested: {
        windowCloseRequestedSignalEmitted = true;
    }

    function getBodyText() {
        let text
        runJavaScript('document.body.innerText', function(t) { text = t })
        testCase.tryVerify(function() { return text !== undefined })
        return text
    }

    function getItemPixel(item) {
        var grabImage = Qt.createQmlObject("
                import QtQuick\n
                Image { }", testCase)
        var itemCanvas = Qt.createQmlObject("
                import QtQuick\n
                Canvas { }", testCase)

        // Mark QML images with objectName: "image" to be able to check if the image is loaded.
        if (item.objectName === "image") {
            testCase.tryVerify(function() { return item.status === Image.Ready });
        }

        item.grabToImage(function(result) {
                grabImage.source = result.url
            });
        testCase.tryVerify(function() { return grabImage.status === Image.Ready });

        itemCanvas.width = item.width;
        itemCanvas.height = item.height;
        var ctx = itemCanvas.getContext("2d");
        ctx.drawImage(grabImage, 0, 0, grabImage.width, grabImage.height);
        var imageData = ctx.getImageData(Math.round(itemCanvas.width/2),
                                         Math.round(itemCanvas.height/2),
                                         itemCanvas.width,
                                         itemCanvas.height);

        grabImage.destroy();
        itemCanvas.destroy();

        return imageData.data;
    }
}

