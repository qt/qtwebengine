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
import QtQuick.Shapes 1.14
import QtQuick.Window 2.14

Item {
    // public API
    // TODO 5.15: required property
    property var document: undefined
    property real renderScale: 1
    property real pageRotation: 0
    property string searchString
    property string selectedText
    property alias currentPage: navigationStack.currentPage
    function copySelectionToClipboard() {
        if (listView.currentItem !== null)
            listView.currentItem.selection.copyToClipboard()
    }
    property alias backEnabled: navigationStack.backAvailable
    property alias forwardEnabled: navigationStack.forwardAvailable
    function back() {
        navigationStack.back()
    }
    function forward() {
        navigationStack.forward()
    }

    function resetScale() {
        root.renderScale = 1
    }

    function scaleToWidth(width, height) {
        root.renderScale = width / (listView.rot90 ? listView.firstPagePointSize.height : listView.firstPagePointSize.width)
    }

    function scaleToPage(width, height) {
        var windowAspect = width / height
        var pageAspect = listView.firstPagePointSize.width / listView.firstPagePointSize.height
        if (listView.rot90) {
            if (windowAspect > pageAspect) {
                root.renderScale = height / listView.firstPagePointSize.width
            } else {
                root.renderScale = width / listView.firstPagePointSize.height
            }
        } else {
            if (windowAspect > pageAspect) {
                root.renderScale = height / listView.firstPagePointSize.height
            } else {
                root.renderScale = width / listView.firstPagePointSize.width
            }
        }
    }

    function goToPage(page) {
        goToLocation(page, Qt.point(0, 0), 0)
    }

    function goToLocation(page, location, zoom) {
        if (zoom > 0)
            root.renderScale = zoom
        navigationStack.push(page, location, zoom)
    }

    id: root
    ListView {
        id: listView
        anchors.fill: parent
        model: root.document === undefined ? 0 : root.document.pageCount
        spacing: 6
        highlightRangeMode: ListView.ApplyRange
        highlightMoveVelocity: 2000 // TODO increase velocity when setting currentIndex somehow, too
        property real rotationModulus: Math.abs(root.pageRotation % 180)
        property bool rot90: rotationModulus > 45 && rotationModulus < 135
        property size firstPagePointSize: document === undefined ? Qt.size(0, 0) : document.pagePointSize(0)
        delegate: Rectangle {
            id: paper
            implicitWidth: image.width
            implicitHeight: image.height
            rotation: root.pageRotation
            property alias selection: selection
            property size pagePointSize: document.pagePointSize(index)
            property real pageScale: image.paintedWidth / pagePointSize.width
            Image {
                id: image
                source: document.source
                currentFrame: index
                asynchronous: true
                fillMode: Image.PreserveAspectFit
                width: pagePointSize.width * root.renderScale
                height: pagePointSize.height * root.renderScale
                property real renderScale: root.renderScale
                property real oldRenderScale: 1
                onRenderScaleChanged: {
                    image.sourceSize.width = pagePointSize.width * renderScale
                    image.sourceSize.height = 0
                    paper.scale = 1
                    paper.x = 0
                    paper.y = 0
                }
            }
            Shape {
                anchors.fill: parent
                opacity: 0.25
                visible: image.status === Image.Ready
                ShapePath {
                    strokeWidth: 1
                    strokeColor: "blue"
                    fillColor: "cyan"
                    scale: Qt.size(paper.pageScale, paper.pageScale)
                    PathMultiline {
                        id: searchResultBoundaries
                        paths: searchModel.matchGeometry
                    }
                }
                ShapePath {
                    fillColor: "orange"
                    scale: Qt.size(paper.pageScale, paper.pageScale)
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
                fromPoint: Qt.point(textSelectionDrag.centroid.pressPosition.x / paper.pageScale, textSelectionDrag.centroid.pressPosition.y / paper.pageScale)
                toPoint: Qt.point(textSelectionDrag.centroid.position.x / paper.pageScale, textSelectionDrag.centroid.position.y / paper.pageScale)
                hold: !textSelectionDrag.active && !tapHandler.pressed
                onTextChanged: root.selectedText = text
            }
            function reRenderIfNecessary() {
                var newSourceWidth = image.sourceSize.width * paper.scale
                var ratio = newSourceWidth / image.sourceSize.width
                if (ratio > 1.1 || ratio < 0.9) {
                    image.sourceSize.height = 0
                    image.sourceSize.width = newSourceWidth
                    paper.scale = 1
                }
            }
            PinchHandler {
                id: pinch
                minimumScale: 0.1
                maximumScale: 10
                minimumRotation: 0
                maximumRotation: 0
                onActiveChanged:
                    if (active) {
                        paper.z = 10
                    } else {
                        paper.x = 0
                        paper.y = 0
                        paper.z = 0
                        image.width = undefined
                        image.height = undefined
                        paper.reRenderIfNecessary()
                    }
                grabPermissions: PointerHandler.CanTakeOverFromAnything
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
                    x: rect.x * paper.pageScale
                    y: rect.y * paper.pageScale
                    width: rect.width * paper.pageScale
                    height: rect.height * paper.pageScale
                    MouseArea { // TODO switch to TapHandler / HoverHandler in 5.15
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (page >= 0)
                                root.goToLocation(page, location, zoom)
                            else
                                Qt.openUrlExternally(url)
                        }
                    }
                }
            }
        }
        ScrollBar.vertical: ScrollBar {
            property bool moved: false
            onPositionChanged: moved = true
            onActiveChanged: {
                var currentPage = listView.indexAt(0, listView.contentY)
                var currentItem = listView.itemAtIndex(currentPage)
                var currentLocation = Qt.point(0, listView.contentY - currentItem.y)
                if (active) {
                    moved = false
                    navigationStack.push(currentPage, currentLocation, root.renderScale);
                } else if (moved) {
                    navigationStack.update(currentPage, currentLocation, root.renderScale);
                }
            }
        }
    }
    PdfNavigationStack {
        id: navigationStack
        onJumped: listView.currentIndex = page
        onCurrentPageChanged: listView.positionViewAtIndex(currentPage, ListView.Beginning)
        onCurrentLocationChanged: listView.contentY += currentLocation.y // currentPageChanged() MUST occur first!
        onCurrentZoomChanged: root.renderScale = currentZoom
        // TODO deal with horizontal location (need another Flickable probably)
    }
}
