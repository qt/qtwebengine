// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls as C

C.Menu {
    id: menu
    signal done()
    objectName: "menu"

    // Use private API for now
    onAboutToHide: doneTimer.start()

    // WORKAROUND On Mac the Menu may be destroyed before the MenuItem
    // is actually triggered (see qtbase commit 08cc9b9991ae9ab51)
    Timer {
        id: doneTimer
        interval: 100
        onTriggered: menu.done()
    }
}
