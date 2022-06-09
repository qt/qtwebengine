// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import "../../TestParams"

QtObject {
    property bool selectMultiple: false
    property bool selectExisting: false
    property bool selectFolder: false
    property var nameFilters: []

    signal filesSelected(var fileList)
    signal rejected()

    function open() {
        FilePickerParams.filePickerOpened = true;
        FilePickerParams.nameFilters = nameFilters;
        if (FilePickerParams.selectFiles)
            filesSelected(FilePickerParams.selectedFilesUrl);
        else
            rejected();
    }
}
