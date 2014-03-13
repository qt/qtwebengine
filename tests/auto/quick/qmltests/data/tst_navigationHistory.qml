import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.0
import QtWebEngine.experimental 1.0

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    ListView {
        id: backItemsList
        anchors.fill: parent
        model: webEngineView.experimental.navigationHistory.backItems
        delegate:
            Text {
                color:"black"
                text: "title : " + title
            }
    }

    ListView {
        id: forwardItemsList
        anchors.fill: parent
        model: webEngineView.experimental.navigationHistory.forwardItems
        delegate:
            Text {
                color:"black"
                text: "title : " + title
            }
    }

    TestCase {
        name: "WebViewNavigationHistory"

        function test_navigationHistory() {
            compare(webEngineView.loadProgress, 0)

            webEngineView.url = Qt.resolvedUrl("test1.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 0)

            webEngineView.url = Qt.resolvedUrl("test2.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 1)

            webEngineView.experimental.goBackTo(0)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test1.html"))
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, true)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 1)

            webEngineView.goForward()
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 1)
            compare(forwardItemsList.count, 0)

            webEngineView.url = Qt.resolvedUrl("javascript.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("javascript.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 2)
            compare(forwardItemsList.count, 0)

            webEngineView.experimental.goBackTo(0)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test1.html"))
            compare(webEngineView.canGoBack, false)
            compare(webEngineView.canGoForward, true)
            compare(backItemsList.count, 0)
            compare(forwardItemsList.count, 2)

            webEngineView.experimental.goForwardTo(1)
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("javascript.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, false)
            compare(backItemsList.count, 2)
            compare(forwardItemsList.count, 0)

            webEngineView.goBack()
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.url, Qt.resolvedUrl("test2.html"))
            compare(webEngineView.canGoBack, true)
            compare(webEngineView.canGoForward, true)
            compare(backItemsList.count, 1)
            compare(forwardItemsList.count, 1)
        }
    }
}
