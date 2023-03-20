// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: root
    height: 30
    property string text: "Filename"
    property bool selected: false
    signal clicked()

    RowLayout {
        id: fileRow
        width: 100

        Item {
            id: item5
            width: 10
            height: 10
        }

        Rectangle {
            id: rectangle2
            width: 10
            height: 10
            color: selected ? "#80c342" : "#25a6e2"
        }

        Text {
            id: filename
            text: root.text
            font.pointSize: 12
        }
    }

    MouseArea {
        id: mouseArea
        width: 200
        height: 30
        onClicked: root.clicked()
    }
}
