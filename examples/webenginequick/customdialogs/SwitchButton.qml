// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    width: parent.width
    height: 40
    property alias checked: switcher.checked
    RowLayout {
        anchors.centerIn: parent
        Text {
            text: qsTr("Use default dialogs")
            font.pointSize: 12
        }
        Switch {
            id: switcher
            checked: true
        }
    }
}
