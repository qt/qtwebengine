// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
