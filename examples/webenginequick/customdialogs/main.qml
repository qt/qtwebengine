// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: mainWindow
    width: 800
    height: 610
    visible: true

    StackView {
        id: stackView
        anchors.fill: parent
        focus: true
        initialItem: Item {
            id: main
            width: mainWindow.width
            height: mainWindow.height
            ColumnLayout {
                anchors.fill: parent
                SwitchButton {
                    id: switcher
                    Layout.fillWidth: true
                }
                WebView {
                    id: webView
                    useDefaultDialogs: switcher.checked
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }

        function closeForm()
        {
            pop(main);
            // reset url in case of proxy error
            webView.url = "qrc:/index.html"
        }

        function openForm(form)
        {
            push(form.item, form.properties);
            currentItem.closeForm.connect(closeForm);
        }

    }

    Component.onCompleted: {
        webView.openForm.connect(stackView.openForm);
    }
}
