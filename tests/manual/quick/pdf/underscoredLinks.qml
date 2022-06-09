// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Pdf
import QtQuick.Shapes

ApplicationWindow {
    id: root
    width: 800
    height: 940
    color: "darkgrey"
    title: doc.source
    visible: true

    property PdfDocument doc: PdfDocument { source: "test.pdf" }

    Component.onCompleted: {
        if (Application.arguments.length > 2)
            doc.source = Application.arguments[Application.arguments.length - 1]
    }
    FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: doc.source = selectedFile
    }
    ScrollView {
        anchors.fill: parent
        contentWidth: paper.width
        contentHeight: paper.height

        Rectangle {
            id: paper
            width: image.width
            height: image.height
            PdfPageImage {
                id: image
                document: doc

                property real zoomFactor: Math.sqrt(2)

                Shortcut {
                    sequence: StandardKey.MoveToNextPage
                    enabled: image.currentFrame < image.frameCount - 1
                    onActivated: image.currentFrame++
                }
                Shortcut {
                    sequence: StandardKey.MoveToPreviousPage
                    enabled: image.currentFrame > 0
                    onActivated: image.currentFrame--
                }
                Shortcut {
                    sequence: StandardKey.ZoomIn
                    enabled: image.sourceSize.width < 5000
                    onActivated: {
                        image.sourceSize.width = image.implicitWidth * image.zoomFactor
                        image.sourceSize.height = image.implicitHeight * image.zoomFactor
                    }
                }
                Shortcut {
                    sequence: StandardKey.ZoomOut
                    enabled: image.width > 50
                    onActivated: {
                        image.sourceSize.width = image.implicitWidth / image.zoomFactor
                        image.sourceSize.height = image.implicitHeight / image.zoomFactor
                    }
                }
                Shortcut {
                    sequence: "Ctrl+0"
                    onActivated: image.sourceSize = undefined
                }
                Shortcut {
                    sequence: StandardKey.Open
                    onActivated: fileDialog.open()
                }
                Shortcut {
                    sequence: StandardKey.Quit
                    onActivated: Qt.quit()
                }
            }

            Menu {
                id: linkContextMenu
                property var currentLink
                MenuItem {
                    text: "Go"
                    onTriggered: {
                        if (linkContextMenu.currentLink.page >= 0)
                            image.currentFrame = linkContextMenu.currentLink.page
                        else
                            Qt.openUrlExternally(linkContextMenu.currentLink.url)
                    }
                }
                MenuItem {
                    text: "Copy"
                    onTriggered: linkContextMenu.currentLink.copyToClipboard()
                }
            }

            Repeater {
                model: PdfLinkModel {
                    id: linkModel
                    document: doc
                    page: image.currentFrame
                }
                delegate: PdfLinkDelegate {
                    x: rectangle.x
                    y: rectangle.y
                    width: rectangle.width
                    height: rectangle.height
                    onTapped:
                        (link) => {
                            if (link.page >= 0)
                                image.currentFrame = link.page
                            else
                                Qt.openUrlExternally(url)
                        }
                    onContextMenuRequested:
                        (link) => {
                            linkContextMenu.currentLink = link
                            linkContextMenu.x = x
                            linkContextMenu.y = y
                            linkContextMenu.open()
                        }
                    Shape {
                        anchors.fill: parent
                        ShapePath {
                            strokeWidth: 1
                            strokeColor: palette.link
                            strokeStyle: ShapePath.DashLine
                            dashPattern: [ 1, 4 ]
                            startX: 0; startY: height
                            PathLine { x: width; y: height }
                        }
                    }
                }
            }
        }
    }
    Label {
        anchors { bottom: parent.bottom; right: parent.right; margins: 6 }
        text: "page " + (image.currentFrame + 1) + " of " + doc.pageCount
    }
}
