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

#include "qpdfdestination.h"
#include "qpdfdestination_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QPdfDestination
    \since 5.15
    \inmodule QtPdf

    \brief The QPdfDestination class defines a location on a page in a PDF
    document, and a suggested zoom level at which it is intended to be viewed.
*/

/*!
    Constructs an invalid Destination.

    \sa valid
*/
QPdfDestination::QPdfDestination()
  : d(new QPdfDestinationPrivate())
{
}

QPdfDestination::QPdfDestination(int page, QPointF location, qreal zoom)
  : d(new QPdfDestinationPrivate(page, location, zoom))
{
}

QPdfDestination::QPdfDestination(QPdfDestinationPrivate *d)
  : d(d)
{
}

QPdfDestination::QPdfDestination(const QPdfDestination &other)
  : d(other.d)
{
}

QPdfDestination::QPdfDestination(QPdfDestination &&other) noexcept
  : d(std::move(other.d))
{
}

QPdfDestination::~QPdfDestination()
{
}

QPdfDestination &QPdfDestination::operator=(const QPdfDestination &other)
{
    d = other.d;
    return *this;
}

/*!
    \property QPdfDestination::valid

    This property holds whether the destination is valid.
*/
bool QPdfDestination::isValid() const
{
    return d->page >= 0;
}

/*!
    \property QPdfDestination::page

    This property holds the page number.
*/
int QPdfDestination::page() const
{
    return d->page;
}

/*!
    \property QPdfDestination::location

    This property holds the location on the page, in units of points.
*/
QPointF QPdfDestination::location() const
{
    return d->location;
}

/*!
    \property QPdfDestination::zoom

    This property holds the suggested magnification level, where 1.0 means default scale
    (1 pixel = 1 point).
*/
qreal QPdfDestination::zoom() const
{
    return d->zoom;
}

QDebug operator<<(QDebug dbg, const QPdfDestination& dest)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "QPdfDestination(page=" << dest.page()
        << " location=" << dest.location()
        << " zoom=" << dest.zoom();
    dbg << ')';
    return dbg;
}

QT_END_NAMESPACE

#include "moc_qpdfdestination.cpp"
