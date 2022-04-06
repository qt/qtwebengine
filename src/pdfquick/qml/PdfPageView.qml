/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Pdf 5.15
import QtQuick.Shapes 1.14
import Qt.labs.animation 1.0

/*!
    \qmltype PdfPageView
    \inqmlmodule QtQuick.Pdf
    \brief A PDF viewer component to show one page a time.

    PdfPageView provides a PDF viewer component that shows one whole page at a
    time, without scrolling. It supports selecting text and copying it to the
    clipboard, zooming in and out, clicking an internal link to jump to another
    section in the document, rotating the view, and searching for text.

    The implementation is a QML assembly of smaller building blocks that are
    available separately. In case you want to make changes in your own version
    of this component, you can copy the QML, which is installed into the
    \c QtQuick/Pdf/qml module directory, and modify it as needed.

    \sa PdfScrollablePageView, PdfMultiPageView, PdfStyle
*/
Rectangle {
    /*!
        \qmlproperty PdfDocument PdfPageView::document

        A PdfDocument object with a valid \c source URL is required:

        \snippet multipageview.qml 0
    */
    required property PdfDocument document

    /*!
        \qmlproperty int PdfPageView::status

        This property holds the \l {QtQuick::Image::status}{rendering status} of
        the \l {currentPage}{current page}.

        \sa PdfPageImage::status
    */
    property alias status: image.status

    /*!
        \qmlproperty PdfDocument PdfPageView::selectedText

        The selected text.
    */
    property alias selectedText: selection.text

    /*!
        \qmlmethod void PdfPageView::selectAll()

        Selects all the text on the \l {currentPage}{current page}, and makes it
        available as the system \l {QClipboard::Selection}{selection} on systems
        that support that feature.

        \sa copySelectionToClipboard()
    */
    function selectAll() {
        selection.selectAll()
    }

    /*!
        \qmlmethod void PdfPageView::copySelectionToClipboard()

        Copies the selected text (if any) to the
        \l {QClipboard::Clipboard}{system clipboard}.

        \sa selectAll()
    */
    function copySelectionToClipboard() {
        selection.copyToClipboard()
    }

    // --------------------------------
    // page navigation

    /*!
        \qmlproperty int PdfPageView::currentPage
        \readonly

        This property holds the zero-based page number of the page visible in the
        scrollable view. If there is no current page, it holds -1.

        This property is read-only, and is typically used in a binding (or
        \c onCurrentPageChanged script) to update the part of the user interface
        that shows the current page number, such as a \l SpinBox.

        \sa PdfNavigationStack::currentPage
    */
    property alias currentPage: navigationStack.currentPage

    /*!
        \qmlproperty bool PdfPageView::backEnabled
        \readonly

        This property indicates if it is possible to go back in the navigation
        history to a previous-viewed page.

        \sa PdfNavigationStack::backAvailable, back()
    */
    property alias backEnabled: navigationStack.backAvailable

    /*!
        \qmlproperty bool PdfPageView::forwardEnabled
        \readonly

        This property indicates if it is possible to go to next location in the
        navigation history.

        \sa PdfNavigationStack::forwardAvailable, forward()
    */
    property alias forwardEnabled: navigationStack.forwardAvailable

    /*!
        \qmlmethod void PdfPageView::back()

        Scrolls the view back to the previous page that the user visited most
        recently; or does nothing if there is no previous location on the
        navigation stack.

        \sa PdfNavigationStack::back(), currentPage, backEnabled
    */
    function back() { navigationStack.back() }

    /*!
        \qmlmethod void PdfPageView::forward()

        Scrolls the view to the page that the user was viewing when the back()
        method was called; or does nothing if there is no "next" location on the
        navigation stack.

        \sa PdfNavigationStack::forward(), currentPage
    */
    function forward() { navigationStack.forward() }

    /*!
        \qmlmethod void PdfPageView::goToPage(int page)

        Changes the view to the \a page, if possible.

        \sa PdfNavigationStack::jump(), currentPage
    */
    function goToPage(page) { goToLocation(page, Qt.point(0, 0), 0) }

    /*!
        \qmlmethod void PdfPageView::goToLocation(int page, point location, real zoom)

        Scrolls the view to the \a location on the \a page, if possible,
        and sets the \a zoom level.

        \sa PdfNavigationStack::jump(), currentPage
    */
    function goToLocation(page, location, zoom) {
        if (zoom > 0)
            root.renderScale = zoom
        navigationStack.push(page, location, zoom)
    }

    // --------------------------------
    // page scaling

    /*!
        \qmlproperty real PdfPageView::renderScale

        This property holds the ratio of pixels to points. The default is \c 1,
        meaning one point (1/72 of an inch) equals 1 logical pixel.

        \sa PdfPageImage::status
    */
    property real renderScale: 1

    /*!
        \qmlproperty size PdfPageView::sourceSize

        This property holds the scaled width and height of the full-frame image.

        \sa PdfPageImage::sourceSize
    */
    property alias sourceSize: image.sourceSize

    /*!
        \qmlmethod void PdfPageView::resetScale()

        Sets \l renderScale back to its default value of \c 1.
    */
    function resetScale() {
        image.sourceSize.width = 0
        image.sourceSize.height = 0
        root.x = 0
        root.y = 0
        root.scale = 1
    }

    /*!
        \qmlmethod void PdfPageView::scaleToWidth(real width, real height)

        Sets \l renderScale such that the width of the first page will fit into a
        viewport with the given \a width and \a height. If the page is not rotated,
        it will be scaled so that its width fits \a width. If it is rotated +/- 90
        degrees, it will be scaled so that its width fits \a height.
    */
    function scaleToWidth(width, height) {
        var halfRotation = Math.abs(root.rotation % 180)
        image.sourceSize = Qt.size((halfRotation > 45 && halfRotation < 135) ? height : width, 0)
        root.x = 0
        root.y = 0
        image.centerInSize = Qt.size(width, height)
        image.centerOnLoad = true
        image.vCenterOnLoad = (halfRotation > 45 && halfRotation < 135)
        root.scale = 1
    }

    /*!
        \qmlmethod void PdfPageView::scaleToPage(real width, real height)

        Sets \l renderScale such that the whole first page will fit into a viewport
        with the given \a width and \a height. The resulting \l renderScale depends
        on \l pageRotation: the page will fit into the viewport at a larger size if
        it is first rotated to have a matching aspect ratio.
    */
    function scaleToPage(width, height) {
        var windowAspect = width / height
        var halfRotation = Math.abs(root.rotation % 180)
        var pagePointSize = document.pagePointSize(navigationStack.currentPage)
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
        root.scale = 1
    }

    // --------------------------------
    // text search

    /*!
        \qmlproperty PdfSearchModel PdfPageView::searchModel

        This property holds a PdfSearchModel containing the list of search results
        for a given \l searchString.

        \sa PdfSearchModel
    */
    property alias searchModel: searchModel

    /*!
        \qmlproperty string PdfPageView::searchString

        This property holds the search string that the user may choose to search
        for. It is typically used in a binding to the
        \l {QtQuick.Controls::TextField::text}{text} property of a TextField.

        \sa searchModel
    */
    property alias searchString: searchModel.searchString

    /*!
        \qmlmethod void PdfPageView::searchBack()

        Decrements the
        \l{PdfSearchModel::currentResult}{searchModel's current result}
        so that the view will jump to the previous search result.
    */
    function searchBack() { --searchModel.currentResult }

    /*!
        \qmlmethod void PdfPageView::searchForward()

        Increments the
        \l{PdfSearchModel::currentResult}{searchModel's current result}
        so that the view will jump to the next search result.
    */
    function searchForward() { ++searchModel.currentResult }

    // --------------------------------
    // implementation
    id: root
    width: image.width
    height: image.height

    PdfSelection {
        id: selection
        document: root.document
        page: navigationStack.currentPage
        fromPoint: Qt.point(textSelectionDrag.centroid.pressPosition.x / image.pageScale, textSelectionDrag.centroid.pressPosition.y / image.pageScale)
        toPoint: Qt.point(textSelectionDrag.centroid.position.x / image.pageScale, textSelectionDrag.centroid.position.y / image.pageScale)
        hold: !textSelectionDrag.active && !tapHandler.pressed
    }

    PdfSearchModel {
        id: searchModel
        document: root.document === undefined ? null : root.document
        onCurrentPageChanged: root.goToPage(currentPage)
    }

    PdfNavigationStack {
        id: navigationStack
        onCurrentPageChanged: searchModel.currentPage = currentPage
        // TODO onCurrentLocationChanged: position currentLocation.x and .y in middle // currentPageChanged() MUST occur first!
        onCurrentZoomChanged: root.renderScale = currentZoom
        // TODO deal with horizontal location (need WheelHandler or Flickable probably)
    }

    Image {
        id: image
        currentFrame: navigationStack.currentPage
        source: document.status === PdfDocument.Ready ? document.source : ""
        asynchronous: true
        fillMode: Image.PreserveAspectFit
        property bool centerOnLoad: false
        property bool vCenterOnLoad: false
        property size centerInSize
        property real pageScale: image.paintedWidth / document.pagePointSize(navigationStack.currentPage).width
        function reRenderIfNecessary() {
            var newSourceWidth = image.sourceSize.width * root.scale
            var ratio = newSourceWidth / image.sourceSize.width
            if (ratio > 1.1 || ratio < 0.9) {
                image.sourceSize.width = newSourceWidth
                image.sourceSize.height = 0
                root.scale = 1
            }
        }
        onStatusChanged:
            if (status == Image.Ready && centerOnLoad) {
                root.x = (centerInSize.width - image.implicitWidth) / 2
                root.y = vCenterOnLoad ? (centerInSize.height - image.implicitHeight) / 2 : 0
                centerOnLoad = false
                vCenterOnLoad = false
            }
    }
    onRenderScaleChanged: {
        image.sourceSize.width = document.pagePointSize(navigationStack.currentPage).width * renderScale
        image.sourceSize.height = 0
        root.scale = 1
    }

    Shape {
        anchors.fill: parent
        opacity: 0.25
        visible: image.status === Image.Ready
        ShapePath {
            strokeWidth: 1
            strokeColor: "cyan"
            fillColor: "steelblue"
            scale: Qt.size(image.pageScale, image.pageScale)
            PathMultiline {
                paths: searchModel.currentPageBoundingPolygons
            }
        }
        ShapePath {
            strokeWidth: 1
            strokeColor: "orange"
            fillColor: "cyan"
            scale: Qt.size(image.pageScale, image.pageScale)
            PathMultiline {
                paths: searchModel.currentResultBoundingPolygons
            }
        }
        ShapePath {
            fillColor: "orange"
            scale: Qt.size(image.pageScale, image.pageScale)
            PathMultiline {
                paths: selection.geometry
            }
        }
    }

    Repeater {
        model: PdfLinkModel {
            id: linkModel
            document: root.document
            page: navigationStack.currentPage
        }
        delegate: Rectangle {
            color: "transparent"
            border.color: "lightgrey"
            x: rect.x * image.pageScale
            y: rect.y * image.pageScale
            width: rect.width * image.pageScale
            height: rect.height * image.pageScale
            MouseArea { // TODO switch to TapHandler / HoverHandler in 5.15
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (page >= 0)
                        navigationStack.push(page, Qt.point(0, 0), root.renderScale)
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
        onActiveChanged: if (!active) image.reRenderIfNecessary()
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
    // prevent it from being scrolled out of view
    BoundaryRule on x {
        minimum: 100 - root.width
        maximum: root.parent.width - 100
    }
    BoundaryRule on y {
        minimum: 100 - root.height
        maximum: root.parent.height - 100
    }
}
