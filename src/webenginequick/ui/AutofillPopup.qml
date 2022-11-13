// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Popup {
    id: root
    // Let Chromium close the popup.
    closePolicy: Popup.NoAutoClose

    property variant controller: null
    property int itemHeight: 0

    signal selected(int index)
    signal accepted()

    function setCurrentIndex(index)
    {
        listView.currentIndex = index;
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true

        model: controller.model
        currentIndex: -1

        delegate: ItemDelegate {
            width: listView.width
            height: root.itemHeight
            text: model.display
            highlighted: ListView.isCurrentItem

            onHoveredChanged: if (hovered) selected(index);
            onClicked: accepted();
        }
    }
}
