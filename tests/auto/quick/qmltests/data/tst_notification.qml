// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
    property var permissionObject

    profile.persistentPermissionsPolicy: WebEngineProfile.PersistentPermissionsPolicy.AskEveryTime

    signal consoleMessage(string message)

    SignalSpy {
        id: spyRequest
        target: view
        signalName: 'permissionRequested'
    }

    onPermissionRequested: function(perm) {
        if (perm.permissionType === WebEnginePermission.PermissionType.Notifications) {
            view.permissionRequested = true
            view.permissionObject = perm
            if (grantPermission)
                perm.grant()
            else
                perm.deny()
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
            if (permissionObject != undefined) {
                permissionObject.reset()
            }
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

            let result = {}

            view.runJavaScript('getPermission()', function (permission) { result.permission = permission })
            tryCompare(result, 'permission', 'default')

            view.runJavaScript('requestPermission()')
            tryCompare(spyRequest, "count", 1)
            verify(permissionRequested)

            view.runJavaScript('getPermission()', function (permission) { result.permission = permission })
            tryCompare(result, 'permission', data.permission)
        }

        function test_notification() {
            grantPermission = true

            view.url = resolverUrl('notification.html')
            view.waitForLoadSucceeded()

            view.runJavaScript('requestPermission()')
            tryCompare(spyRequest, "count", 1)
            verify(permissionRequested)

            let title = 'Title', message = 'Message', notification = null
            view.profile.presentNotification.connect(function (n) { notification = n })

            view.runJavaScript('sendNotification("' + title + '", "' + message + '")')
            tryVerify(function () { return notification !== null })
            compare(notification.title, title)
            compare(notification.message, message)
            compare(notification.direction, Qt.RightToLeft)
            compare(notification.origin, permissionObject.origin)
            compare(notification.tag, 'tst')
            compare(notification.language, 'de')
        }
    }
}
