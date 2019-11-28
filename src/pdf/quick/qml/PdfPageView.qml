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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Pdf 5.15
import QtQuick.Shapes 1.15

Rectangle {
    id: paper
    width: image.width
    height: image.height

    // public API
    // TODO 5.15: required property
    property var document: null
    property real renderScale: 1
    property alias sourceSize: image.sourceSize
    property alias currentPage: image.currentFrame
    property alias pageCount: image.frameCount
    property alias searchString: searchModel.searchString
    property alias selectedText: selection.text
    property alias status: image.status

    property real __pageScale: image.paintedWidth / document.pagePointSize(image.currentFrame).width

    function resetScale() {
        image.sourceSize.width = 0
        image.sourceSize.height = 0
        paper.x = 0
        paper.y = 0
        paper.scale = 1
    }

    function scaleToWidth(width, height) {
        var halfRotation = Math.abs(paper.rotation % 180)
        image.sourceSize = Qt.size((halfRotation > 45 && halfRotation < 135) ? height : width, 0)
        paper.x = 0
        paper.y = 0
        image.centerInSize = Qt.size(width, height)
        image.centerOnLoad = true
        image.vCenterOnLoad = (halfRotation > 45 && halfRotation < 135)
        paper.scale = 1
    }

    function scaleToPage(width, height) {
        var windowAspect = width / height
        var halfRotation = Math.abs(paper.rotation % 180)
        var pagePointSize = document.pagePointSize(image.currentFrame)
        if (halfRotation > 45 && halfRotation < 135) {
            // rotated 90 or 270º
            var pageAspect = pagePointSize.height / pagePointSize.width
            if (windowAspect > pageAspect) {
                image.sourceSize = Qt.size(height, 0)
            } else {
                image.sourceSize = Qt.size(0, width)
            }
        } else {
            var pageAspect = pagePointSize.width / pagePointSize.height
            if (windowAspect > pageAspect) {
                image.sourceSize = Qt.size(0, height)
            } else {
                image.sourceSize = Qt.size(width, 0)
            }
        }
        image.centerInSize = Qt.size(width, height)
        image.centerOnLoad = true
        image.vCenterOnLoad = true
        paper.scale = 1
    }

    PdfSelection {
        id: selection
        document: paper.document
        page: image.currentFrame
        fromPoint: Qt.point(textSelectionDrag.centroid.pressPosition.x / paper.__pageScale, textSelectionDrag.centroid.pressPosition.y / paper.__pageScale)
        toPoint: Qt.point(textSelectionDrag.centroid.position.x / paper.__pageScale, textSelectionDrag.centroid.position.y / paper.__pageScale)
        hold: !textSelectionDrag.active && !tapHandler.pressed
    }
    function copySelectionToClipboard() {
        selection.copyToClipboard()
    }

    PdfSearchModel {
        id: searchModel
        document: paper.document
        page: image.currentFrame
    }

    Image {
        id: image
        source: document.status === PdfDocument.Ready ? document.source : ""
        asynchronous: true
        fillMode: Image.PreserveAspectFit
        property bool centerOnLoad: false
        property bool vCenterOnLoad: false
        property size centerInSize
        onStatusChanged:
            if (status == Image.Ready && centerOnLoad) {
                paper.x = (centerInSize.width - image.implicitWidth) / 2
                paper.y = vCenterOnLoad ? (centerInSize.height - image.implicitHeight) / 2 : 0
                centerOnLoad = false
                vCenterOnLoad = false
            }
    }
    function reRenderIfNecessary() {
        var newSourceWidth = image.sourceSize.width * paper.scale
        var ratio = newSourceWidth / image.sourceSize.width
        if (ratio > 1.1 || ratio < 0.9) {
            image.sourceSize.width = newSourceWidth
            image.sourceSize.height = 0
            paper.scale = 1
        }
    }
    onRenderScaleChanged: {
        image.sourceSize.width = document.pagePointSize(image.currentFrame).width * renderScale
        image.sourceSize.height = 0
        paper.scale = 1
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

    Repeater {
        model: PdfLinkModel {
            id: linkModel
            document: paper.document
            page: image.currentFrame
        }
        delegate: Rectangle {
            color: "transparent"
            border.color: "lightgrey"
            x: rect.x * paper.__pageScale
            y: rect.y * paper.__pageScale
            width: rect.width * paper.__pageScale
            height: rect.height * paper.__pageScale
            HoverHandler { cursorShape: Qt.PointingHandCursor } // 5.15 onward (QTBUG-68073)
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

    PinchHandler {
        id: pinch
        minimumScale: 0.1
        maximumScale: 10
        minimumRotation: 0
        maximumRotation: 0
        onActiveChanged: if (!active) paper.reRenderIfNecessary()
        grabPermissions: PinchHandler.TakeOverForbidden // don't allow takeover if pinch has started
    }
    DragHandler {
        id: pageMovingTouchDrag
        acceptedDevices: PointerDevice.TouchScreen
    }
    DragHandler {
        id: pageMovingMiddleMouseDrag
        acceptedDevices: PointerDevice.Mouse | PointerDevice.Stylus
        acceptedButtons: Qt.MiddleButton
        snapMode: DragHandler.NoSnap
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
}
