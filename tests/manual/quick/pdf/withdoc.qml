// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Pdf
import QtQuick.Shapes

Window {
    width: 800
    height: 940
    color: "lightgrey"
    title: doc.source
    visible: true

    PdfDocument {
        id: doc
        source: "test.pdf"
        onPasswordRequired: function() { passwordDialog.open() }
    }

    FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: doc.source = selectedFile
    }

    Dialog {
        id: passwordDialog
        title: "Password"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        closePolicy: Popup.CloseOnEscape
        anchors.centerIn: parent
        width: 300

        contentItem: TextField {
            id: passwordField
            placeholderText: qsTr("Please provide the password")
            echoMode: TextInput.Password
            width: parent.width
            onAccepted: passwordDialog.accept()
        }
        onOpened: function() { passwordField.forceActiveFocus() }
        onAccepted: doc.password = passwordField.text
    }

    PdfSelection {
        id: selection
        document: doc
        page: image.currentFrame
        from: dragHandler.centroid.pressPosition
        to: dragHandler.centroid.position
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
                PdfPageImage {
                    id: image
                    document: doc

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
                        HoverHandler { cursorShape: Qt.PointingHandCursor }
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
        text: "page " + (image.currentFrame + 1) + " of " + doc.pageCount + " label: " + doc.pageLabel(image.currentFrame)
    }
}
