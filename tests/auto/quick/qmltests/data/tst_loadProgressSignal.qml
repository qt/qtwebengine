import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    SignalSpy {
        id: spyProgress
        target: webEngineView
        signalName: "loadProgressChanged"
    }

    TestCase {
        name: "WebEngineViewLoadProgressSignal"

        function test_loadProgressSignal() {
            compare(spyProgress.count, 0)
            compare(webEngineView.loadProgress, 0)
            webEngineView.url = Qt.resolvedUrl("test1.html")
            spyProgress.wait()
            compare(true, webEngineView.loadProgress > -1 && webEngineView.loadProgress < 101)
            if (webEngineView.loadProgress > 0 && webEngineView.loadProgress < 100) {
                verify(webEngineView.waitForLoadSucceeded())
                spyProgress.wait()
                compare(webEngineView.loadProgress, 100)
            }
        }
    }
}
