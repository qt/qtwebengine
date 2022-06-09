// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Pdf
import Qt.labs.animation

ApplicationWindow {
    id: root
    width: 800
    height: 1024
    color: "lightgrey"
    title: document.title
    visible: true
    property string source // for main.cpp
    property real scaleStep: Math.sqrt(2)

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: 6
            ToolButton {
                action: Action {
                    shortcut: StandardKey.Open
                    icon.source: "../../../../examples/pdf/pdfviewer/resources/document-open.svg"
                    onTriggered: fileDialog.open()
                }
            }
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
                    icon.source: "../../../../examples/pdf/pdfviewer/resources/zoom-fit-width.svg"
                    onTriggered: pageView.scaleToWidth(root.contentItem.width, root.contentItem.height)
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "../../../../examples/pdf/pdfviewer/resources/zoom-fit-best.svg"
                    onTriggered: pageView.scaleToPage(root.contentItem.width, root.contentItem.height)
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
                    shortcut: "Ctrl+L"
                    icon.source: "../../../../examples/pdf/pdfviewer/resources/rotate-left.svg"
                    onTriggered: pageView.rotation -= 90
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+R"
                    icon.source: "../../../../examples/pdf/pdfviewer/resources/rotate-right.svg"
                    onTriggered: pageView.rotation += 90
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
                from: 0
                to: document.pageCount
                editable: true
                value: pageView.currentPage
                onValueModified: pageView.goToPage(value)

                textFromValue: function(value) { return document.pageLabel(value) }
                valueFromText: function(text) {
                    for (var i = 0; i < document.pageCount; ++i) {
                        if (document.pageLabel(i).toLowerCase().indexOf(text.toLowerCase()) === 0)
                            return i
                    }
                    return spinBox.value
                }

                Shortcut {
                    sequence: StandardKey.MoveToPreviousPage
                    onActivated: pageView.goToPage(currentPageSB.value - 1)
                }
                Shortcut {
                    sequence: StandardKey.MoveToNextPage
                    onActivated: pageView.goToPage(currentPageSB.value + 1)
                }
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
            ToolButton {
                action: Action {
                    shortcut: StandardKey.Copy
                    icon.source: "../../../../examples/pdf/pdfviewer/resources/edit-copy.svg"
                    enabled: pageView.selectedText !== ""
                    onTriggered: pageView.copySelectionToClipboard()
                }
            }
            Shortcut {
                sequence: StandardKey.Quit
                onActivated: Qt.quit()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: document.source = selectedFile
    }

    Component.onCompleted: {
        if (Application.arguments.length > 2)
            document.source = Application.arguments[Application.arguments.length - 1]
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
        x: searchDrawer.position * searchDrawer.width // TODO binding gets broken during centering
        document: PdfDocument {
            id: document
            source: Qt.resolvedUrl(root.source)
            onStatusChanged: (status) => { if (status === PdfDocument.Error) errorDialog.open() }
        }
        searchString: searchField.text
        onRenderScaleChanged: pageView.returnToBounds()

        DragHandler {
            acceptedButtons: Qt.MiddleButton
            onActiveChanged: if (!active) pageView.returnToBounds()
        }
        DragHandler {
            acceptedDevices: PointerDevice.TouchScreen
            onActiveChanged: if (!active) pageView.returnToBounds()
        }

        BoundaryRule on x {
            id: brx
            minimumOvershoot: 100
            maximumOvershoot: 100
            minimum: Math.min(0, root.width - pageView.width)
            maximum: 0
        }

        BoundaryRule on y {
            id: bry
            minimumOvershoot: 100
            maximumOvershoot: 100
            minimum: Math.min(0, root.height - pageView.height)
            maximum: 0
        }

        function returnToBounds() {
            bry.returnToBounds()
            brx.returnToBounds()
        }
    }

    WheelHandler {
        target: pageView
        property: "x"
        orientation: Qt.Horizontal
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        acceptedModifiers: Qt.NoModifier
        onActiveChanged: if (!active) brx.returnToBounds()
    }
    WheelHandler {
        target: pageView
        property: "y"
        orientation: Qt.Vertical
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        acceptedModifiers: Qt.NoModifier
        onActiveChanged: if (!active) bry.returnToBounds()
    }

    Drawer {
        id: searchDrawer
        edge: Qt.LeftEdge
        modal: false
        width: searchLayout.implicitWidth
        y: root.header.height
        height: root.contentItem.height
        dim: false
        Shortcut {
            sequence: StandardKey.Find
            onActivated: {
                searchDrawer.open()
                searchField.forceActiveFocus()
            }
        }
        ColumnLayout {
            id: searchLayout
            anchors.fill: parent
            anchors.margins: 2
            RowLayout {
                ToolButton {
                    action: Action {
                        icon.source: "../../../../examples/pdf/pdfviewer/resources/go-up-search.svg"
                        shortcut: StandardKey.FindPrevious
                        onTriggered: pageView.searchBack()
                    }
                    ToolTip.visible: enabled && hovered
                    ToolTip.delay: 2000
                    ToolTip.text: "find previous"
                }
                TextField {
                    id: searchField
                    placeholderText: "search"
                    Layout.minimumWidth: 200
                    Layout.fillWidth: true
                    Image {
                        visible: searchField.text !== ""
                        source: "../../../../examples/pdf/pdfviewer/resources/edit-clear.svg"
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
                ToolButton {
                    action: Action {
                        icon.source: "../../../../examples/pdf/pdfviewer/resources/go-down-search.svg"
                        shortcut: StandardKey.FindNext
                        onTriggered: pageView.searchForward()
                    }
                    ToolTip.visible: enabled && hovered
                    ToolTip.delay: 2000
                    ToolTip.text: "find next"
                }
            }
            ListView {
                id: searchResultsList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: pageView.searchModel
                ScrollBar.vertical: ScrollBar { }
                delegate: ItemDelegate {
                    width: parent ? parent.width : 0
                    text: "page " + document.pageLabel(page) + ": " + contextBefore + pageView.searchString + contextAfter
                    highlighted: ListView.isCurrentItem
                    onClicked: {
                        searchResultsList.currentIndex = index
                        pageView.goToLocation(page, location, 0)
                        pageView.searchModel.currentResult = indexOnPage
                    }
                }
            }
        }
    }

    footer: Label {
        property size implicitPointSize: document.pagePointSize(pageView.currentPage)
        text: "page " + (pageView.currentPage + 1) + " of " + document.pageCount +
              " scale " + pageView.renderScale.toFixed(2) +
              " original " + implicitPointSize.width.toFixed(1) + "x" + implicitPointSize.height.toFixed(1) + "pts"
        visible: document.status === PdfDocument.Ready
    }
}
