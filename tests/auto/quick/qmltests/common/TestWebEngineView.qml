import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0

WebEngineView {
    property var loadStatus: null
    property var viewportReady: false

    function waitForLoadSucceeded() {
        var success = _waitFor(function() { return loadStatus == WebEngineView.LoadSucceededStatus })
        loadStatus = null
        return success
    }
    function waitForViewportReady() {
        // Note: You need to have "when: windowShown" in your TestCase for this to work.
        // The viewport is locked until the first frame is rendered, and the rendering isn't
        // activated until the WebView is visible in a mapped QQuickView.
        return _waitFor(function() { return viewportReady })
    }
    function waitForLoadFailed() {
        var failure = _waitFor(function() { return loadStatus == WebEngineView.LoadFailedStatus })
        loadStatus = null
        return failure
    }
    function waitForLoadStopped() {
        var stop = _waitFor(function() { return loadStatus == WebEngineView.LoadStoppedStatus })
        loadStatus = null
        return stop
    }
    function _waitFor(predicate) {
        var timeout = 5000
        var i = 0
        while (i < timeout && !predicate()) {
            testResult.wait(50)
            i += 50
        }
        return predicate()
    }

    TestResult { id: testResult }

    onLoadingStateChanged: {
        loadStatus = loadRequest.status
        if (loadRequest.status == WebEngineView.LoadStartedStatus)
            viewportReady = false
    }

}

