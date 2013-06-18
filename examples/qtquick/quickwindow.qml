import QtQuick 2.0
import QtWebEngine 1.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: browserWindow
    height: 600
    width: 800
    visible: true
    title: webContentsView.title

    toolBar: ToolBar {
        id: navigationBar
        RowLayout {
            anchors.fill: parent

            ToolButton {
                id: backButton
                iconName: "go-previous"
                iconSource: ":/icons/go-previous.png"
                onClicked: webContentsView.goBack()
                enabled: webContentsView.canGoBack
            }
            ToolButton {
                id: forwardButton
                iconName: "go-next"
                iconSource: ":/icons/go-next.png"
                onClicked: webContentsView.goForward()
                enabled: webContentsView.canGoForward
            }
            ToolButton {
                id: reloadButton
                iconName: webContentsView.loading ? "process-stop" : "view-refresh"
                iconSource: webContentsView.loading ? ":/icons/process-stop.png" : ":/icons/view-refresh.png"
                onClicked: webContentsView.reload()
            }
            TextField {
                id: addressBar
                focus: true
                Layout.fillWidth: true

                onAccepted: webContentsView.url = utils.fromUserInput(text)
            }
        }
    }

    WebContentsView {
        id: webContentsView
        focus: true
        anchors.fill: parent
        url: utils.initialUrl()

        onUrlChanged: addressBar.text = url
    }
}
