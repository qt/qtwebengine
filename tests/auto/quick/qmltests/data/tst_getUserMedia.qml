// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtWebEngine
import Test.Shared as Shared

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 400

    settings.screenCaptureEnabled: true
    profile.persistentPermissionsPolicy: WebEngineProfile.PersistentPermissionsPolicy.AskEveryTime

    TestCase {
        id: testCase
        name: "GetUserMedia"
        when: windowShown

        function init_data() {
            return [
                {
                    tag: "device audio",
                    constraints: { audio: true },
                    feature: WebEnginePermission.PermissionType.MediaAudioCapture,
                },
                {
                    tag: "device video",
                    constraints: { video: true },
                    feature: WebEnginePermission.PermissionType.MediaVideoCapture,
                },
                {
                    tag: "device audio+video",
                    constraints: { audio: true, video: true },
                    feature: WebEnginePermission.PermissionType.MediaAudioVideoCapture,
                },
                {
                    tag: "desktop video",
                    constraints: {
                        video: {
                            mandatory: {
                                chromeMediaSource: "desktop"
                            }
                        }
                    },
                    feature: WebEnginePermission.PermissionType.DesktopVideoCapture,
                },
                {
                    tag: "desktop audio+video",
                    constraints: {
                        audio: {
                            mandatory: {
                                chromeMediaSource: "desktop"
                            }
                        },
                        video: {
                            mandatory: {
                                chromeMediaSource: "desktop"
                            }
                        }
                    },
                    feature: WebEnginePermission.PermissionType.DesktopAudioVideoCapture,
                }
            ]
        }

        function test_getUserMedia(row) {
            loadSync(Qt.resolvedUrl("test1.html"))

            // 1. Rejecting request on QML side should reject promise on JS side.
            jsGetUserMedia(row.constraints)
            verifyPermissionType(row.feature)
            rejectPendingRequest()
            tryVerify(jsPromiseRejected)

            // 2. Accepting request on QML side should either fulfill or reject the
            // Promise on JS side. Due to the potential lack of physical media devices
            // deeper in the content layer we cannot guarantee that the promise will
            // always be fulfilled, however in this case an error should be returned to
            // JS instead of leaving the Promise in limbo.
            jsGetUserMedia(row.constraints)
            verifyPermissionType(row.feature)
            acceptPendingRequest()
            tryVerify(jsPromiseSettled)

            // 3. Media feature permissions are not remembered.
            jsGetUserMedia(row.constraints);
            verifyPermissionType(row.feature)
            acceptPendingRequest()
            tryVerify(jsPromiseSettled)
        }
    }

    ////
    // synchronous loading

    signal loadFinished

    SignalSpy {
        id: spyOnLoadFinished
        target: webEngineView
        signalName: "loadFinished"
    }

    onLoadingChanged: function(load) {
        if (load.status == WebEngineView.LoadSucceededStatus) {
            loadFinished()
        }
    }

    function loadSync(url) {
        webEngineView.url = url
        spyOnLoadFinished.wait()
    }

    ////
    // synchronous permission requests

    property variant permissionObject
    property bool isDesktopMediaRequestHandled: false
    property bool gotEmptyDesktopMediaRequest: false

    onPermissionRequested: function(perm) {
        permissionObject = perm
    }

    onDesktopMediaRequested: function(request) {
        gotEmptyDesktopMediaRequest = request.screensModel.rowCount() == 0
        if (gotEmptyDesktopMediaRequest)
            request.cancel()
        else
            request.selectScreen(request.screensModel.index(0, 0))
        isDesktopMediaRequestHandled = true
    }

    function verifyPermissionType(expectedFeature) {
        // When webrtc is disabled, desktop media requests come through as non-desktop.
        var isDesktopPermission = Shared.TestEnvironment.hasWebRTC() &&
                (expectedFeature == WebEnginePermission.PermissionType.DesktopAudioVideoCapture ||
                 expectedFeature == WebEnginePermission.PermissionType.DesktopVideoCapture)

        if (isDesktopPermission) {
            testCase.tryVerify(function() { return isDesktopMediaRequestHandled })

            // Request has been cancelled
            if (gotEmptyDesktopMediaRequest) {
                testCase.compare(permissionObject, undefined)
                return
            }
        }

        testCase.tryVerify(function() { return permissionObject != undefined })
        testCase.compare(permissionObject.permissionType, expectedFeature)
    }

    function acceptPendingRequest() {
        if (permissionObject)
            permissionObject.grant()
        resetRequestState()
    }

    function resetRequestState() {
        permissionObject = undefined
        isDesktopMediaRequestHandled = false
        gotEmptyDesktopMediaRequest = false
    }

    function rejectPendingRequest() {
        if (permissionObject)
            permissionObject.deny()
        resetRequestState()
    }

    ////
    // Intercept promise callback results

    SignalSpy {
        id: promiseMessageSpy
        target: webEngineView
        signalName: "javaScriptConsoleMessage"
    }

    function jsPromiseSettled()
    {
        return promiseMessageSpy.count > 0;
    }

    function jsPromiseFulfilled()
    {
        if (!jsPromiseSettled())
            return false;

        return promiseMessageSpy.signalArguments[0][1] === "fulfilled"
    }

    function jsPromiseRejected()
    {
        if (!jsPromiseSettled())
            return false;

        return promiseMessageSpy.signalArguments[0][1] === "rejected"
    }

    ////
    // JavaScript snippets

    function jsGetUserMedia(constraints) {
        promiseMessageSpy.clear();
        runJavaScript(
            "navigator.mediaDevices.getUserMedia(" + JSON.stringify(constraints) + ")" +
            ".then(stream => { console.info('fulfilled') })" +
            ".catch(err => { console.info('rejected') })")
    }
}
