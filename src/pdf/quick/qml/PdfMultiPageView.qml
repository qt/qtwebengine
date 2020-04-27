/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
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
    function selectAll() {
        var currentItem = tableView.itemAtPos(0, tableView.contentY + root.height / 2)
        if (currentItem !== null)
            currentItem.selection.selectAll()
    }
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
        searchModel.currentPage = page
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
    PdfStyle { id: style }
    TableView {
        id: tableView
        anchors.fill: parent
        anchors.leftMargin: 2
        model: root.document === undefined ? 0 : root.document.pageCount
        rowSpacing: 6
        property real rotationNorm: Math.round((360 + (root.pageRotation % 360)) % 360)
        property bool rot90: rotationNorm == 90 || rotationNorm == 270
        onRot90Changed: forceLayout()
        property size firstPagePointSize: document === undefined ? Qt.size(0, 0) : document.pagePointSize(0)
        contentWidth: document === undefined ? 0 : (rot90 ? document.maxPageHeight : document.maxPageWidth) * root.renderScale + vscroll.width + 2
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
            if (root.debug && ret !== null)
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
                anchors.centerIn: pinch.active ? undefined : parent
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
                        searchHighlights.update()
                    }
                }
                Shape {
                    anchors.fill: parent
                    visible: image.status === Image.Ready
                    onVisibleChanged: searchHighlights.update()
                    ShapePath {
                        strokeWidth: -1
                        fillColor: style.pageSearchResultsColor
                        scale: Qt.size(paper.pageScale, paper.pageScale)
                        PathMultiline {
                            id: searchHighlights
                            function update() {
                                // paths could be a binding, but we need to be able to "kick" it sometimes
                                paths = searchModel.boundingPolygonsOnPage(index)
                            }
                        }
                    }
                    Connections {
                        target: searchModel
                        // whenever the highlights on the _current_ page change, they actually need to change on _all_ pages
                        // (usually because the search string has changed)
                        function onCurrentPageBoundingPolygonsChanged() { searchHighlights.update() }
                    }
                    ShapePath {
                        strokeWidth: -1
                        fillColor: style.selectionColor
                        scale: Qt.size(paper.pageScale, paper.pageScale)
                        PathMultiline {
                            paths: selection.geometry
                        }
                    }
                }
                Shape {
                    anchors.fill: parent
                    visible: image.status === Image.Ready && searchModel.currentPage === index
                    ShapePath {
                        strokeWidth: style.currentSearchResultStrokeWidth
                        strokeColor: style.currentSearchResultStrokeColor
                        fillColor: "transparent"
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
                    minimumRotation: root.pageRotation
                    maximumRotation: root.pageRotation
                    enabled: image.sourceSize.width < 5000
                    onActiveChanged:
                        if (active) {
                            paper.z = 10
                        } else {
                            paper.z = 0
                            var centroidInPoints = Qt.point(pinch.centroid.position.x / root.renderScale,
                                                            pinch.centroid.position.y / root.renderScale)
                            var centroidInFlickable = tableView.mapFromItem(paper, pinch.centroid.position.x, pinch.centroid.position.y)
                            var newSourceWidth = image.sourceSize.width * paper.scale
                            var ratio = newSourceWidth / image.sourceSize.width
                            if (root.debug)
                                console.log("pinch ended on page", index, "with centroid", pinch.centroid.position, centroidInPoints, "wrt flickable", centroidInFlickable,
                                            "page at", pageHolder.x.toFixed(2), pageHolder.y.toFixed(2),
                                            "contentX/Y were", tableView.contentX.toFixed(2), tableView.contentY.toFixed(2))
                            if (ratio > 1.1 || ratio < 0.9) {
                                var centroidOnPage = Qt.point(centroidInPoints.x * root.renderScale * ratio, centroidInPoints.y * root.renderScale * ratio)
                                paper.scale = 1
                                paper.x = 0
                                paper.y = 0
                                root.renderScale *= ratio
                                tableView.forceLayout()
                                if (tableView.rotationNorm == 0) {
                                    tableView.contentX = pageHolder.x + tableView.originX + centroidOnPage.x - centroidInFlickable.x
                                    tableView.contentY = pageHolder.y + tableView.originY + centroidOnPage.y - centroidInFlickable.y
                                } else if (tableView.rotationNorm == 90) {
                                    tableView.contentX = pageHolder.x + tableView.originX + image.height - centroidOnPage.y - centroidInFlickable.x
                                    tableView.contentY = pageHolder.y + tableView.originY + centroidOnPage.x - centroidInFlickable.y
                                } else if (tableView.rotationNorm == 180) {
                                    tableView.contentX = pageHolder.x + tableView.originX + image.width - centroidOnPage.x - centroidInFlickable.x
                                    tableView.contentY = pageHolder.y + tableView.originY + image.height - centroidOnPage.y - centroidInFlickable.y
                                } else if (tableView.rotationNorm == 270) {
                                    tableView.contentX = pageHolder.x + tableView.originX + centroidOnPage.y - centroidInFlickable.x
                                    tableView.contentY = pageHolder.y + tableView.originY + image.width - centroidOnPage.x - centroidInFlickable.y
                                }
                                if (root.debug)
                                    console.log("contentX/Y adjusted to", tableView.contentX.toFixed(2), tableView.contentY.toFixed(2), "y @top", pageHolder.y)
                                tableView.returnToBounds()
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
                    delegate: Shape {
                        x: rect.x * paper.pageScale
                        y: rect.y * paper.pageScale
                        width: rect.width * paper.pageScale
                        height: rect.height * paper.pageScale
                        ShapePath {
                            strokeWidth: style.linkUnderscoreStrokeWidth
                            strokeColor: style.linkUnderscoreColor
                            strokeStyle: style.linkUnderscoreStrokeStyle
                            dashPattern: style.linkUnderscoreDashPattern
                            startX: 0; startY: height
                            PathLine { x: width; y: height }
                        }
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
            id: vscroll
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
        onCurrentPageChanged: if (currentPage != navigationStack.currentPage) root.goToPage(currentPage)
    }
}
