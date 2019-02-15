/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.1
import QtQuick.Controls 1.0
import QtWebEngine 1.1
import QtQuick.Layouts 1.0

Rectangle {
    property var requestedFeature;
    property url securityOrigin;
    property WebEngineView view;

    id: permissionBar
    visible: false
    height: acceptButton.height + 4


    function textForFeature(feature) {
        switch (feature) {
            case WebEngineView.Geolocation:              return 'Allow %1 to access your location information?'
            case WebEngineView.MediaAudioCapture:        return 'Allow %1 to access your microphone?'
            case WebEngineView.MediaVideoCapture:        return 'Allow %1 to access your webcam?'
            case WebEngineView.MediaAudioVideoCapture:   return 'Allow %1 to access your microphone and webcam?'
            case WebEngineView.DesktopVideoCapture:      return 'Allow %1 to capture video of your desktop?'
            case WebEngineView.DesktopAudioVideoCapture: return 'Allow %1 to capture audio and video of your desktop?'
            case WebEngineView.Notifications:            return 'Allow %1 to show notification on your desktop?'
            default: break
        }
        return 'Grant permission for %1 to unknown or unsupported feature [' + feature + ']?'
    }

    onRequestedFeatureChanged: {
        message.text = textForFeature(requestedFeature).arg(securityOrigin);
    }

    RowLayout {
        anchors {
            fill: permissionBar
            leftMargin: 5
            rightMargin: 5
        }
        Label {
            id: message
            Layout.fillWidth: true
        }

        Button {
            id: acceptButton
            text: "Accept"
            Layout.alignment: Qt.AlignRight
            onClicked: {
                view.grantFeaturePermission(securityOrigin, requestedFeature, true);
                permissionBar.visible = false;
            }
        }

        Button {
            text: "Deny"
            Layout.alignment: Qt.AlignRight
            onClicked: {
                view.grantFeaturePermission(securityOrigin, requestedFeature, false);
                permissionBar.visible = false
            }
        }
    }
}
