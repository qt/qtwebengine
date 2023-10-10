// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

ToolButton {
    id: root
    font.bold: true
    font.pointSize: 12
    ToolTip.delay: 1000
    ToolTip.visible: hovered
    implicitWidth: 32
    implicitHeight: 32
}
