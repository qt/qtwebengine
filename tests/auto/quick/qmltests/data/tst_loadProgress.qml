import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    TestCase {
        name: "WebEngineViewLoadProgress"

        function test_loadProgress() {
            compare(webEngineView.loadProgress, 0)
            webEngineView.url = Qt.resolvedUrl("test1.html")
            compare(webEngineView.loadProgress, 0)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.loadProgress, 100)
        }
    }
}
