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

#include "qquickpdfselection_p.h"
#include "qquickpdfdocument_p.h"
#include <QClipboard>
#include <QQuickItem>
#include <QQmlEngine>
#include <QStandardPaths>
#include <private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PdfSelection
    \instantiates QQuickPdfSelection
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A representation of a text selection within a PDF Document.
    \since 5.15

    PdfSelection provides the text string and its geometry within a bounding box
    from one point to another.
*/

/*!
    Constructs a SearchModel.
*/
QQuickPdfSelection::QQuickPdfSelection(QObject *parent)
    : QObject(parent)
{
}

QQuickPdfDocument *QQuickPdfSelection::document() const
{
    return m_document;
}

void QQuickPdfSelection::setDocument(QQuickPdfDocument *document)
{
    if (m_document == document)
        return;

    if (m_document) {
        disconnect(m_document, &QQuickPdfDocument::sourceChanged,
                   this, &QQuickPdfSelection::resetPoints);
    }
    m_document = document;
    emit documentChanged();
    resetPoints();
    connect(m_document, &QQuickPdfDocument::sourceChanged,
            this, &QQuickPdfSelection::resetPoints);
}

/*!
    \qmlproperty list<list<point>> PdfSelection::geometry

    A set of paths in a form that can be bound to the \c paths property of a
    \l {QtQuick::PathMultiline}{PathMultiline} instance to render a batch of
    rectangles around the text regions that are included in the selection:

    \qml
    PdfDocument {
        id: doc
    }
    PdfSelection {
        id: selection
        document: doc
        fromPoint: textSelectionDrag.centroid.pressPosition
        toPoint: textSelectionDrag.centroid.position
        hold: !textSelectionDrag.active
    }
    Shape {
        ShapePath {
            PathMultiline {
                paths: selection.geometry
            }
        }
    }
    DragHandler {
        id: textSelectionDrag
        acceptedDevices: PointerDevice.Mouse | PointerDevice.Stylus
        target: null
    }
    \endqml

    \sa PathMultiline
*/
QVector<QPolygonF> QQuickPdfSelection::geometry() const
{
    return m_geometry;
}

void QQuickPdfSelection::resetPoints()
{
    bool wasHolding = m_hold;
    m_hold = false;
    setFromPoint(QPointF());
    setToPoint(QPointF());
    m_hold = wasHolding;
}

/*!
    \qmlproperty int PdfSelection::page

    The page number on which to search.

    \sa QtQuick::Image::currentFrame
*/
int QQuickPdfSelection::page() const
{
    return m_page;
}

void QQuickPdfSelection::setPage(int page)
{
    if (m_page == page)
        return;

    m_page = page;
    emit pageChanged();
    resetPoints();
}

/*!
    \qmlproperty point PdfSelection::fromPoint

    The beginning location, in \l {https://en.wikipedia.org/wiki/Point_(typography)}{points}
    from the upper-left corner of the page, from which to find selected text.
    This can be bound to a scaled version of the \c centroid.pressPosition
    of a \l DragHandler to begin selecting text from the position where the user
    presses the mouse button and begins dragging, for example.
*/
QPointF QQuickPdfSelection::fromPoint() const
{
    return m_fromPoint;
}

void QQuickPdfSelection::setFromPoint(QPointF fromPoint)
{
    if (m_hold || m_fromPoint == fromPoint)
        return;

    m_fromPoint = fromPoint;
    emit fromPointChanged();
    updateResults();
}

/*!
    \qmlproperty point PdfSelection::toPoint

    The ending location, in \l {https://en.wikipedia.org/wiki/Point_(typography)}{points}
    from the upper-left corner of the page, from which to find selected text.
    This can be bound to a scaled version of the \c centroid.position
    of a \l DragHandler to end selection of text at the position where the user
    is currently dragging the mouse, for example.
*/
QPointF QQuickPdfSelection::toPoint() const
{
    return m_toPoint;
}

void QQuickPdfSelection::setToPoint(QPointF toPoint)
{
    if (m_hold || m_toPoint == toPoint)
        return;

    m_toPoint = toPoint;
    emit toPointChanged();
    updateResults();
}

/*!
    \qmlproperty bool PdfSelection::hold

    Controls whether to hold the existing selection regardless of changes to
    \l fromPoint and \l toPoint. This property can be set to \c true when the mouse
    or touchpoint is released, so that the selection is not lost due to the
    point bindings changing.
*/
bool QQuickPdfSelection::hold() const
{
    return m_hold;
}

void QQuickPdfSelection::setHold(bool hold)
{
    if (m_hold == hold)
        return;

    m_hold = hold;
    emit holdChanged();
}

/*!
    \qmlproperty string PdfSelection::string

    The string found.
*/
QString QQuickPdfSelection::text() const
{
    return m_text;
}

#if QT_CONFIG(clipboard)
/*!
    \qmlmethod void PdfSelection::copyToClipboard()

    Copies plain text from the \l string property to the system clipboard.
*/
void QQuickPdfSelection::copyToClipboard() const
{
     QGuiApplication::clipboard()->setText(m_text);
}
#endif

void QQuickPdfSelection::updateResults()
{
    if (!m_document)
        return;
    QPdfSelection sel = m_document->document().getSelection(m_page, m_fromPoint, m_toPoint);
    if (sel.text() != m_text) {
        m_text = sel.text();
        if (QGuiApplication::clipboard()->supportsSelection())
            sel.copyToClipboard(QClipboard::Selection);
        emit textChanged();
    }

    if (sel.bounds() != m_geometry) {
        m_geometry = sel.bounds();
        emit geometryChanged();
    }
}

QT_END_NAMESPACE
