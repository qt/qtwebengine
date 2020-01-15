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
import QtQuick 2.14
import QtQuick.Controls 2.14
import Qt.labs.platform 1.1 as Platform
import QtQuick.Pdf 5.15
import QtQuick.Shapes 1.14
import QtQuick.Window 2.14

Window {
    width: 800
    height: 940
    color: "lightgrey"
    title: doc.source
    visible: true

    PdfDocument {
        id: doc
        source: "test.pdf"
    }

    Platform.FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: doc.source = file
    }

    PdfSelection {
        id: selection
        document: doc
        page: image.currentFrame
        fromPoint: dragHandler.centroid.pressPosition
        toPoint: dragHandler.centroid.position
        hold: !dragHandler.active
    }

    Column {
        id: column
        anchors.fill: parent
        anchors.margins: 6
        spacing: 6
        Text { text: "title: " + doc.title; visible: doc.title !== "" }
        Text { text: "author: " + doc.author; visible: doc.author !== "" }
        Text { text: "subject: " + doc.subject; visible: doc.subject !== "" }
        Text { text: "keywords: " + doc.keywords; visible: doc.keywords !== "" }
        Text { text: "producer: " + doc.producer; visible: doc.producer !== "" }
        Text { text: "creator: " + doc.creator; visible: doc.creator !== "" }
        Text { text: "creationDate: " + doc.creationDate; visible: doc.creationDate !== "" }
        Text { text: "modificationDate: " + doc.modificationDate; visible: doc.modificationDate !== "" }
        Text { text: "implicit size: " + image.implicitWidth + "x" + image.implicitHeight }
        Text { text: "source size: " + image.sourceSize.width + "x" + image.sourceSize.height }
        Text { text: "painted size: " + image.paintedWidth + "x" + image.paintedHeight }

        Flickable {
            width: column.width
            height: width
            contentWidth: paper.width
            contentHeight: paper.height
            z: -1
            Rectangle {
                id: paper
                width: image.width
                height: image.height
                Image {
                    id: image
                    source: doc.status === PdfDocument.Ready ? doc.source : ""

                    property real zoomFactor: Math.sqrt(2)

                    DragHandler {
                        id: dragHandler
                        target: null
                    }

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

                Shape {
                    anchors.fill: parent
                    opacity: 0.25
                    ShapePath {
                        fillColor: "cyan"
                        PathMultiline {
                            id: selectionBoundaries
                            paths: selection.geometry
                        }
                    }
                }

                Repeater {
                    model: PdfLinkModel {
                        id: linkModel
                        document: doc
                        page: image.currentFrame
                    }
                    delegate: Rectangle {
                        color: "transparent"
                        border.color: "lightgrey"
                        x: rect.x
                        y: rect.y
                        width: rect.width
                        height: rect.height
//                        HoverHandler { cursorShape: Qt.PointingHandCursor } // 5.15 onward (QTBUG-68073)
                        TapHandler {
                            onTapped: {
                                if (page >= 0)
                                    image.currentFrame = page
                                else
                                    Qt.openUrlExternally(url)
                            }
                        }
                    }
                }
            }
        }
    }
    Text {
        anchors.bottom: parent.bottom
        text: "page " + (image.currentFrame + 1) + " of " + doc.pageCount
    }
}
