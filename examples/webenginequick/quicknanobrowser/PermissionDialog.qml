// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtWebEngine

Dialog {
    id: root
    contentWidth: mainTextForPermissionDialog.width
    contentHeight: mainTextForPermissionDialog.height
    standardButtons: Dialog.No | Dialog.Yes
    title: "Permission Request"

    property var permission

    contentItem: Item {
        Label {
            id: mainTextForPermissionDialog
        }
    }

    onAccepted: permission.grant()
    onRejected: permission.deny()
    onVisibleChanged: {
        if (visible) {
            mainTextForPermissionDialog.text = questionForPermissionType();
            width = contentWidth + 20;
        }
    }

    function questionForPermissionType() {
        var question = "Allow " + permission.origin + " to ";

        switch (permission.permissionType) {
        case WebEnginePermission.PermissionType.Geolocation:
            question += "access your location information?";
            break;
        case WebEnginePermission.PermissionType.MediaAudioCapture:
            question += "access your microphone?";
            break;
        case WebEnginePermission.PermissionType.MediaVideoCapture:
            question += "access your webcam?";
            break;
        case WebEnginePermission.PermissionType.MediaAudioVideoCapture:
            question += "access your microphone and webcam?";
            break;
        case WebEnginePermission.PermissionType.MouseLock:
            question += "lock your mouse cursor?";
            break;
        case WebEnginePermission.PermissionType.DesktopVideoCapture:
            question += "capture video of your desktop?";
            break;
        case WebEnginePermission.PermissionType.DesktopAudioVideoCapture:
            question += "capture audio and video of your desktop?";
            break;
        case WebEnginePermission.PermissionType.Notifications:
            question += "show notification on your desktop?";
            break;
        case WebEnginePermission.PermissionType.ClipboardReadWrite:
            question += "read from and write to your clipboard?";
            break;
        case WebEnginePermission.PermissionType.LocalFontsAccess:
            question += "access the fonts stored on your machine?";
            break;
        default:
            question += "access unknown or unsupported permission type [" + permission.permissionType + "] ?";
            break;
        }

        return question;
    }
}
