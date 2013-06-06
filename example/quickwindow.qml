import QtQuick 2.0
import QtWebEngine 1.0

Item {
    id: browserWindow
    height: 480
    width: 320

    Rectangle {
        id: navigationBar
        color: "grey"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 26

        Rectangle {
            id: backButton
            color: "red"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: height

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    webContentsView.goBack()
                }
            }
        }
        Rectangle {
            id: forwardButton
            color: "green"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: backButton.right
            width: height

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    webContentsView.goForward()
                }
            }
        }
        Rectangle {
            id: reloadButton
            color: "blue"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: forwardButton.right
            width: height

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    webContentsView.reload()
                }
            }
        }
        TextInput {
            id: addressBar
            focus: true
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: reloadButton.right
            anchors.right: parent.right

            cursorVisible: true
            persistentSelection: true
            selectByMouse: true

            onAccepted: {
                webContentsView.url = text
            }
        }
    }

    WebContentsView {
        id: webContentsView
        focus: true
        anchors.top: navigationBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

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
