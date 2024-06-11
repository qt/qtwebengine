// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [Minimal Example]
import QtQuick
import QtQuick.Window
import QtWebEngine

Window {
    width: 1024
    height: 750
    visible: true
    WebEngineView {
        anchors.fill: parent
        url: "https://www.qt.io"
    }
}
//! [Minimal Example]
