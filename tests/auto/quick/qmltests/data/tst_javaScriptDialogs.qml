import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 0.9
import QtWebEngine.experimental 1.0

TestWebEngineView {
    id: webEngineView

    property bool modelMessageEqualsMessage: false
    property string messageFromAlertDialog: ""
    property int confirmCount: 0
    property int promptCount: 0

    experimental {
        alertDialog: Item {
            Component.onCompleted: {
                // Testing both attached property and id defined in the Component context.
                WebEngineView.view.messageFromAlertDialog = message
                parent.modelMessageEqualsMessage = Boolean(model.message == message)
                model.dismiss()
            }
        }

        confirmDialog: Item {
            Component.onCompleted: {
                WebEngineView.view.confirmCount += 1
                if (message == "ACCEPT")
                    model.accept()
                else
                    model.reject()
            }
        }

        promptDialog: Item {
            Component.onCompleted: {
                WebEngineView.view.promptCount += 1
                if (message == "REJECT")
                    model.reject()
                else {
                    var reversedDefaultValue = defaultValue.split("").reverse().join("")
                    model.accept(reversedDefaultValue)
                }
            }
        }
    }

    TestCase {
        id: test
        name: "WebEngineViewJavaScriptDialogs"

        function init() {
            webEngineView.modelMessageEqualsMessage = false
            webEngineView.messageFromAlertDialog = ""
            webEngineView.confirmCount = 0
            webEngineView.promptCount = 0
        }

        function test_alert() {
            webEngineView.url = Qt.resolvedUrl("alert.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.messageFromAlertDialog, "Hello Qt")
            verify(webEngineView.modelMessageEqualsMessage)
        }
/*
        function test_alertWithoutDialog() {
            webEngineView.experimental.alertDialog = null
            webEngineView.url = Qt.resolvedUrl("alert.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.messageFromAlertDialog, "")
        }
*/
        function test_confirm() {
            webEngineView.url = Qt.resolvedUrl("confirm.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.confirmCount, 2)
            compare(webEngineView.title, "ACCEPTED REJECTED")
        }
/*
        function test_confirmWithoutDialog() {
            webEngineView.experimental.confirmDialog = null
            webEngineView.url = Qt.resolvedUrl("confirm.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.confirmCount, 0)
            compare(webEngineView.title, "ACCEPTED ACCEPTED")
        }
*/
        function test_prompt() {
            webEngineView.url = Qt.resolvedUrl("prompt.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.promptCount, 2)
            compare(webEngineView.title, "tQ olleH")
        }
/*
        function test_promptWithoutDialog() {
            webEngineView.experimental.promptDialog = null
            webEngineView.url = Qt.resolvedUrl("prompt.html")
            verify(webEngineView.waitForLoadSucceeded())
            compare(webEngineView.promptCount, 0)
            compare(webEngineView.title, "FAIL")
        }
*/
    }
}
