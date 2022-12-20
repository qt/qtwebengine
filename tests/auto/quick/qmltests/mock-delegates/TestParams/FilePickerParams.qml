// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

pragma Singleton
import QtQuick

QtObject {
    property var selectedFilesUrl: [];
    property bool selectFiles: false;
    property bool filePickerOpened: false;
    property bool directoryPickerOpened: false;
    property var nameFilters: [];
}
