/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.2
import QtTest 1.0
import QtWebEngine 1.1

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 200

    property bool deniedGeolocation: false
    property bool geoPermissionRequested: false
    property string consoleErrorMessage: ""

    SignalSpy {
        id: featurePermissionSpy
        target: webEngineView
        signalName: "featurePermissionRequested"
    }

    onFeaturePermissionRequested: {
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

    onJavaScriptConsoleMessage: {
        if (level === WebEngineView.ErrorMessageLevel)
            consoleErrorMessage = message
    }

    TestCase {
        name: "WebViewGeopermission"
        when: windowShown

        function init() {
            deniedGeolocation = false
            consoleErrorMessage = ""
            featurePermissionSpy.clear()
        }

        function test_geoPermissionRequest() {
            compare(featurePermissionSpy.count, 0)
            webEngineView.url = Qt.resolvedUrl("geolocation.html")
            featurePermissionSpy.wait()
            verify(geoPermissionRequested)
            compare(featurePermissionSpy.count, 1)
            if (consoleErrorMessage) // Print the error message if it fails to get user's location
                fail(consoleErrorMessage)
        }

        function test_deniedGeolocationByUser() {
            deniedGeolocation = true
            webEngineView.url = Qt.resolvedUrl("geolocation.html")
            featurePermissionSpy.wait()
            compare(consoleErrorMessage, "User denied Geolocation")
        }
    }
}
