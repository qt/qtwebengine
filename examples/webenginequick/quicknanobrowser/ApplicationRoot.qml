// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

QtObject {
    id: root
    required property string startupUrl
    Component.onCompleted: BrowserManager.load(startupUrl)
}
