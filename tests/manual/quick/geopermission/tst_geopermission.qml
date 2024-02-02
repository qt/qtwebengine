// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

WebEngineView {
    id: webEngineView
    width: 200
    height: 200
    url: Qt.resolvedUrl("qrc:/geolocation.html")
    property bool deniedGeolocation: false
    property bool geoPermissionRequested: false

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

}
