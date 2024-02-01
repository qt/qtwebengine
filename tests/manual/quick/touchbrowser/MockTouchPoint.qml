// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
import QtQuick

Item {
    id: mockTouchPoint

    property bool pressed: false
    property int pointId: 0

    Image {
        source: "qrc:/touchpoint.png"
        x: -(width / 2)
        y: -(height / 2)
        opacity: mockTouchPoint.pressed ? 0.6 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        Text {
            text: mockTouchPoint.pointId
            anchors.centerIn: parent
        }
    }
}
