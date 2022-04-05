/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Pdf
import Qt.labs.animation

ApplicationWindow {
    id: root
    width: 1280
    height: 1024
    color: "transparent"
    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    property string source // for main.cpp
    property real scaleStep: Math.sqrt(2)

    Component {
        id: pdfWindow

        PdfPageView {
            property alias source: document.source

            Rectangle {
                visible: parent.activeFocus
                color: "transparent"
                border.color: "cyan"
                border.width: 3
                anchors.fill: parent
                anchors.margins: -border.width
                anchors.topMargin: -toolbar.height - border.width
            }

            ToolBar {
                id: toolbar
                width: parent.width
                y: -height
                RowLayout {
                    anchors.fill: parent
                    anchors.rightMargin: 6
                    ToolButton {
                        action: Action {
                            shortcut: StandardKey.ZoomIn
                            enabled: pageView.sourceSize.width < 10000
                            icon.source: "../../../../examples/pdf/pdfviewer/resources/zoom-in.svg"
                            onTriggered: pageView.renderScale *= root.scaleStep
                        }
                    }
                    ToolButton {
                        action: Action {
                            shortcut: StandardKey.ZoomOut
                            enabled: pageView.sourceSize.width > 50
                            icon.source: "../../../../examples/pdf/pdfviewer/resources/zoom-out.svg"
                            onTriggered: pageView.renderScale /= root.scaleStep
                        }
                    }
                    ToolButton {
                        action: Action {
                            shortcut: "Ctrl+0"
                            icon.source: "../../../../examples/pdf/pdfviewer/resources/zoom-original.svg"
                            onTriggered: pageView.resetScale()
                        }
                    }
                    ToolButton {
                        action: Action {
                            icon.source: "../../../../examples/pdf/pdfviewer/resources/go-previous-view-page.svg"
                            enabled: pageView.backEnabled
                            onTriggered: pageView.back()
                        }
                        ToolTip.visible: enabled && hovered
                        ToolTip.delay: 2000
                        ToolTip.text: "go back"
                    }
                    SpinBox {
                        id: currentPageSB
                        from: 1
                        to: document.pageCount
                        editable: true
                        value: pageView.currentPage + 1
                        onValueModified: pageView.goToPage(value - 1)
                        Layout.maximumWidth: 60
                    }
                    ToolButton {
                        action: Action {
                            icon.source: "../../../../examples/pdf/pdfviewer/resources/go-next-view-page.svg"
                            enabled: pageView.forwardEnabled
                            onTriggered: pageView.forward()
                        }
                        ToolTip.visible: enabled && hovered
                        ToolTip.delay: 2000
                        ToolTip.text: "go forward"
                    }
                    Text {
                        text: document.title
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    DragHandler {
                        target: pageView
                    }
                    TapHandler {
                        onTapped: pageView.z++
                    }
                    ToolButton {
                        action: Action {
                            shortcut: StandardKey.Close
                            text: "☒"
                            onTriggered: pageView.destroy()
                        }
                    }
                }
            }

            id: pageView
            document: PdfDocument {
                id: document
                source: Qt.resolvedUrl(root.source)
                onStatusChanged: (status) => { if (status === PdfDocument.Error) errorDialog.open() }
            }

            DragHandler {
                acceptedButtons: Qt.MiddleButton
            }
            DragHandler {
                acceptedDevices: PointerDevice.TouchScreen
            }
            HoverHandler {
                onHoveredChanged: if (hovered) pageView.forceActiveFocus()
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
        }
    }

    Shortcut {
        sequence: StandardKey.MoveToPreviousPage
        onActivated: root.activeFocusItem.goToPage(root.activeFocusItem.currentPage - 1)
    }
    Shortcut {
        sequence: StandardKey.MoveToNextPage
        onActivated: root.activeFocusItem.goToPage(root.activeFocusItem.currentPage + 1)
    }
    Shortcut {
        sequence: StandardKey.Open
        onActivated: fileDialog.open()
    }
    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    function open(src) {
        var win = pdfWindow.createObject(root, { source: src, y: 50 })
    }

    Component.onCompleted: {
        if (Application.arguments.length > 2)
            root.open(Application.arguments[Application.arguments.length - 1])
    }

    FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: root.open(selectedFile)
    }
}
