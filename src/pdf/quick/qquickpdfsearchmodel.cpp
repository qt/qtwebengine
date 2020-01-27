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

#include "qquickpdfsearchmodel_p.h"
#include <QQuickItem>
#include <QQmlEngine>
#include <QStandardPaths>
#include <private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PdfSearchModel
    \instantiates QQuickPdfSearchModel
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A representation of text search results within a PDF Document.
    \since 5.15

    PdfSearchModel provides the ability to search for text strings within a
    document and get the geometric locations of matches on each page.
*/

QQuickPdfSearchModel::QQuickPdfSearchModel(QObject *parent)
    : QPdfSearchModel(parent)
{
}

QQuickPdfDocument *QQuickPdfSearchModel::document() const
{
    return m_quickDocument;
}

void QQuickPdfSearchModel::setDocument(QQuickPdfDocument *document)
{
    if (document == m_quickDocument)
        return;
    m_quickDocument = document;
    QPdfSearchModel::setDocument(&document->m_doc);
}

/*!
    \qmlproperty list<list<point>> PdfSearchModel::matchGeometry

    A set of paths in a form that can be bound to the \c paths property of a
    \l {QtQuick::PathMultiline}{PathMultiline} instance to render a batch of
    rectangles around all the locations where search results are found:

    \qml
    PdfDocument {
        id: doc
    }
    PdfSearchModel {
        id: searchModel
        document: doc
        page: doc.currentPage
    }
    Shape {
        ShapePath {
            PathMultiline {
                paths: searchModel.matchGeometry
            }
        }
    }
    \endqml

    \sa PathMultiline
*/
QVector<QPolygonF> QQuickPdfSearchModel::matchGeometry() const
{
    return m_matchGeometry;
}

/*!
    \qmlproperty string PdfSearchModel::searchString

    The string to search for.
*/
QString QQuickPdfSearchModel::searchString() const
{
    return m_searchString;
}

void QQuickPdfSearchModel::setSearchString(QString searchString)
{
    if (m_searchString == searchString)
        return;

    m_searchString = searchString;
    emit searchStringChanged();
    updateResults();
}

/*!
    \qmlproperty int PdfSearchModel::page

    The page number on which to search.

    \sa QtQuick::Image::currentFrame
*/
int QQuickPdfSearchModel::page() const
{
    return m_page;
}

void QQuickPdfSearchModel::setPage(int page)
{
    if (m_page == page)
        return;

    m_page = page;
    emit pageChanged();
    updateResults();
}

void QQuickPdfSearchModel::updateResults()
{
    if (!document() || (m_searchString.isEmpty() && !m_matchGeometry.isEmpty()) || m_page < 0 || m_page > document()->pageCount()) {
        m_matchGeometry.clear();
        emit matchGeometryChanged();
    }
    QVector<QRectF> m = QPdfSearchModel::matches(m_page, m_searchString);
    QVector<QPolygonF> matches;
    for (QRectF r : m)
        matches << QPolygonF(r);
    if (matches != m_matchGeometry) {
        m_matchGeometry = matches;
        emit matchGeometryChanged();
    }
}

QT_END_NAMESPACE
