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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Pdf 5.15
import QtQuick.Shapes 1.15
import QtQuick.Window 2.15

Item {
    // public API
    // TODO 5.15: required property
    property var document: undefined
    property real renderScale: 1
    property string searchString
    property string selectedText
    property alias currentPage: listView.currentIndex
    function copySelectionToClipboard() {
        if (listView.currentItem !== null)
            listView.currentItem.selection.copyToClipboard()
    }
    property alias backEnabled: navigationStack.backAvailable
    property alias forwardEnabled: navigationStack.forwardAvailable
    function back() { navigationStack.back() }
    function forward() { navigationStack.forward() }
    signal currentPageReallyChanged(page: int)

    id: root
    ListView {
        id: listView
        anchors.fill: parent
        model: root.document === undefined ? 0 : root.document.pageCount
        spacing: 6
        highlightRangeMode: ListView.ApplyRange
        highlightMoveVelocity: 2000 // TODO increase velocity when setting currentIndex somehow, too
        onCurrentIndexChanged: {
            navigationStack.currentPage = currentIndex
            root.currentPageReallyChanged(currentIndex)
        }
        delegate: Rectangle {
            id: paper
            width: image.width
            height: image.height
            property alias selection: selection
            property real __pageScale: image.paintedWidth / document.pagePointSize(index).width
            Image {
                id: image
                source: document.source
                currentFrame: index
                asynchronous: true
                fillMode: Image.PreserveAspectFit
                width: document.pagePointSize(currentFrame).width
                height: document.pagePointSize(currentFrame).height
            }
            Shape {
                anchors.fill: parent
                opacity: 0.25
                visible: image.status === Image.Ready
                ShapePath {
                    strokeWidth: 1
                    strokeColor: "blue"
                    fillColor: "cyan"
                    scale: Qt.size(paper.__pageScale, paper.__pageScale)
                    PathMultiline {
                        id: searchResultBoundaries
                        paths: searchModel.matchGeometry
                    }
                }
                ShapePath {
                    fillColor: "orange"
                    scale: Qt.size(paper.__pageScale, paper.__pageScale)
                    PathMultiline {
                        id: selectionBoundaries
                        paths: selection.geometry
                    }
                }
            }
            PdfSearchModel {
                id: searchModel
                document: root.document
                page: image.currentFrame
                searchString: root.searchString
            }
            PdfSelection {
                id: selection
                document: root.document
                page: image.currentFrame
                fromPoint: Qt.point(textSelectionDrag.centroid.pressPosition.x / paper.__pageScale, textSelectionDrag.centroid.pressPosition.y / paper.__pageScale)
                toPoint: Qt.point(textSelectionDrag.centroid.position.x / paper.__pageScale, textSelectionDrag.centroid.position.y / paper.__pageScale)
                hold: !textSelectionDrag.active && !tapHandler.pressed
                onTextChanged: root.selectedText = text
            }
            DragHandler {
                id: textSelectionDrag
                acceptedDevices: PointerDevice.Mouse | PointerDevice.Stylus
                target: null
            }
            TapHandler {
                id: tapHandler
                acceptedDevices: PointerDevice.Mouse | PointerDevice.Stylus
            }
            Repeater {
                model: PdfLinkModel {
                    id: linkModel
                    document: root.document
                    page: image.currentFrame
                }
                delegate: Rectangle {
                    color: "transparent"
                    border.color: "lightgrey"
                    x: rect.x * paper.__pageScale
                    y: rect.y * paper.__pageScale
                    width: rect.width * paper.__pageScale
                    height: rect.height * paper.__pageScale
                    HoverHandler { cursorShape: Qt.PointingHandCursor } // 5.15 only (QTBUG-68073)
                    TapHandler {
                        onTapped: {
                            if (page >= 0)
                                listView.currentIndex = page
                            else
                                Qt.openUrlExternally(url)
                        }
                    }
                }
            }
        }
    }
    PdfNavigationStack {
        id: navigationStack
        onCurrentPageJumped: listView.currentIndex = page
        onCurrentPageChanged: root.currentPageReallyChanged(navigationStack.currentPage)
    }
}
