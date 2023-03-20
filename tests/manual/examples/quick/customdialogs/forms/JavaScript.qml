// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWebEngine

JavaScriptForm {
    property QtObject request
    signal closeForm()

    cancelButton.onClicked: {
        request.dialogReject();
        closeForm();
    }

    okButton.onClicked: {
        request.dialogAccept(prompt.text);
        closeForm();
    }

    Component.onCompleted: {
        switch (request.type) {
        case JavaScriptDialogRequest.DialogTypeAlert:
            cancelButton.visible = false;
            title = qsTr("Alert");
            message = request.message;
            prompt.text = "";
            prompt.visible = false;
            break;
        case JavaScriptDialogRequest.DialogTypeConfirm:
            title = qsTr("Confirm");
            message = request.message;
            prompt.text = "";
            prompt.visible = false;
            break;
        case JavaScriptDialogRequest.DialogTypePrompt:
            title = qsTr("Prompt");
            message = request.message;
            prompt.text = request.defaultText;
            prompt.visible = true;
            break;
        }
    }
}
