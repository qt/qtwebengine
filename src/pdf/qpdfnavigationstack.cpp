/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#include "qpdfnavigationstack.h"
#include "qpdfdocument.h"
#include "qpdflink_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcNav, "qt.pdf.navigationstack")

struct QPdfNavigationStackPrivate
{
    QPdfNavigationStack *q = nullptr;

    QList<QExplicitlySharedDataPointer<QPdfLinkPrivate>> pageHistory;
    int currentHistoryIndex = 0;
    bool changing = false;
};

/*!
    \class QPdfNavigationStack
    \since 6.4
    \inmodule QtPdf
    \brief Navigation history within a PDF document.

    The QPdfNavigationStack class remembers which destinations the user
    has visited in a PDF document, and provides the ability to traverse
    backward and forward. It is used to implement back and forward actions
    similar to the back and forward buttons in a web browser.

    \sa QPdfDocument
*/

/*!
    Constructs a page navigation stack with parent object \a parent.
*/
QPdfNavigationStack::QPdfNavigationStack(QObject *parent)
    : QObject(parent), d(new QPdfNavigationStackPrivate)
{
    d->q = this;
}

/*!
    Destroys the page navigation stack.
*/
QPdfNavigationStack::~QPdfNavigationStack()
{
}

/*!
    Goes back to the page, location and zoom level that was being viewed before
    back() was called, and then emits the \l jumped() signal.

    If a new destination was pushed since the last time \l back() was called,
    the forward() function does nothing, because there is a branch in the
    timeline which causes the "future" to be lost.
*/
void QPdfNavigationStack::forward()
{
    if (d->currentHistoryIndex >= d->pageHistory.count() - 1)
        return;
    const bool backAvailableWas = backAvailable();
    const bool forwardAvailableWas = forwardAvailable();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    ++d->currentHistoryIndex;
    d->changing = true;
    emit jumped(currentPage(), currentLocation(), currentZoom());
    if (currentZoomWas != currentZoom())
        emit currentZoomChanged(currentZoom());
    emit currentPageChanged(currentPage());
    if (currentLocationWas != currentLocation())
        emit currentLocationChanged(currentLocation());
    if (!backAvailableWas)
        emit backAvailableChanged(backAvailable());
    if (forwardAvailableWas != forwardAvailable())
        emit forwardAvailableChanged(forwardAvailable());
    d->changing = false;
    qCDebug(qLcNav) << "forward: index" << d->currentHistoryIndex << "page" << currentPage()
                    << "@" << currentLocation() << "zoom" << currentZoom();
}

/*!
    Pops the stack, updates the \l currentPage, \l currentLocation and
    \l currentZoom properties to the most-recently-viewed destination, and then
    emits the \l jumped() signal.
*/
void QPdfNavigationStack::back()
{
    if (d->currentHistoryIndex <= 0)
        return;
    const bool backAvailableWas = backAvailable();
    const bool forwardAvailableWas = forwardAvailable();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    --d->currentHistoryIndex;
    d->changing = true;
    emit jumped(currentPage(), currentLocation(), currentZoom());
    if (currentZoomWas != currentZoom())
        emit currentZoomChanged(currentZoom());
    emit currentPageChanged(currentPage());
    if (currentLocationWas != currentLocation())
        emit currentLocationChanged(currentLocation());
    if (backAvailableWas != backAvailable())
        emit backAvailableChanged(backAvailable());
    if (!forwardAvailableWas)
        emit forwardAvailableChanged(forwardAvailable());
    d->changing = false;
    qCDebug(qLcNav) << "back: index" << d->currentHistoryIndex << "page" << currentPage()
                    << "@" << currentLocation() << "zoom" << currentZoom();
}
/*!
    \property QPdfNavigationStack::currentPage

    This property holds the current page that is being viewed.
    The default is \c 0.
*/
int QPdfNavigationStack::currentPage() const
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.count())
        return -1; // only until ctor or clear() runs
    return d->pageHistory.at(d->currentHistoryIndex)->page;
}

/*!
    \property QPdfNavigationStack::currentLocation

    This property holds the current location on the page that is being viewed
    (the location that was last given to jump() or update()). The default is
    \c {0, 0}.
*/
QPointF QPdfNavigationStack::currentLocation() const
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.count())
        return QPointF();
    return d->pageHistory.at(d->currentHistoryIndex)->location;
}

/*!
    \property QPdfNavigationStack::currentZoom

    This property holds the magnification scale (1 logical pixel = 1 point)
    on the page that is being viewed. The default is \c 1.
*/
qreal QPdfNavigationStack::currentZoom() const
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.count())
        return 1;
    return d->pageHistory.at(d->currentHistoryIndex)->zoom;
}

