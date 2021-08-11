import QtQuick
import QtQuick.Window
import QtWebEngine

Window {
    visible: true
    width: 640
    height: 480

    property var url: view && view.url

    WebEngineView {
        id: view
        anchors.fill: parent
    }
}
