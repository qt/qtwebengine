import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    TestCase {
        name: "WebEngineViewLoadHtml"

        function test_loadProgressAfterLoadHtml() {
            compare(webEngineView.loadProgress, 0)
            webEngineView.loadHtml("<html><head><title>Test page 1</title></head><body>Hello.</body></html>")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.loadProgress, 100)
        }
    }
}
