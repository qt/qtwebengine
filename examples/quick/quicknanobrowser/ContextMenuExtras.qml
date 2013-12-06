import QtQuick 2.1
import QtWebEngine.UIDelegates 1.0

VisualItemModel {
    MenuItem {
        text: "An application specific entry"
        onTriggered: console.log("Application specific action triggered")
    }
    Menu {
        title: "Extras Submenu"
        MenuItem {
            text: "something"
            onTriggered: console.log("something triggered")
        }
        MenuItem {
            text: "something else"
            enabled: false
        }
    }
}

