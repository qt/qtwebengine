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
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcS, "qt.pdf.search")

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
    connect(this, &QPdfSearchModel::searchStringChanged,
            this, &QQuickPdfSearchModel::onResultsChanged);
}

QQuickPdfDocument *QQuickPdfSearchModel::document() const
{
    return m_quickDocument;
}

void QQuickPdfSearchModel::setDocument(QQuickPdfDocument *document)
{
    if (document == m_quickDocument || !document)
        return;

    m_quickDocument = document;
    QPdfSearchModel::setDocument(&document->m_doc);
}

/*!
    \qmlproperty list<list<point>> PdfSearchModel::currentResultBoundingPolygons

    A set of paths in a form that can be bound to the \c paths property of a
    \l {QtQuick::PathMultiline}{PathMultiline} instance to render a batch of
    rectangles around the regions comprising the search result \l currentResult
    on \l currentPage.  This is normally used to highlight one search result
    at a time, in a UI that allows stepping through the results:

    \qml
    PdfDocument {
        id: doc
    }
    PdfSearchModel {
        id: searchModel
        document: doc
        currentPage: view.currentPage
        currentResult: ...
    }
    Shape {
        ShapePath {
            PathMultiline {
                paths: searchModel.currentResultBoundingPolygons
            }
        }
    }
    \endqml

    \sa PathMultiline
*/
QVector<QPolygonF> QQuickPdfSearchModel::currentResultBoundingPolygons() const
{
    QVector<QPolygonF> ret;
    const auto &results = const_cast<QQuickPdfSearchModel *>(this)->resultsOnPage(m_currentPage);
    if (m_currentResult < 0 || m_currentResult >= results.count())
        return ret;
    const auto result = results[m_currentResult];
    for (auto rect : result.rectangles())
        ret << QPolygonF(rect);
    return ret;
}

/*!
    \qmlproperty point PdfSearchModel::currentResultBoundingRect

    The bounding box containing all \l currentResultBoundingPolygons.

    When this property changes, a scrollable view should automatically scroll
    itself in such a way as to ensure that this region is visible; for example,
    it could try to position the upper-left corner near the upper-left of its
    own viewport, subject to the constraints of the scrollable area.
*/
QRectF QQuickPdfSearchModel::currentResultBoundingRect() const
{
    QRectF ret;
    const auto &results = const_cast<QQuickPdfSearchModel *>(this)->resultsOnPage(m_currentPage);
    if (m_currentResult < 0 || m_currentResult >= results.count())
        return ret;
    auto rects = results[m_currentResult].rectangles();
    ret = rects.takeFirst();
    for (auto rect : rects)
        ret = ret.united(rect);
    return ret;
}

void QQuickPdfSearchModel::onResultsChanged()
{
    emit currentPageBoundingPolygonsChanged();
    emit currentResultBoundingPolygonsChanged();
}

/*!
    \qmlproperty list<list<point>> PdfSearchModel::currentPageBoundingPolygons

    A set of paths in a form that can be bound to the \c paths property of a
    \l {QtQuick::PathMultiline}{PathMultiline} instance to render a batch of
    rectangles around all the regions where search results are found on
    \l currentPage:

    \qml
    PdfDocument {
        id: doc
    }
    PdfSearchModel {
        id: searchModel
        document: doc
    }
    Shape {
        ShapePath {
            PathMultiline {
                paths: searchModel.matchGeometry(view.currentPage)
            }
        }
    }
    \endqml

    \sa PathMultiline
*/
QVector<QPolygonF> QQuickPdfSearchModel::currentPageBoundingPolygons() const
{
    return const_cast<QQuickPdfSearchModel *>(this)->boundingPolygonsOnPage(m_currentPage);
}

/*!
    \qmlfunction list<list<point>> PdfSearchModel::boundingPolygonsOnPage(int page)

    Returns a set of paths in a form that can be bound to the \c paths property of a
    \l {QtQuick::PathMultiline}{PathMultiline} instance to render a batch of
    rectangles around all the locations where search results are found:

    \qml
    PdfDocument {
        id: doc
    }
    PdfSearchModel {
        id: searchModel
        document: doc
    }
    Shape {
        ShapePath {
            PathMultiline {
                paths: searchModel.matchGeometry(view.currentPage)
            }
        }
    }
    \endqml

    \sa PathMultiline
*/
QVector<QPolygonF> QQuickPdfSearchModel::boundingPolygonsOnPage(int page)
{
    if (!document() || searchString().isEmpty() || page < 0 || page > document()->pageCount())
        return {};

    updatePage(page);

    QVector<QPolygonF> ret;
    auto m = QPdfSearchModel::resultsOnPage(page);
    for (auto result : m) {
        for (auto rect : result.rectangles())
            ret << QPolygonF(rect);
    }

    return ret;
}

/*!
    \qmlproperty int PdfSearchModel::currentPage

    The page on which \l currentMatchGeometry should provide filtered search results.
*/
void QQuickPdfSearchModel::setCurrentPage(int currentPage)
{
    if (m_currentPage == currentPage)
        return;

    if (currentPage < 0)
        currentPage = document()->pageCount() - 1;
    else if (currentPage >= document()->pageCount())
        currentPage = 0;

    m_currentPage = currentPage;
    if (!m_suspendSignals) {
        emit currentPageChanged();
        onResultsChanged();
    }
}

/*!
    \qmlproperty int PdfSearchModel::currentResult

    The result index on \l currentPage for which \l currentResultBoundingPolygons
    should provide the regions to highlight.
*/
void QQuickPdfSearchModel::setCurrentResult(int currentResult)
{
    if (m_currentResult == currentResult)
        return;

    int currentResultWas = currentResult;
    int currentPageWas = m_currentPage;
    if (currentResult < 0) {
        setCurrentPage(m_currentPage - 1);
        while (resultsOnPage(m_currentPage).count() == 0 && m_currentPage != currentPageWas) {
            m_suspendSignals = true;
            setCurrentPage(m_currentPage - 1);
        }
        if (m_suspendSignals) {
            emit currentPageChanged();
            m_suspendSignals = false;
        }
        const auto results = resultsOnPage(m_currentPage);
        currentResult = results.count() - 1;
    } else {
        const auto results = resultsOnPage(m_currentPage);
        if (currentResult >= results.count()) {
            setCurrentPage(m_currentPage + 1);
            while (resultsOnPage(m_currentPage).count() == 0 && m_currentPage != currentPageWas) {
                m_suspendSignals = true;
                setCurrentPage(m_currentPage + 1);
            }
            if (m_suspendSignals) {
                emit currentPageChanged();
                m_suspendSignals = false;
            }
            currentResult = 0;
        }
    }
    qCDebug(qLcS) << "currentResult was" << m_currentResult
                  << "requested" << currentResultWas << "on page" << currentPageWas
                  << "->" << currentResult << "on page" << m_currentPage;

    m_currentResult = currentResult;
    emit currentResultChanged();
    emit currentResultBoundingPolygonsChanged();
    emit currentResultBoundingRectChanged();
}

/*!
    \qmlproperty string PdfSearchModel::searchString

    The string to search for.
*/

QT_END_NAMESPACE
