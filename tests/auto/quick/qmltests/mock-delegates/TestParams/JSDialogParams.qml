// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
pragma Singleton
import QtQml

QtObject {
    property string dialogMessage: "";
    property string dialogTitle: "";
    property bool shouldAcceptDialog: true;
    property string inputForPrompt;
    property int dialogCount: 0
}
