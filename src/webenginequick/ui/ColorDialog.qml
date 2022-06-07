// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick.Dialogs

ColorDialog {
    id: colorDialog
    objectName: "colorDialog"

    signal selectedColor(var color)

    onAccepted : selectedColor(selectedColor)
}
