// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtWebEngine

AuthenticationForm {
    property QtObject request
    signal closeForm()

    cancelButton.onClicked: {
        request.dialogReject();
        closeForm();
    }

    loginButton.onClicked: {
        request.dialogReject();
        closeForm();
    }

    Component.onCompleted: {
        switch (request.type) {
        case  AuthenticationDialogRequest.AuthenticationTypeHTTP:
            console.log("HTTP Authentication Required. Host says: " + request.realm);
            break;
        case  AuthenticationDialogRequest.AuthenticationTypeProxy:
            console.log("Proxy Authentication Required for: " + request.proxyHost);
            break;
        }
    }
}
