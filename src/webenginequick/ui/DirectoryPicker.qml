// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick.Dialogs

FolderDialog {
    id: folderDialog
    objectName: "folderDialog"

    signal folderSelected(var folder)

    onAccepted: {
        folderSelected([selectedFolder])
    }
}
