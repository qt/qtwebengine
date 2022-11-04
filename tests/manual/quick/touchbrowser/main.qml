// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick
import QtQuick.Layouts
import QtWebEngine

Item {
    function load(url) {
        webEngineView.url = url;
    }

    ColumnLayout {
        anchors.fill: parent

        AddressBar {
            id: addressBar

            Layout.fillWidth: true
            Layout.margins: 5
            height: 25

            color: "white"
            radius: 4

            progress: webEngineView && webEngineView.loadProgress
            iconUrl: webEngineView && webEngineView.icon
            pageUrl: webEngineView && webEngineView.url

            onAccepted: webEngineView.url = addressUrl
        }

        WebEngineView {
            id: webEngineView

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
