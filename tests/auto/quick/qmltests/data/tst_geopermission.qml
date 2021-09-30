/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
