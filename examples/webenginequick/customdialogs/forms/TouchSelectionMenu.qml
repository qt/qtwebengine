// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

TouchSelectionMenuForm {
    property QtObject request
    signal closeForm()

    cut.onClicked: closeForm()
    copy.onClicked: closeForm()
    paste.onClicked: closeForm()
    contextMenu.onClicked: closeForm()
}
