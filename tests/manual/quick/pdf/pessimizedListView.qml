// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Pdf

ApplicationWindow {
    width: 900
    height: 1000
    color: "lightgrey"
    title: doc.source + " scale " + imageScale.toFixed(2)
    visible: true
    property real imageScale: 1

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: 6
            ToolButton {
                action: Action {
                    text: "🗁"
                    shortcut: StandardKey.Open
                    onTriggered: fileDialog.open()
                }
            }
            ToolButton {
                action: Action {
                    text: "⊕"
                    shortcut: StandardKey.ZoomIn
                    onTriggered: imageScale *= Math.sqrt(2)
                }
            }
            ToolButton {
                action: Action {
                    text: "⊖"
                    shortcut: StandardKey.ZoomOut
                    onTriggered: imageScale /= Math.sqrt(2)
                }
            }
            ToolButton {
                action: Action {
                    text: "1x"
                    shortcut: "Ctrl+0"
                    onTriggered: imageScale = 1
                }
            }

            Label {
                text: "Pixels/point:"
            }
            SpinBox {
                id: oversamplingSB
                from: 1; to: 8; value: 4
            }

            Label {
                text: "cacheBuffer:"
            }
            SpinBox {
                id: cacheBufferSB
                from: 0; to: 1000; stepSize: 50; value: 100
            }

            CheckBox {
                id: asyncCB
                text: "async"
            }

            CheckBox {
                id: cacheCB
                text: "cache"
            }
        }
    }

    PdfDocument {
        id: doc
        source: "test.pdf"
    }

    FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: doc.source = selectedFile
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 10
        model: doc.pageCount
        spacing: 6
        cacheBuffer: cacheBufferSB.value
        ScrollBar.vertical: ScrollBar { }
        delegate: Rectangle {
            id: paper
            width: Math.max(60, image.width) * imageScale
            height: 100 * imageScale
            BusyIndicator {
                anchors.centerIn: parent
                running: image.status === Image.Loading
            }
            PdfPageImage {
                id: image
                document: doc
                scale: imageScale
                anchors.centerIn: parent
                sourceSize.width: doc.pagePointSize(index).width * oversamplingSB.value
                height: 100
                fillMode: Image.PreserveAspectFit
                objectName: "PDF page " + index
                currentFrame: index
                asynchronous: asyncCB.checked
                cache: cacheCB.checked
            }
        }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: "page " + Math.max(1, (listView.indexAt(0, listView.contentY) + 1)) + " of " + doc.pageCount
    }
}
