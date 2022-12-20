// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import "../../TestParams"

QtObject {
    signal folderSelected(var folder)
    signal rejected()

    function open() {
        FilePickerParams.directoryPickerOpened = true;
        if (FilePickerParams.selectFiles)
            folderSelected(FilePickerParams.selectedFilesUrl);
        else
            rejected();
    }
}
