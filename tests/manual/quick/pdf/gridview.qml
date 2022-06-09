// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Pdf

Window {
    width: 320
    height: 440
    color: "lightgrey"
    title: doc.source
    visible: true

    property real cellSize: 150

    PdfDocument {
        id: doc
        source: "test.pdf"
    }

    GridView {
        id: view
        anchors.fill: parent
        anchors.margins: 10
        model: doc.pageModel
        cellWidth: cellSize
        cellHeight: cellSize
        delegate: Item {
            required property int index
            required property string label
            required property size pointSize
            width: view.cellWidth
            height: view.cellHeight
            Rectangle {
                id: paper
                width: image.width
                height: image.height
                x: (parent.width - width) / 2
                y: (parent.height - height - pageNumber.height) / 2
                PdfPageImage {
                    id: image
                    document: doc
                    currentFrame: index
                    asynchronous: true
                    fillMode: Image.PreserveAspectFit
                    property bool landscape: pointSize.width > pointSize.height
                    width: landscape ? Math.min(view.cellWidth, pointSize.width)
                                     : height * pointSize.width / pointSize.height
                    height: landscape ? width * pointSize.height / pointSize.width
                                      : Math.min(view.cellHeight - pageNumber.height, pointSize.height)
                    sourceSize.width: width
                    sourceSize.height: height
                }
            }
            Text {
                id: pageNumber
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Page " + label
            }
        }
    }
}
