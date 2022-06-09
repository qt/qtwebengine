// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQml
import QtTest
import "../../TestParams"

QtObject {
    property string text
    property string title
    signal accepted()
    signal rejected()

    function open() {
        JSDialogParams.dialogTitle = title;
        JSDialogParams.dialogMessage = text;
        JSDialogParams.dialogCount++;
        if (JSDialogParams.shouldAcceptDialog)
            accepted();
        else
            rejected();
    }
}