/*!
    Adds the given destination, consisting of \a page, \a location, and \a zoom,
    to the history of visited locations.

    The \a zoom argument represents magnification (where \c 1 is the default
    scale, 1 logical pixel = 1 point). If \a zoom is given as \c 0, currentZoom
    keeps its existing value, and currentZoomChanged is not emitted.

    The \a location should be the same as QPdfLink::location() if the user is
    following a link; and since that is specified as the upper-left corner of
    the destination, it is best for consistency to always use the location
    visible in the upper-left corner of the viewport, in points.

    If forwardAvailable is \c true, calling this function represents a branch
    in the timeline which causes the "future" to be lost, and therefore
    forwardAvailable will change to \c false.
*/
void QPdfNavigationStack::jump(int page, const QPointF &location, qreal zoom)
{
    if (page == currentPage() && location == currentLocation() && zoom == currentZoom())
        return;
    if (qFuzzyIsNull(zoom))
        zoom = currentZoom();
    const bool zoomChange = !qFuzzyCompare(zoom, currentZoom());
    const bool pageChange = (page != currentPage());
    const bool locationChange = (location != currentLocation());
    const bool backAvailableWas = backAvailable();
    const bool forwardAvailableWas = forwardAvailable();
    if (!d->changing) {
        if (d->currentHistoryIndex >= 0 && forwardAvailableWas)
            d->pageHistory.remove(d->currentHistoryIndex + 1, d->pageHistory.count() - d->currentHistoryIndex - 1);
        d->pageHistory.append(QExplicitlySharedDataPointer<QPdfLinkPrivate>(new QPdfLinkPrivate(page, location, zoom)));
        d->currentHistoryIndex = d->pageHistory.count() - 1;
    }
    if (zoomChange)
        emit currentZoomChanged(currentZoom());
    if (pageChange)
        emit currentPageChanged(currentPage());
    if (locationChange)
        emit currentLocationChanged(currentLocation());
    if (d->changing)
        return;
    if (!backAvailableWas)
        emit backAvailableChanged(backAvailable());
    if (forwardAvailableWas)
        emit forwardAvailableChanged(forwardAvailable());
    emit jumped(page, location, zoom);
    qCDebug(qLcNav) << "push: index" << d->currentHistoryIndex << "page" << page
                    << "@" << location << "zoom" << zoom << "-> history" <<
        [this]() {
            QStringList ret;
            for (auto d : d->pageHistory)
                ret << QString::number(d->page);
            return ret.join(QLatin1Char(','));
        }();
}

/*!
    Modifies the current destination, consisting of \a page, \a location and \a zoom.

    This can be called periodically while the user is manually moving around
    the document, so that after back() is called, forward() will jump back to
    the most-recently-viewed destination rather than the destination that was
    last specified by push().

    The \c currentZoomChanged, \c currentPageChanged and \c currentLocationChanged
    signals will be emitted if the respective properties are actually changed.
    The \l jumped signal is not emitted, because this operation represents
    smooth movement rather than a navigational jump.
*/
void QPdfNavigationStack::update(int page, const QPointF &location, qreal zoom)
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.count())
        return;
    int currentPageWas = currentPage();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    if (page == currentPageWas && location == currentLocationWas && zoom == currentZoomWas)
        return;
    d->pageHistory[d->currentHistoryIndex]->page = page;
    d->pageHistory[d->currentHistoryIndex]->location = location;
    d->pageHistory[d->currentHistoryIndex]->zoom = zoom;
    if (currentZoomWas != zoom)
        emit currentZoomChanged(currentZoom());
    if (currentPageWas != page)
        emit currentPageChanged(currentPage());
    if (currentLocationWas != location)
        emit currentLocationChanged(currentLocation());
    qCDebug(qLcNav) << "update: index" << d->currentHistoryIndex << "page" << page
                    << "@" << location << "zoom" << zoom << "-> history" <<
        [this]() {
            QStringList ret;
            for (auto d : d->pageHistory)
                ret << QString::number(d->page);
            return ret.join(QLatin1Char(','));
        }();
}

/*!
    \property QPdfNavigationStack::backAvailable
    \readonly

    Holds \c true if a \e back destination is available in the history:
    that is, if push() or forward() has been called.
*/
bool QPdfNavigationStack::backAvailable() const
{
    return d->currentHistoryIndex > 0;
}

/*!
    \property QPdfNavigationStack::forwardAvailable
    \readonly

    Holds \c true if a \e forward destination is available in the history:
    that is, if back() has been previously called.
*/
bool QPdfNavigationStack::forwardAvailable() const
{
    return d->currentHistoryIndex < d->pageHistory.count() - 1;
}

/*!
    \fn void QPdfNavigationStack::jumped(int page, const QPointF &location, qreal zoom)

    This signal is emitted when an abrupt jump occurs, to the specified \a page
    index, \a location on the page, and \a zoom level; but \e not when simply
    scrolling through the document one page at a time. That is, jump(),
    forward() and back() emit this signal, but update() does not.
*/

QT_END_NAMESPACE

#include "moc_qpdfnavigationstack.cpp"
