// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 200

    property bool deniedGeolocation: false
    property bool geoPermissionRequested: false
    property var permissionObject: undefined

    profile.persistentPermissionsPolicy: WebEngineProfile.PersistentPermissionsPolicy.AskEveryTime

    SignalSpy {
        id: permissionSpy
        target: webEngineView
        signalName: "permissionRequested"
    }

    onPermissionRequested: function(perm) {
        if (perm.permissionType === WebEnginePermission.PermissionType.Geolocation) {
            geoPermissionRequested = true
            permissionObject = perm
            if (deniedGeolocation) {
                perm.deny()
            }
            else {
                perm.grant()
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
            if (permissionObject != undefined) {
                permissionObject.reset()
            }
            deniedGeolocation = false
            permissionSpy.clear()
        }

        function test_geoPermissionRequest() {
            compare(permissionSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("geolocation.html")
            tryCompare(permissionSpy, "count", 1)
            verify(geoPermissionRequested)
            tryVerify(isHandled, 5000)
            verify(getErrorMessage() === "")
        }

        function test_deniedGeolocationByUser() {
            compare(permissionSpy.count, 0)
            deniedGeolocation = true
            webEngineView.url = Qt.resolvedUrl("geolocation.html")
            tryCompare(permissionSpy, "count", 1)
            tryVerify(isHandled, 5000)
            compare(getErrorMessage(), "User denied Geolocation")
        }
    }
}
