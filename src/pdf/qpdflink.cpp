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

#include "qpdflink.h"
#include "qpdflink_p.h"

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

QT_END_NAMESPACE

#include "moc_qpdflink.cpp"
