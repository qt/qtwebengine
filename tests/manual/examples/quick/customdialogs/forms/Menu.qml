// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

MenuForm {
    property QtObject request
    signal closeForm()

    followLink.onClicked: closeForm()
    back.onClicked: closeForm()
    forward.onClicked: closeForm()
    reload.onClicked: closeForm()
    copyLinkUrl.onClicked: closeForm()
    saveLink.onClicked: closeForm()
    close.onClicked: closeForm()

    Component.onCompleted: {
        back.btnEnable = false;
        forward.btnEnable = false;
    }
}
