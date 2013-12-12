import QtQuick 2.0
import QtQuick.Window 2.1
import QtWebEngine 1.0
import QtWebEngine.experimental 1.0

Window {
    id: launcherWindow
    width: 720
    height: 1280
    visible: true
   
    Item {
        anchors.fill: parent
        MouseArea {
            width: parent.width
            height: 48
            property int barf: 0
            onClicked: {
                barf = 1 - barf;
                if (barf) {
                    web_view_11.parent = null
                    web_view_22.parent = launcherWindow.contentItem
                } else {
                    web_view_22.parent = null
                    web_view_11.parent = launcherWindow.contentItem
                }
            }
        }

        WebEngineView {
            id: web_view_11
            width: 720
            height: 100
            y: 48
            url: 'http://www.google.com/'
        }
       
        property QtObject foo: WebEngineView {
            id: web_view_22
            width: 720
            height: 100
            y: 148
            url: 'http://www.yahoo.com/'
        }
    }
}

