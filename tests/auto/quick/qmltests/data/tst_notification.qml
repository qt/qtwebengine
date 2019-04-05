/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

import QtQuick 2.2
import QtTest 1.0
import QtWebEngine 1.9

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

    onFeaturePermissionRequested: {
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
            return Qt.resolvedUrl('../../../shared/data/' + html)
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
