// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 400

    settings.screenCaptureEnabled: true

    TestCase {
        name: "GetUserMedia"
        when: windowShown

        function init_data() {
            return [
                {
                    tag: "device audio",
                    constraints: { audio: true },
                    feature: WebEngineView.MediaAudioCapture,
                },
                {
                    tag: "device video",
                    constraints: { video: true },
                    feature: WebEngineView.MediaVideoCapture,
                },
                {
                    tag: "device audio+video",
                    constraints: { audio: true, video: true },
                    feature: WebEngineView.MediaAudioVideoCapture,
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
                    feature: WebEngineView.DesktopVideoCapture,
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
                    feature: WebEngineView.DesktopAudioVideoCapture,
                }
            ]
        }

        function test_getUserMedia(row) {
            loadSync(Qt.resolvedUrl("test1.html"))

            // 1. Rejecting request on QML side should reject promise on JS side.
            jsGetUserMedia(row.constraints)
            tryVerify(function(){ return gotFeatureRequest(row.feature) })
            rejectPendingRequest()
            tryVerify(function(){ return !jsPromiseFulfilled() && jsPromiseRejected() })

            // 2. Accepting request on QML side should either fulfill or reject the
            // Promise on JS side. Due to the potential lack of physical media devices
            // deeper in the content layer we cannot guarantee that the promise will
            // always be fulfilled, however in this case an error should be returned to
            // JS instead of leaving the Promise in limbo.
            jsGetUserMedia(row.constraints)
            tryVerify(function(){ return gotFeatureRequest(row.feature) })
            acceptPendingRequest()
            tryVerify(function(){ return jsPromiseFulfilled() || jsPromiseRejected() });

            // 3. Media feature permissions are not remembered.
            jsGetUserMedia(row.constraints);
            tryVerify(function(){ return gotFeatureRequest(row.feature) })
            acceptPendingRequest()
            tryVerify(function(){ return jsPromiseFulfilled() || jsPromiseRejected() });
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

    property variant requestedFeature
    property variant requestedSecurityOrigin

    onFeaturePermissionRequested: function(securityOrigin, feature) {
        requestedFeature = feature
        requestedSecurityOrigin = securityOrigin
    }

    function gotFeatureRequest(expectedFeature) {
        return requestedFeature == expectedFeature
    }

    function acceptPendingRequest() {
        webEngineView.grantFeaturePermission(requestedSecurityOrigin, requestedFeature, true)
        requestedFeature = undefined
        requestedSecurityOrigin = undefined
    }

    function rejectPendingRequest() {
        webEngineView.grantFeaturePermission(requestedSecurityOrigin, requestedFeature, false)
        requestedFeature = undefined
        requestedSecurityOrigin = undefined
    }

    ////
    // synchronous JavaScript evaluation

    signal runJavaScriptFinished(variant result)

    SignalSpy {
        id: spyOnRunJavaScriptFinished
        target: webEngineView
        signalName: "runJavaScriptFinished"
    }

    function runJavaScriptSync(code) {
        spyOnRunJavaScriptFinished.clear()
        runJavaScript(code, runJavaScriptFinished)
        spyOnRunJavaScriptFinished.wait()
        return spyOnRunJavaScriptFinished.signalArguments[0][0]
    }

    ////
    // JavaScript snippets

    function jsGetUserMedia(constraints) {
        runJavaScript(
            "var promiseFulfilled = false;" +
            "var promiseRejected = false;" +
            "navigator.mediaDevices.getUserMedia(" + JSON.stringify(constraints) + ")" +
            ".then(stream => { promiseFulfilled = true})" +
            ".catch(err => { promiseRejected = true})")
    }

    function jsPromiseFulfilled() {
        return runJavaScriptSync("promiseFulfilled")
    }

    function jsPromiseRejected() {
        return runJavaScriptSync("promiseRejected")
    }
}
