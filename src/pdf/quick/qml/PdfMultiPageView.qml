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
    property bool debug: false

    property string selectedText
    function copySelectionToClipboard() {
        var currentItem = tableView.itemAtPos(0, tableView.contentY + root.height / 2)
        if (debug)
            console.log("currentItem", currentItem, "sel", currentItem.selection.text)
        if (currentItem !== null)
            currentItem.selection.copyToClipboard()
    }

    // page navigation
    property alias currentPage: navigationStack.currentPage
    property alias backEnabled: navigationStack.backAvailable
    property alias forwardEnabled: navigationStack.forwardAvailable
    function back() { navigationStack.back() }
    function forward() { navigationStack.forward() }
    function goToPage(page) {
        if (page === navigationStack.currentPage)
            return
        goToLocation(page, Qt.point(0, 0), 0)
    }
    function goToLocation(page, location, zoom) {
        if (zoom > 0)
            root.renderScale = zoom
        navigationStack.push(page, location, zoom)
    }

    // page scaling
    property real renderScale: 1
    property real pageRotation: 0
    function resetScale() { root.renderScale = 1 }
    function scaleToWidth(width, height) {
        root.renderScale = width / (tableView.rot90 ? tableView.firstPagePointSize.height : tableView.firstPagePointSize.width)
    }
    function scaleToPage(width, height) {
        var windowAspect = width / height
        var pageAspect = tableView.firstPagePointSize.width / tableView.firstPagePointSize.height
        if (tableView.rot90) {
            if (windowAspect > pageAspect) {
                root.renderScale = height / tableView.firstPagePointSize.width
            } else {
                root.renderScale = width / tableView.firstPagePointSize.height
            }
        } else {
            if (windowAspect > pageAspect) {
                root.renderScale = height / tableView.firstPagePointSize.height
            } else {
                root.renderScale = width / tableView.firstPagePointSize.width
            }
        }
    }

    // text search
    property alias searchModel: searchModel
    property alias searchString: searchModel.searchString
    function searchBack() { --searchModel.currentResult }
    function searchForward() { ++searchModel.currentResult }

    id: root
    TableView {
        id: tableView
        anchors.fill: parent
        model: root.document === undefined ? 0 : root.document.pageCount
        rowSpacing: 6
        property real rotationModulus: Math.abs(root.pageRotation % 180)
        property bool rot90: rotationModulus > 45 && rotationModulus < 135
        onRot90Changed: forceLayout()
        property size firstPagePointSize: document === undefined ? Qt.size(0, 0) : document.pagePointSize(0)
        contentWidth: document === undefined ? 0 : document.maxPageWidth * root.renderScale
        // workaround for missing function (see https://codereview.qt-project.org/c/qt/qtdeclarative/+/248464)
        function itemAtPos(x, y, includeSpacing) {
            // we don't care about x (assume col 0), and assume includeSpacing is true
            var ret = null
            for (var i = 0; i < contentItem.children.length; ++i) {
                var child = contentItem.children[i];
                if (root.debug)
                    console.log(child, "@y", child.y)
                if (child.y < y && (!ret || child.y > ret.y))
                    ret = child
            }
            if (root.debug)
                console.log("given y", y, "found", ret, "@", ret.y)
            return ret // the delegate with the largest y that is less than the given y
        }
        rowHeightProvider: function(row) { return (rot90 ? document.pagePointSize(row).width : document.pagePointSize(row).height) * root.renderScale }
        delegate: Rectangle {
            id: pageHolder
            color: root.debug ? "beige" : "transparent"
            Text {
                visible: root.debug
                anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                rotation: -90; text: pageHolder.width.toFixed(1) + "x" + pageHolder.height.toFixed(1) + "\n" +
                                     image.width.toFixed(1) + "x" + image.height.toFixed(1)
            }
            implicitWidth: Math.max(root.width, (tableView.rot90 ? document.maxPageHeight : document.maxPageWidth) * root.renderScale)
            implicitHeight: tableView.rot90 ? image.width : image.height
            onImplicitWidthChanged: tableView.forceLayout()
            objectName: "page " + index
            property int delegateIndex: row // expose the context property for JS outside of the delegate
            property alias selection: selection
            Rectangle {
                id: paper
                width: image.width
                height: image.height
                rotation: root.pageRotation
                anchors.centerIn: parent
                property size pagePointSize: document.pagePointSize(index)
                property real pageScale: image.paintedWidth / pagePointSize.width
                Image {
                    id: image
                    source: document.source
                    currentFrame: index
                    asynchronous: true
                    fillMode: Image.PreserveAspectFit
                    width: paper.pagePointSize.width * root.renderScale
                    height: paper.pagePointSize.height * root.renderScale
                    property real renderScale: root.renderScale
                    property real oldRenderScale: 1
                    onRenderScaleChanged: {
                        image.sourceSize.width = paper.pagePointSize.width * renderScale
                        image.sourceSize.height = 0
                        paper.scale = 1
                    }
                }
                Shape {
                    anchors.fill: parent
                    opacity: 0.25
                    visible: image.status === Image.Ready
                    ShapePath {
                        strokeWidth: 1
                        strokeColor: "cyan"
                        fillColor: "steelblue"
                        scale: Qt.size(paper.pageScale, paper.pageScale)
                        PathMultiline {
                            paths: searchModel.boundingPolygonsOnPage(index)
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
                Shape {
                    anchors.fill: parent
                    opacity: 0.5
                    visible: image.status === Image.Ready && searchModel.currentPage === index
                    ShapePath {
                        strokeWidth: 1
                        strokeColor: "blue"
                        fillColor: "cyan"
                        scale: Qt.size(paper.pageScale, paper.pageScale)
                        PathMultiline {
                            paths: searchModel.currentResultBoundingPolygons
                        }
                    }
                }
                PinchHandler {
                    id: pinch
                    minimumScale: 0.1
                    maximumScale: root.renderScale < 4 ? 2 : 1
                    minimumRotation: 0
                    maximumRotation: 0
                    enabled: image.sourceSize.width < 5000
                    onActiveChanged:
                        if (active) {
                            paper.z = 10
                        } else {
                            paper.z = 0
                            var newSourceWidth = image.sourceSize.width * paper.scale
                            var ratio = newSourceWidth / image.sourceSize.width
                            if (ratio > 1.1 || ratio < 0.9) {
                                paper.scale = 1
                                root.renderScale *= ratio
                            }
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
                            id: linkMA
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: {
                                if (page >= 0)
                                    root.goToLocation(page, location, zoom)
                                else
                                    Qt.openUrlExternally(url)
                            }
                        }
                        ToolTip {
                            visible: linkMA.containsMouse
                            delay: 1000
                            text: page >= 0 ?
                                      ("page " + (page + 1) +
                                       " location " + location.x.toFixed(1) + ", " + location.y.toFixed(1) +
                                       " zoom " + zoom) : url
                        }
                    }
                }
            }
            PdfSelection {
                id: selection
                document: root.document
                page: image.currentFrame
                fromPoint: Qt.point(textSelectionDrag.centroid.pressPosition.x / paper.pageScale,
                                    textSelectionDrag.centroid.pressPosition.y / paper.pageScale)
                toPoint: Qt.point(textSelectionDrag.centroid.position.x / paper.pageScale,
                                  textSelectionDrag.centroid.position.y / paper.pageScale)
                hold: !textSelectionDrag.active && !tapHandler.pressed
                onTextChanged: root.selectedText = text
            }
        }
        ScrollBar.vertical: ScrollBar {
            property bool moved: false
            onPositionChanged: moved = true
            onActiveChanged: {
                var currentItem = tableView.itemAtPos(0, tableView.contentY + root.height / 2)
                var currentPage = currentItem.delegateIndex
                var currentLocation = Qt.point((tableView.contentX - currentItem.x + root.width / 2) / root.renderScale,
                                               (tableView.contentY - currentItem.y + root.height / 2) / root.renderScale)
                if (active) {
                    moved = false
                    navigationStack.push(currentPage, currentLocation, root.renderScale)
                } else if (moved) {
                    navigationStack.update(currentPage, currentLocation, root.renderScale)
                }
            }
        }
        ScrollBar.horizontal: ScrollBar { }
    }
    onRenderScaleChanged: {
        tableView.forceLayout()
        var currentItem = tableView.itemAtPos(tableView.contentX + root.width / 2, tableView.contentY + root.height / 2)
        if (currentItem !== undefined)
            navigationStack.update(currentItem.delegateIndex, Qt.point(currentItem.x / renderScale, currentItem.y / renderScale), renderScale)
    }
    PdfNavigationStack {
        id: navigationStack
        onJumped: {
            root.renderScale = zoom
            tableView.contentX = Math.max(0, location.x - root.width / 2) * root.renderScale
            tableView.contentY = tableView.originY + root.document.heightSumBeforePage(page, tableView.rowSpacing / root.renderScale) * root.renderScale
            if (root.debug) {
                console.log("going to page", page,
                            "@y", root.document.heightSumBeforePage(page, tableView.rowSpacing / root.renderScale) * root.renderScale,
                            "ended up @", tableView.contentY, "originY is", tableView.originY)
            }
        }
    }
    PdfSearchModel {
        id: searchModel
        document: root.document === undefined ? null : root.document
        onCurrentPageChanged: root.goToPage(currentPage)
    }
}
