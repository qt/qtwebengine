/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Pdf 5.15
import QtQuick.Shapes 1.15
import QtQuick.Window 2.15
import Qt.labs.platform 1.1 as Platform

ApplicationWindow {
    id: root
    width: 800
    height: 640
    color: "lightgrey"
    title: document.title
    visible: true
    property alias source: document.source // for main.cpp
    property real scaleStep: Math.sqrt(2)

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: 6
            ToolButton {
                action: Action {
                    shortcut: StandardKey.Open
                    icon.source: "resources/document-open.svg"
                    onTriggered: fileDialog.open()
                }
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.ZoomIn
                    enabled: pageView.sourceSize.width < 10000
                    icon.source: "resources/zoom-in.svg"
                    onTriggered: pageView.renderScale *= root.scaleStep
                }
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.ZoomOut
                    enabled: pageView.sourceSize.width > 50
                    icon.source: "resources/zoom-out.svg"
                    onTriggered: pageView.renderScale /= root.scaleStep
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+0"
                    icon.source: "resources/zoom-original.svg"
                    onTriggered: pageView.renderScale = 1
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+L"
                    icon.source: "resources/rotate-left.svg"
                    onTriggered: pageView.rotation -= 90
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+R"
                    icon.source: "resources/rotate-right.svg"
                    onTriggered: pageView.rotation += 90
                }
            }
            SpinBox {
                id: currentPageSB
                from: 1
                to: document.pageCount
                value: 1
                editable: true
                Shortcut {
                    sequence: StandardKey.MoveToPreviousPage
                    onActivated: currentPageSB.value--
                }
                Shortcut {
                    sequence: StandardKey.MoveToNextPage
                    onActivated: currentPageSB.value++
                }
            }
            TextField {
                id: searchField
                placeholderText: "search"
                Layout.minimumWidth: 200
                Layout.fillWidth: true
                Image {
                    visible: searchField.text !== ""
                    source: "resources/edit-clear.svg"
                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        margins: 3
                        rightMargin: 5
                    }
                    TapHandler {
                        onTapped: searchField.clear()
                    }
                }
            }
            Shortcut {
                sequence: StandardKey.Find
                onActivated: searchField.forceActiveFocus()
            }
            Shortcut {
                sequence: StandardKey.Quit
                onActivated: Qt.quit()
            }
        }
    }

    Platform.FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: document.source = file
    }

    Dialog {
        id: errorDialog
        title: "Error loading " + document.source
        standardButtons: Dialog.Ok
        modal: true
        closePolicy: Popup.CloseOnEscape
        anchors.centerIn: parent
        width: 300

        Label {
            id: errorField
            text: document.error
        }
    }

    PdfPageView {
        id: pageView
        currentPage: currentPageSB.value - 1
        document: PdfDocument {
            id: document
            onStatusChanged: if (status === PdfDocument.Error) errorDialog.open()
        }
        searchString: searchField.text
    }

    footer: Label {
        property size implicitPointSize: document.pagePointSize(pageView.currentPage)
        text: "page " + (pageView.currentPage + 1) + " of " + pageView.pageCount +
              " scale " + pageView.renderScale.toFixed(2) +
              " sourceSize " + pageView.sourceSize.width.toFixed(1) + "x" + pageView.sourceSize.height.toFixed(1) +
              " original " + implicitPointSize.width.toFixed(1) + "x" + implicitPointSize.height.toFixed(1)
        visible: pageView.pageCount > 0
    }
}
