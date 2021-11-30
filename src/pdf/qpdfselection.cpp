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
QPdfSelection::QPdfSelection(const QString &text, QList<QPolygonF> bounds, QRectF boundingRect, int startIndex, int endIndex)
  : d(new QPdfSelectionPrivate(text, bounds, boundingRect, startIndex, endIndex))
{
}

QPdfSelection::QPdfSelection(QPdfSelectionPrivate *d)
  : d(d)
{
}

QPdfSelection::~QPdfSelection() = default;
QPdfSelection::QPdfSelection(const QPdfSelection &other) = default;
QPdfSelection::QPdfSelection(QPdfSelection &&other) noexcept = default;
QPdfSelection &QPdfSelection::operator=(const QPdfSelection &other) = default;

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
QList<QPolygonF> QPdfSelection::bounds() const
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
    \property QPdfSelection::boundingRectangle

    This property holds the overall bounding rectangle (convex hull) around \l bounds.
*/
QRectF QPdfSelection::boundingRectangle() const
{
    return d->boundingRect;
}

/*!
    \property QPdfSelection::startIndex

    This property holds the index at the beginning of \l text within the full text on the page.
*/
int QPdfSelection::startIndex() const
{
    return d->startIndex;
}

/*!
    \property QPdfSelection::endIndex

    This property holds the index at the end of \l text within the full text on the page.
*/
int QPdfSelection::endIndex() const
{
    return d->endIndex;
}

#if QT_CONFIG(clipboard)
/*!
    Copies \l text to the \l {QGuiApplication::clipboard()}{system clipboard}
    depending on the \a mode selected.
*/
void QPdfSelection::copyToClipboard(QClipboard::Mode mode) const
{
    QGuiApplication::clipboard()->setText(d->text, mode);
}
#endif

QT_END_NAMESPACE

#include "moc_qpdfselection.cpp"
