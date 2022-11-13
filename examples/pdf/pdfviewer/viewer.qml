// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Pdf

ApplicationWindow {
    id: root
    width: 800
    height: 1024
    color: "lightgrey"
    title: document.title
    visible: true
    required property url source // for main.cpp
    property real scaleStep: Math.sqrt(2)

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: 6
            ToolButton {
                action: Action {
                    shortcut: StandardKey.Open
                    icon.source: "qrc:/pdfviewer/resources/document-open.svg"
                    onTriggered: fileDialog.open()
                }
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.ZoomIn
                    enabled: view.sourceSize.width < 10000
                    icon.source: "qrc:/pdfviewer/resources/zoom-in.svg"
                    onTriggered: view.renderScale *= root.scaleStep
                }
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.ZoomOut
                    enabled: view.sourceSize.width > 50
                    icon.source: "qrc:/pdfviewer/resources/zoom-out.svg"
                    onTriggered: view.renderScale /= root.scaleStep
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "qrc:/pdfviewer/resources/zoom-fit-width.svg"
                    onTriggered: view.scaleToWidth(root.contentItem.width, root.contentItem.height)
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "qrc:/pdfviewer/resources/zoom-fit-best.svg"
                    onTriggered: view.scaleToPage(root.contentItem.width, root.contentItem.height)
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+0"
                    icon.source: "qrc:/pdfviewer/resources/zoom-original.svg"
                    onTriggered: view.resetScale()
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+L"
                    icon.source: "qrc:/pdfviewer/resources/rotate-left.svg"
                    onTriggered: view.pageRotation -= 90
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+R"
                    icon.source: "qrc:/pdfviewer/resources/rotate-right.svg"
                    onTriggered: view.pageRotation += 90
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "qrc:/pdfviewer/resources/go-previous-view-page.svg"
                    enabled: view.backEnabled
                    onTriggered: view.back()
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
                value: view.currentPage + 1
                onValueModified: view.goToPage(value - 1)
                Shortcut {
                    sequence: StandardKey.MoveToPreviousPage
                    onActivated: view.goToPage(currentPageSB.value - 2)
                }
                Shortcut {
                    sequence: StandardKey.MoveToNextPage
                    onActivated: view.goToPage(currentPageSB.value)
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "qrc:/pdfviewer/resources/go-next-view-page.svg"
                    enabled: view.forwardEnabled
                    onTriggered: view.forward()
                }
                ToolTip.visible: enabled && hovered
                ToolTip.delay: 2000
                ToolTip.text: "go forward"
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.SelectAll
                    icon.source: "qrc:/pdfviewer/resources/edit-select-all.svg"
                    onTriggered: view.selectAll()
                }
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.Copy
                    icon.source: "qrc:/pdfviewer/resources/edit-copy.svg"
                    enabled: view.selectedText !== ""
                    onTriggered: view.copySelectionToClipboard()
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

    FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: document.source = selectedFile
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
        onAccepted: document.password = passwordField.text
    }

    Dialog {
        id: errorDialog
        title: "Error loading " + document.source
        standardButtons: Dialog.Close
        modal: true
        closePolicy: Popup.CloseOnEscape
        anchors.centerIn: parent
        width: 300
        visible: document.status === PdfDocument.Error

        contentItem: Label {
            id: errorField
            text: document.error
        }
    }

    PdfScrollablePageView {
        id: view
        anchors.fill: parent
        anchors.leftMargin: searchDrawer.position * searchDrawer.width
        document: PdfDocument {
            id: document
            source: Qt.resolvedUrl(root.source)
            onPasswordRequired: passwordDialog.open()
        }
        searchString: searchField.text
    }

    Drawer {
        id: searchDrawer
        edge: Qt.LeftEdge
//        modal: false
//        dim: false // commented out as workaround for QTBUG-83859
        width: 300
        y: root.header.height
        height: view.height
        clip: true
        ListView {
            id: searchResultsList
            anchors.fill: parent
            anchors.margins: 2
            model: view.searchModel
            currentIndex: view.searchModel.currentResult
            ScrollBar.vertical: ScrollBar { }
            delegate: ItemDelegate {
                id: resultDelegate
                required property int index
                required property int page
                required property string contextBefore
                required property string contextAfter
                width: parent ? parent.width : 0
                RowLayout {
                    anchors.fill: parent
                    spacing: 0
                    Label {
                        text: "Page " + (resultDelegate.page + 1) + ": "
                    }
                    Label {
                        text: resultDelegate.contextBefore
                        elide: Text.ElideLeft
                        horizontalAlignment: Text.AlignRight
                        Layout.fillWidth: true
                        Layout.preferredWidth: parent.width / 2
                    }
                    Label {
                        font.bold: true
                        text: view.searchString
                        width: implicitWidth
                    }
                    Label {
                        text: resultDelegate.contextAfter
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Layout.preferredWidth: parent.width / 2
                    }
                }
                highlighted: ListView.isCurrentItem
                onClicked: view.searchModel.currentResult = resultDelegate.index
            }
        }
    }

    footer: ToolBar {
        height: footerRow.implicitHeight
        RowLayout {
            id: footerRow
            anchors.fill: parent
            ToolButton {
                action: Action {
                    icon.source: "qrc:/pdfviewer/resources/go-up-search.svg"
                    shortcut: StandardKey.FindPrevious
                    onTriggered: view.searchBack()
                }
                ToolTip.visible: enabled && hovered
                ToolTip.delay: 2000
                ToolTip.text: "find previous"
            }
            TextField {
                id: searchField
                placeholderText: "search"
                Layout.minimumWidth: 150
                Layout.maximumWidth: 300
                Layout.fillWidth: true
                onAccepted: searchDrawer.open()
                Image {
                    visible: searchField.text !== ""
                    source: "qrc:/pdfviewer/resources/edit-clear.svg"
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
                    icon.source: "qrc:/pdfviewer/resources/go-down-search.svg"
                    shortcut: StandardKey.FindNext
                    onTriggered: view.searchForward()
                }
                ToolTip.visible: enabled && hovered
                ToolTip.delay: 2000
                ToolTip.text: "find next"
            }
            Label {
                Layout.fillWidth: true
                property size implicitPointSize: document.pagePointSize(view.currentPage)
                text: "page " + (view.currentPage + 1) + " of " + document.pageCount +
                      " scale " + view.renderScale.toFixed(2) +
                      " original " + implicitPointSize.width.toFixed(1) + "x" + implicitPointSize.height.toFixed(1) + "pts"
                visible: document.status === PdfDocument.Ready
            }
        }
    }
}
