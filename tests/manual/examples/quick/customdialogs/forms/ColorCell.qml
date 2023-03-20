// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: rectangle
    width: 50
    height: 50
    signal clicked()
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: rectangle.clicked()
    }
}
