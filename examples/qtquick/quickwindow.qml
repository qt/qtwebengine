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
            }
            ToolButton {
                id: forwardButton
                iconName: "go-next"
                iconSource: ":/icons/go-next.png"
                onClicked: webContentsView.goForward()
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
        url: "http://qt-project.org/"

        Binding {
            target: webContentsView.children[0]
            property: 'anchors.fill'
            value: webContentsView
            when: webContentsView.children.length > 0
        }

        onUrlChanged: addressBar.text = url
    }

    Text {
        id: info
        anchors.top: parent.top
        anchors.right: parent.right
        horizontalAlignment: "AlignRight"
        width: 100
        height: 100

        text: viewContainer.children[0].width + "x" + viewContainer.children[0].height
    }
}
