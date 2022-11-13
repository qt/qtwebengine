// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQml.Models
import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    signal closeRequested(int index)
    signal drawerRequested

    property int freezeDelay
    property int discardDelay

    property int currentIndex
    property var currentTab: model.children[currentIndex]
    property alias count: model.count

    color: "white"

    ObjectModel {
        id: model
    }

    Component {
        id: factory
        WebTab {
            readonly property int index : ObjectModel.index
            anchors.fill: parent
            visible: index == root.currentIndex
            freezeDelay: root.freezeDelay
            discardDelay: root.discardDelay
            onCloseRequested: root.closeRequested(index)
            onDrawerRequested: root.drawerRequested()
        }
    }

    function createNewTab(properties) {
        const tab = factory.createObject(root, properties)
        model.append(tab)
        return tab
    }

    function closeTab(index) {
        const tab = model.get(index)
        model.remove(index)
        tab.destroy()
    }
}
