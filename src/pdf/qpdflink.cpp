// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdflink.h"
#include "qpdflink_p.h"
#include "qpdflinkmodel_p.h"
#include <QGuiApplication>
#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
    \class QPdfLink
    \since 6.4
    \inmodule QtPdf

    \brief The QPdfLink class defines a link between a region on a page
    (such as a hyperlink or a search result) and a destination
    (page, location on the page, and zoom level at which to view it).
*/

/*!
    Constructs an invalid Destination.

    \sa valid
*/
QPdfLink::QPdfLink() :
    QPdfLink(new QPdfLinkPrivate()) { }

QPdfLink::QPdfLink(int page, QPointF location, qreal zoom)
  : QPdfLink(new QPdfLinkPrivate(page, location, zoom))
{
}

QPdfLink::QPdfLink(int page, QList<QRectF> rects,
                                   QString contextBefore, QString contextAfter)
    : QPdfLink(new QPdfLinkPrivate(page, std::move(rects),
                                                   std::move(contextBefore),
                                                   std::move(contextAfter)))
{
}

QPdfLink::QPdfLink(QPdfLinkPrivate *d) : d(d) {}

QPdfLink::~QPdfLink() = default;
QPdfLink::QPdfLink(const QPdfLink &other) noexcept = default;
QPdfLink::QPdfLink(QPdfLink &&other) noexcept = default;
QPdfLink &QPdfLink::operator=(const QPdfLink &other) = default;

/*!
    \property QPdfLink::valid

    This property holds whether the link is valid.
*/
bool QPdfLink::isValid() const
{
    return d->page >= 0;
}

/*!
    \property QPdfLink::page

    This property holds the page number.
    If the link is a search result, it is the page number on which the result is found;
    if the link is a hyperlink, it is the destination page number.
*/
int QPdfLink::page() const
{
    return d->page;
}

/*!
    \property QPdfLink::location

    This property holds the location on the \l page, in units of points.
    If the link is a search result, it is the location where the result is found;
    if the link is a hyperlink, it is the destination location.
*/
QPointF QPdfLink::location() const
{
    return d->location;
}

/*!
    \property QPdfLink::zoom

    This property holds the suggested magnification level, where 1.0 means default scale
    (1 pixel = 1 point). If the link is a search result, this value is not used.
*/
qreal QPdfLink::zoom() const
{
    return d->zoom;
}

/*!
    \property QPdfLink::url

    This property holds the destination URL if the link is an external hyperlink;
    otherwise, it's empty.
*/
QUrl QPdfLink::url() const
{
    return d->url;
}

/*!
    \property QPdfLink::contextBefore

    This property holds adjacent text found on the page before the search string.
    If the link is a hyperlink, this string is empty.

    \sa QPdfSearchModel::resultsOnPage(), QPdfSearchModel::resultAtIndex()
*/
QString QPdfLink::contextBefore() const
{
    return d->contextBefore;
}

/*!
    \property QPdfLink::contextAfter

    This property holds adjacent text found on the page after the search string.
    If the link is a hyperlink, this string is empty.

    \sa QPdfSearchModel::resultsOnPage(), QPdfSearchModel::resultAtIndex()
*/
QString QPdfLink::contextAfter() const
{
    return d->contextAfter;
}

/*!
    \property QPdfLink::rectangles

    This property holds the region (set of rectangles) occupied by the link or
    search result on the page where it was found. If the text wraps around to
    multiple lines on the page, there may be multiple rectangles:

    \image wrapping-search-result.png

    \sa QPdfSearchModel::resultsOnPage(), QPdfSearchModel::resultAtIndex()
*/
QList<QRectF> QPdfLink::rectangles() const
{
    return d->rects;
}

/*!
    Returns a translated representation for display.

    \sa copyToClipboard()
*/
QString QPdfLink::toString() const
{
    if (d->page <= 0)
         return d->url.toString();
    return QPdfLinkModel::tr("Page %1 location %2, %3 zoom %4")
        .arg(d->page).arg(d->location.x(), 0, 'f', 1).arg(d->location.y(), 0, 'f', 1)
        .arg(d->zoom, 0, 'f', 0);
}

/*!
    Copies the toString() representation of the link to the
    \l {QGuiApplication::clipboard()}{system clipboard} depending on the \a mode given.
*/
void QPdfLink::copyToClipboard(QClipboard::Mode mode) const
{
    QGuiApplication::clipboard()->setText(toString(), mode);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QPdfLink &link)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "QPdfLink(page=" << link.page()
        << " location=" << link.location()
        << " zoom=" << link.zoom()
        << " contextBefore=" << link.contextBefore()
        << " contextAfter=" << link.contextAfter()
        << " rects=" << link.rectangles();
    dbg << ')';
    return dbg;
}
#endif

QT_END_NAMESPACE

#include "moc_qpdflink.cpp"
