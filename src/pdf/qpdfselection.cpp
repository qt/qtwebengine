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

#include "qpdfselection.h"
#include "qpdfselection_p.h"
#include <QGuiApplication>

QT_BEGIN_NAMESPACE

/*!
    \class QPdfSelection
    \since 5.15
    \inmodule QtPdf

    \brief The QPdfSelection class defines a range of text that has been selected
    on one page in a PDF document, and its geometric boundaries.

    \sa QPdfDocument::getSelection()
*/

/*!
    Constructs an invalid selection.

    \sa valid
*/
QPdfSelection::QPdfSelection()
  : d(new QPdfSelectionPrivate())
{
}

/*!
    \internal
    Constructs a selection including the range of characters that make up the
    \a text string, and which take up space on the page within the polygon
    regions given in \a bounds.
*/
QPdfSelection::QPdfSelection(const QString &text, QVector<QPolygonF> bounds, QRectF boundingRect, int startIndex, int endIndex)
  : d(new QPdfSelectionPrivate(text, bounds, boundingRect, startIndex, endIndex))
{
}

QPdfSelection::QPdfSelection(QPdfSelectionPrivate *d)
  : d(d)
{
}

QPdfSelection::QPdfSelection(const QPdfSelection &other)
  : d(other.d)
{
}

QPdfSelection::QPdfSelection(QPdfSelection &&other) noexcept
  : d(std::move(other.d))
{
}

QPdfSelection::~QPdfSelection()
{
}

QPdfSelection &QPdfSelection::operator=(const QPdfSelection &other)
{
    d = other.d;
    return *this;
}

/*!
    \property QPdfSelection::valid

    This property holds whether the selection is valid.
*/
bool QPdfSelection::isValid() const
{
    return !d->bounds.isEmpty();
}

/*!
    \property QPdfSelection::bounds

    This property holds a set of regions that the selected text occupies on the
    page, represented as polygons. The coordinate system for the polygons has
    the origin at the upper-left corner of the page, and the units are
    \l {https://en.wikipedia.org/wiki/Point_(typography)}{points}.

    \note For now, the polygons returned from \l QPdfDocument::getSelection()
    are always rectangles; but in the future it may be possible to represent
    more complex regions.
*/
QVector<QPolygonF> QPdfSelection::bounds() const
{
    return d->bounds;
}

/*!
    \property QPdfSelection::text

    This property holds the selected text.
*/
QString QPdfSelection::text() const
{
    return d->text;
}

/*!
    \property rect QPdfSelection::boundingRectangle

    This property holds the overall bounding rectangle (convex hull) around \l bounds.
*/
QRectF QPdfSelection::boundingRectangle() const
{
    return d->boundingRect;
}

/*!
    \property int QPdfSelection::startIndex

    This property holds the index at the beginning of \l text within the full text on the page.
*/
int QPdfSelection::startIndex() const
{
    return d->startIndex;
}

/*!
    \property int QPdfSelection::endIndex

    This property holds the index at the end of \l text within the full text on the page.
*/
int QPdfSelection::endIndex() const
{
    return d->endIndex;
}

#if QT_CONFIG(clipboard)
/*!
    Copies \l text to the \l {QGuiApplication::clipboard()}{system clipboard}.
*/
void QPdfSelection::copyToClipboard(QClipboard::Mode mode) const
{
    QGuiApplication::clipboard()->setText(d->text, mode);
}
#endif

QT_END_NAMESPACE

#include "moc_qpdfselection.cpp"
