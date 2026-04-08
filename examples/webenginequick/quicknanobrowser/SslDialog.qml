// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Dialog {
    id: root
    contentWidth: Math.max(mainTextForSSLDialog.width, detailedTextForSSLDialog.width)
    contentHeight: mainTextForSSLDialog.height + detailedTextForSSLDialog.height
    property var certErrors: []
    // fixme: icon!
    // icon: StandardIcon.Warning
    standardButtons: Dialog.No | Dialog.Yes
    title: "Server's certificate not trusted"
    contentItem: Item {
        Label {
            id: mainTextForSSLDialog
            text: "Do you wish to continue?"
        }
        Text {
            id: detailedTextForSSLDialog
            anchors.top: mainTextForSSLDialog.bottom
            text: "If you wish so, you may continue with an unverified certificate.\n" +
                  "Accepting an unverified certificate means\n" +
                  "you may not be connected with the host you tried to connect to.\n" +
                  "Do you wish to override the security check and continue?"
        }
    }

    onAccepted: {
        certErrors.shift().acceptCertificate();
        presentError();
    }
    onRejected: reject()

    function reject() {
        certErrors.shift().rejectCertificate();
        presentError();
    }
    function enqueue(error) {
        certErrors.push(error);
        presentError();
    }
    function presentError() {
        visible = certErrors.length > 0;
    }
}
