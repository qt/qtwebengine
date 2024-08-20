// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQml
import "../../TestParams"

QtObject {
    id: menu
    property string linkText: ""
    property var mediaType: null
    property string selectedText: ""

    signal done()

    function open() {
        MenuParams.isMenuOpened = true;
    }
}
