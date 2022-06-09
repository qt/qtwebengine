// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtTest
import QtWebEngine

import Test.Shared as Shared

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    property var loadProgressArray: []

    onLoadProgressChanged: {
        loadProgressArray.push(webEngineView.loadProgress)
    }

    SignalSpy {
        id: spyProgress
        target: webEngineView
        signalName: "loadProgressChanged"
    }

    TestCase {
        name: "WebEngineViewLoadProgress"

        function test_loadProgress() {
            compare(webEngineView.loadProgress, 0)
            compare(spyProgress.count, 0)
            loadProgressArray = []

            verify(Shared.HttpServer.start())
            Shared.HttpServer.newRequest.connect(request => {
                wait(250) // just add delay to trigger some progress for every sub resource
            })
            webEngineView.url = Shared.HttpServer.url('/loadprogress/main.html')
            // Wait for the first loadProgressChanged signal, which have to be non-negative
            spyProgress.wait()
            compare(loadProgressArray[0], 0)
            verify(webEngineView.loadProgress >= 0)

            // Wait for the last loadProgressChanged signal, which have to be 100%
            verify(webEngineView.waitForLoadSucceeded())
            spyProgress.wait()
            compare(loadProgressArray[loadProgressArray.length - 1], 100)
            compare(webEngineView.loadProgress, 100)

            // Test whether the chromium emits progress numbers in strict monotonic ascending order
            let progress = 0
            for (let i = 1; i < loadProgressArray.length; ++i) {
                let nextProgress = loadProgressArray[i]
                if (nextProgress <= progress)
                    fail("Invalid sequence of progress-values: " + loadProgressArray)
                progress = nextProgress
            }
        }
    }
}
