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
import QtQuick.Layouts 1.14
import QtQuick.Pdf 5.15
import Qt.labs.platform 1.1 as P

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

    P.FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: doc.source = file
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
            Image {
                id: image
                scale: imageScale
                anchors.centerIn: parent
                sourceSize.width: doc.pagePointSize(index).width * oversamplingSB.value
                height: 100
                fillMode: Image.PreserveAspectFit
                objectName: "PDF page " + index
                source: doc.source
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
