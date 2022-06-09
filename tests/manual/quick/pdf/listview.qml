// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Pdf

Window {
    width: 600
    height: 440
    color: "lightgrey"
    title: doc.source
    visible: true

    PdfDocument {
        id: doc
        source: "test.pdf"
    }

    ListView {
        id: listView
        anchors.fill: parent
        model: doc.pageCount
        spacing: 6
        delegate: Column {
            Rectangle {
                id: paper
                width: image.width
                height: image.height
                Image {
                    id: image
                    objectName: "PDF page " + index
                    source: doc.source
                    currentFrame: index
                    asynchronous: true
                }
            }
            Text {
                text: "Page " + doc.pageLabel(image.currentFrame)
            }
        }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: "page " + Math.max(1, (listView.indexAt(0, listView.contentY) + 1)) + " of " + doc.pageCount
    }
}
