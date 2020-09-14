import QtQuick 2.2
import QtTest 1.0
import QtWebEngine 1.9

TestWebEngineView {
    id: view
    width: 320
    height: 320

    property bool acceptRequest: false

    onFullScreenRequested: function(request) {
        view.acceptRequest ? request.accept() : request.reject()
    }

    SignalSpy {
        id: spyRequest
        target: view
        signalName: 'fullScreenRequested'
    }

    TestCase {
        name: 'FullScreenRequest'
        when: windowShown

        function init() {
            spyRequest.clear()
        }

        function test_request_data() {
            return [
                { tag: 'accept', accept: true },
                { tag: 'reject', accept: false },
            ]
        }

        function test_request(data) {
            view.acceptRequest = data.accept
            view.settings.fullscreenSupportEnabled = true

            // full screen request is only allowed by user gesture, so emulate key press
            view.loadHtml(
                '<html><body onkeypress="onKeyPress()"><a id="a">WRYYYY</a><script>' +
                'function onKeyPress() {' +
                ' document.webkitIsFullScreen'+
                '  ? document.webkitExitFullscreen()' +
                '  : document.documentElement.webkitRequestFullScreen()' +
                '} </script></body></html>')
            view.waitForLoadSucceeded()
            verify(!view.isFullScreen)

            let result = null
            view.runJavaScript('document.webkitFullscreenEnabled', function(r) { result = r })
            tryVerify(function() { return result === true })

            result = null
            view.runJavaScript('document.webkitIsFullScreen', function(r) { result = r })
            tryVerify(function() { return result === false })

            // will trigger full screen request through key press
            keyClick(Qt.Key_Space)
            spyRequest.wait()
            compare(spyRequest.count, 1)
            verify(spyRequest.signalArguments[0][0].toggleOn)
            compare(data.accept, view.isFullScreen)

            view.runJavaScript('document.webkitIsFullScreen', function(r) { result = r })
            tryVerify(function() { return result === data.accept })

            if (data.accept) {
                // expected to toggle from current state
                keyClick(Qt.Key_Space)
                spyRequest.wait()
                compare(spyRequest.count, 2)
                verify(!spyRequest.signalArguments[1][0].toggleOn)
                view.runJavaScript('document.webkitIsFullScreen', function(r) { result = r })
                tryVerify(function() { return result === false })
                verify(!view.isFullScreen)
            }
        }
    }
}
