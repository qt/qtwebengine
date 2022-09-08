// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickpdfpagenavigator_p.h"
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PdfPageNavigator
//!    \instantiates QQuickPdfPageNavigator
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief History of the destinations visited within a PDF Document.
    \since 5.15

    PdfPageNavigator remembers which destinations the user has visited in a PDF
    document, and provides the ability to traverse backward and forward.
*/

QQuickPdfPageNavigator::QQuickPdfPageNavigator(QObject *parent)
    : QObject(parent)
{
}

/*!
    \internal
*/
QQuickPdfPageNavigator::~QQuickPdfPageNavigator() = default;

/*!
    \internal
*/
QPdfPageNavigator *QQuickPdfPageNavigator::navStack()
{
    return static_cast<QPdfPageNavigator *>(qmlExtendedObject(this));
}

/*!
    \qmlmethod void PdfPageNavigator::forward()

    Goes back to the page, location and zoom level that was being viewed before
    back() was called, and then emits the \l jumped() signal.

    If a new destination was pushed since the last time \l back() was called,
    the forward() function does nothing, because there is a branch in the
    timeline which causes the "future" to be lost.
*/

/*!
    \qmlmethod void PdfPageNavigator::back()

    Pops the stack, updates the \l currentPage, \l currentLocation and
    \l currentZoom properties to the most-recently-viewed destination, and then
    emits the \l jumped() signal.
*/

/*!
    \qmlproperty int PdfPageNavigator::currentPage

    This property holds the current page that is being viewed.
    If there is no current page, it holds \c -1.
*/

/*!
    \qmlproperty point PdfPageNavigator::currentLocation

    This property holds the current location on the page that is being viewed.
*/

/*!
    \qmlproperty real PdfPageNavigator::currentZoom

    This property holds the magnification scale on the page that is being viewed.
*/

/*!
    \qmlmethod void PdfPageNavigator::jump(int page, point location, qreal zoom, bool emitJumped)

    Adds the given destination, consisting of \a page, \a location, and \a zoom,
    to the history of visited locations.  If \a emitJumped is \c false, the
    \l jumped() signal will not be emitted.

    If forwardAvailable is \c true, calling this function represents a branch
    in the timeline which causes the "future" to be lost, and therefore
    forwardAvailable will change to \c false.
*/

/*!
    \qmlmethod void PdfPageNavigator::update(int page, point location, qreal zoom)

    Modifies the current destination, consisting of \a page, \a location and \a zoom.

    This can be called periodically while the user is manually moving around
    the document, so that after back() is called, forward() will jump back to
    the most-recently-viewed destination rather than the destination that was
    last specified by jump().

    The \c currentZoomChanged, \c currentPageChanged and \c currentLocationChanged
    signals will be emitted if the respective properties are actually changed.
    The \l jumped signal is not emitted, because this operation
    represents smooth movement rather than a navigational jump.
*/

/*!
    \qmlproperty bool PdfPageNavigator::backAvailable
    \readonly

    Holds \c true if a \e back destination is available in the history.
*/

/*!
    \qmlproperty bool PdfPageNavigator::forwardAvailable
    \readonly

    Holds \c true if a \e forward destination is available in the history.
*/

/*!
    \qmlsignal PdfPageNavigator::jumped(int page, point location, qreal zoom)

    This signal is emitted when an abrupt jump occurs, to the specified \a page
    index, \a location on the page, and \a zoom level; but \e not when simply
    scrolling through the document one page at a time. That is, forward(),
    back() and jump() always emit this signal; update() does not.
*/

QT_END_NAMESPACE

#include "moc_qquickpdfpagenavigator_p.cpp"
