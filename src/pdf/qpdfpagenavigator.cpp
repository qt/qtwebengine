// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdfpagenavigator.h"
#include "qpdfdocument.h"
#include "qpdflink_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcNav, "qt.pdf.pagenavigator")

struct QPdfPageNavigatorPrivate
{
    QPdfPageNavigator *q = nullptr;

    QList<QExplicitlySharedDataPointer<QPdfLinkPrivate>> pageHistory;
    int currentHistoryIndex = 0;
    bool changing = false;
};

/*!
    \class QPdfPageNavigator
    \since 6.4
    \inmodule QtPdf
    \brief Navigation history within a PDF document.

    The QPdfPageNavigator class remembers which destinations the user
    has visited in a PDF document, and provides the ability to traverse
    backward and forward. It is used to implement back and forward actions
    similar to the back and forward buttons in a web browser.

    \sa QPdfDocument
*/

/*!
    Constructs a page navigation stack with parent object \a parent.
*/
QPdfPageNavigator::QPdfPageNavigator(QObject *parent)
    : QObject(parent), d(new QPdfPageNavigatorPrivate)
{
    d->q = this;
    clear();
}

/*!
    Destroys the page navigation stack.
*/
QPdfPageNavigator::~QPdfPageNavigator()
{
}

/*!
    Goes back to the page, location and zoom level that was being viewed before
    back() was called, and then emits the \l jumped() signal.

    If a new destination was pushed since the last time \l back() was called,
    the forward() function does nothing, because there is a branch in the
    timeline which causes the "future" to be lost.
*/
void QPdfPageNavigator::forward()
{
    if (d->currentHistoryIndex >= d->pageHistory.size() - 1)
        return;
    const bool backAvailableWas = backAvailable();
    const bool forwardAvailableWas = forwardAvailable();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    ++d->currentHistoryIndex;
    d->changing = true;
    emit jumped(currentLink());
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
void QPdfPageNavigator::back()
{
    if (d->currentHistoryIndex <= 0)
        return;
    const bool backAvailableWas = backAvailable();
    const bool forwardAvailableWas = forwardAvailable();
    QPointF currentLocationWas = currentLocation();
    qreal currentZoomWas = currentZoom();
    --d->currentHistoryIndex;
    d->changing = true;
    emit jumped(currentLink());
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
    \property QPdfPageNavigator::currentPage

    This property holds the current page that is being viewed.
    The default is \c 0.
*/
int QPdfPageNavigator::currentPage() const
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.size())
        return -1; // only until ctor or clear() runs
    return d->pageHistory.at(d->currentHistoryIndex)->page;
}

/*!
    \property QPdfPageNavigator::currentLocation

    This property holds the current location on the page that is being viewed
    (the location that was last given to jump() or update()). The default is
    \c {0, 0}.
*/
QPointF QPdfPageNavigator::currentLocation() const
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.size())
        return QPointF();
    return d->pageHistory.at(d->currentHistoryIndex)->location;
}

/*!
    \property QPdfPageNavigator::currentZoom

    This property holds the magnification scale (1 logical pixel = 1 point)
    on the page that is being viewed. The default is \c 1.
*/
qreal QPdfPageNavigator::currentZoom() const
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.size())
        return 1;
    return d->pageHistory.at(d->currentHistoryIndex)->zoom;
}

QPdfLink QPdfPageNavigator::currentLink() const
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.size())
        return QPdfLink();
    return QPdfLink(d->pageHistory.at(d->currentHistoryIndex).data());
}

/*!
    Clear the history and restore \l currentPage, \l currentLocation and
    \l currentZoom to their default values.
*/
void QPdfPageNavigator::clear()
{
    d->pageHistory.clear();
    d->currentHistoryIndex = 0;
    // Begin with an implicit jump to page 0, so that
    // backAvailable() will become true after jump() is called one more time.
    d->pageHistory.append(QExplicitlySharedDataPointer<QPdfLinkPrivate>(new QPdfLinkPrivate(0, {}, 1)));
}

