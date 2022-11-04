// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine
import Test.Shared as Shared

TestWebEngineView {
    id: view
    width: 320
    height: 320

    property bool permissionRequested: false
    property bool grantPermission: false
    property url securityOrigin: ''

    signal consoleMessage(string message)

    SignalSpy {
        id: spyRequest
        target: view
        signalName: 'featurePermissionRequested'
    }

    onFeaturePermissionRequested: function(securityOrigin, feature) {
        if (feature === WebEngineView.Notifications) {
            view.permissionRequested = true
            view.securityOrigin = securityOrigin
            view.grantFeaturePermission(securityOrigin, feature, grantPermission)
        }
    }

    TestCase {
        name: 'WebEngineNotification'
        when: windowShown

        function resolverUrl(html) {
            console.log(Shared.HttpServer.sharedDataDir())
            return Qt.resolvedUrl(Shared.HttpServer.sharedDataDir() + "/" + html)
        }

        function init() {
            permissionRequested = false
            spyRequest.clear()
        }

        function test_request_data() {
            return [
                { tag: 'grant', grant: true, permission: 'granted' },
                { tag: 'deny', grant: false, permission: 'denied' },
            ]
        }

        function test_request(data) {
            grantPermission = data.grant

            view.url = resolverUrl('notification.html')
            verify(view.waitForLoadSucceeded())

            view.runJavaScript('resetPermission()')
            let result = {}

            view.runJavaScript('getPermission()', function (permission) { result.permission = permission })
            tryCompare(result, 'permission', 'default')

            view.runJavaScript('requestPermission()')
            spyRequest.wait()
            verify(permissionRequested)
            compare(spyRequest.count, 1)

            view.runJavaScript('getPermission()', function (permission) { result.permission = permission })
            tryCompare(result, 'permission', data.permission)
        }

        function test_notification() {
            grantPermission = true

            view.url = resolverUrl('notification.html')
            view.waitForLoadSucceeded()

            view.runJavaScript('requestPermission()')
            spyRequest.wait()
            verify(permissionRequested)

            let title = 'Title', message = 'Message', notification = null
            view.profile.presentNotification.connect(function (n) { notification = n })

            view.runJavaScript('sendNotification("' + title + '", "' + message + '")')
            tryVerify(function () { return notification !== null })
            compare(notification.title, title)
            compare(notification.message, message)
            compare(notification.direction, Qt.RightToLeft)
            compare(notification.origin, securityOrigin)
            compare(notification.tag, 'tst')
            compare(notification.language, 'de')
        }
    }
}
