import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0
import "../common"

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300


    SignalSpy {
        id: spyTitle
        target: webEngineView
        signalName: "titleChanged"
    }


    TestCase {
        name: "WebEngineViewTitleChangedSignal"

        function test_titleFirstLoad() {
            compare(spyTitle.count, 0)

            var testUrl = Qt.resolvedUrl("../common/test3.html")
            webEngineView.url = testUrl
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.title, "Test page 3")
            spyTitle.clear()

            spyTitle.wait()
            compare(webEngineView.title, "New Title")
        }
    }
}