/*!
    Adds the given \a destination to the history of visited locations.

    In this case, PDF views respond to the \l jumped signal by scrolling to
    place \c destination.rectangles in the viewport, as opposed to placing
    \c destination.location in the viewport. So it's appropriate to call this
    method to jump to a search result from QPdfSearchModel (because the
    rectangles cover the region of text found). To jump to a hyperlink
    destination, call jump(page, location, zoom) instead, because in that
    case the QPdfLink object's \c rectangles cover the hyperlink origin
    location rather than the destination.
*/
void QPdfPageNavigator::jump(QPdfLink destination)
{
    const bool zoomChange = !qFuzzyCompare(destination.zoom(), currentZoom());
    const bool pageChange = (destination.page() != currentPage());
    const bool locationChange = (destination.location() != currentLocation());
    const bool backAvailableWas = backAvailable();
    const bool forwardAvailableWas = forwardAvailable();
    if (!d->changing) {
        if (d->currentHistoryIndex >= 0 && forwardAvailableWas)
            d->pageHistory.remove(d->currentHistoryIndex + 1, d->pageHistory.size() - d->currentHistoryIndex - 1);
        d->pageHistory.append(destination.d);
        d->currentHistoryIndex = d->pageHistory.size() - 1;
    }
    if (zoomChange)
        emit currentZoomChanged(currentZoom());
    if (pageChange)
        emit currentPageChanged(currentPage());
    if (locationChange)
        emit currentLocationChanged(currentLocation());
    if (d->changing)
        return;
    if (backAvailableWas != backAvailable())
        emit backAvailableChanged(backAvailable());
    if (forwardAvailableWas != forwardAvailable())
        emit forwardAvailableChanged(forwardAvailable());
    emit jumped(currentLink());
    qCDebug(qLcNav) << "push: index" << d->currentHistoryIndex << destination << "-> history" <<
        [this]() {
            QStringList ret;
            for (auto d : d->pageHistory)
                ret << QString::number(d->page);
            return ret.join(QLatin1Char(','));
        }();
}

/*!
    Adds the given destination, consisting of \a page, \a location, and \a zoom,
    to the history of visited locations.

    The \a zoom argument represents magnification (where \c 1 is the default
    scale, 1 logical pixel = 1 point). If \a zoom is not given or is \c 0,
    currentZoom keeps its existing value, and currentZoomChanged is not emitted.

    The \a location should be the same as QPdfLink::location() if the user is
    following a link; and since that is specified as the upper-left corner of
    the destination, it is best for consistency to always use the location
    visible in the upper-left corner of the viewport, in points.

    If forwardAvailable is \c true, calling this function represents a branch
    in the timeline which causes the "future" to be lost, and therefore
    forwardAvailable will change to \c false.
*/
void QPdfPageNavigator::jump(int page, const QPointF &location, qreal zoom)
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
            d->pageHistory.remove(d->currentHistoryIndex + 1, d->pageHistory.size() - d->currentHistoryIndex - 1);
        d->pageHistory.append(QExplicitlySharedDataPointer<QPdfLinkPrivate>(new QPdfLinkPrivate(page, location, zoom)));
        d->currentHistoryIndex = d->pageHistory.size() - 1;
    }
    if (zoomChange)
        emit currentZoomChanged(currentZoom());
    if (pageChange)
        emit currentPageChanged(currentPage());
    if (locationChange)
        emit currentLocationChanged(currentLocation());
    if (d->changing)
        return;
    if (backAvailableWas != backAvailable())
        emit backAvailableChanged(backAvailable());
    if (forwardAvailableWas != forwardAvailable())
        emit forwardAvailableChanged(forwardAvailable());
    emit jumped(currentLink());
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
void QPdfPageNavigator::update(int page, const QPointF &location, qreal zoom)
{
    if (d->currentHistoryIndex < 0 || d->currentHistoryIndex >= d->pageHistory.size())
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
    \property QPdfPageNavigator::backAvailable
    \readonly

    Holds \c true if a \e back destination is available in the history:
    that is, if push() or forward() has been called.
*/
bool QPdfPageNavigator::backAvailable() const
{
    return d->currentHistoryIndex > 0;
}

/*!
    \property QPdfPageNavigator::forwardAvailable
    \readonly

    Holds \c true if a \e forward destination is available in the history:
    that is, if back() has been previously called.
*/
bool QPdfPageNavigator::forwardAvailable() const
{
    return d->currentHistoryIndex < d->pageHistory.size() - 1;
}

/*!
    \fn void QPdfPageNavigator::jumped(QPdfLink current)

    This signal is emitted when an abrupt jump occurs, to the \a current
    page index, location on the page, and zoom level; but \e not when simply
    scrolling through the document one page at a time. That is, jump(),
    forward() and back() emit this signal, but update() does not.

    If \c {current.rectangles.length > 0}, they are rectangles that cover
    a specific destination area: a search result that should be made
    visible; otherwise, \c {current.location} is the destination location on
    the \c page (a hyperlink destination, or during forward/back navigation).
*/

QT_END_NAMESPACE

#include "moc_qpdfpagenavigator.cpp"
