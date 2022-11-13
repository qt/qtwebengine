// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 200

    property bool deniedGeolocation: false
    property bool geoPermissionRequested: false

    SignalSpy {
        id: featurePermissionSpy
        target: webEngineView
        signalName: "featurePermissionRequested"
    }

    onFeaturePermissionRequested: function(securityOrigin, feature) {
        if (feature === WebEngineView.Geolocation) {
            geoPermissionRequested = true
            if (deniedGeolocation) {
                webEngineView.grantFeaturePermission(securityOrigin, feature, false)
            }
            else {
                webEngineView.grantFeaturePermission(securityOrigin, feature, true)
            }
        }
    }

    TestCase {
        name: "WebViewGeopermission"
        when: windowShown

        function isHandled() {
            var handled;
            runJavaScript("handled", function(result) {
                handled = result;
            });
            tryVerify(function() { return handled != undefined; }, 5000);
            return handled;
        }

        function getErrorMessage() {
            var errorMessage;
            runJavaScript("errorMessage", function(result) {
                errorMessage = result;
            });
            tryVerify(function() { return errorMessage != undefined; }, 5000);
            return errorMessage;
        }

        function init() {
            deniedGeolocation = false
            featurePermissionSpy.clear()
        }

        function test_geoPermissionRequest() {
            compare(featurePermissionSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("geolocation.html")
            featurePermissionSpy.wait()
            verify(geoPermissionRequested)
            compare(featurePermissionSpy.count, 1)
            tryVerify(isHandled, 5000)
            verify(getErrorMessage() === "")
        }

        function test_deniedGeolocationByUser() {
            deniedGeolocation = true
            webEngineView.url = Qt.resolvedUrl("geolocation.html")
            featurePermissionSpy.wait()
            tryVerify(isHandled, 5000)
            compare(getErrorMessage(), "User denied Geolocation")
        }
    }
}
