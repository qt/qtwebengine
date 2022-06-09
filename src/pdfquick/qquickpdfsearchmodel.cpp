// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickpdfsearchmodel_p.h"
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcSearch, "qt.pdf.search")

/*!
    \qmltype PdfSearchModel
//!    \instantiates QQuickPdfSearchModel
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

/*!
    \internal
*/
QQuickPdfSearchModel::~QQuickPdfSearchModel() = default;

QQuickPdfDocument *QQuickPdfSearchModel::document() const
{
    return m_quickDocument;
}

void QQuickPdfSearchModel::setDocument(QQuickPdfDocument *document)
{
    if (document == m_quickDocument || !document)
        return;

    m_quickDocument = document;
    QPdfSearchModel::setDocument(document->document());
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

    It becomes empty whenever \c {currentPage != currentResultLink.page}.

    \sa PathMultiline
*/
QList<QPolygonF> QQuickPdfSearchModel::currentResultBoundingPolygons() const
{
    QList<QPolygonF> ret;
    const auto result = currentResultLink();
    if (result.page() != m_currentPage)
        return ret;
    for (auto rect : result.rectangles())
        ret << QPolygonF(rect);
    return ret;
}

/*!
    \qmlproperty point PdfSearchModel::currentResultBoundingRect

    The bounding box containing all \l currentResultBoundingPolygons,
    if \c {currentPage == currentResultLink.page}; otherwise, an invalid rectangle.
*/
QRectF QQuickPdfSearchModel::currentResultBoundingRect() const
{
    QRectF ret;
    const auto result = currentResultLink();
    if (result.page() != m_currentPage)
        return ret;
    auto rects = result.rectangles();
    if (!rects.isEmpty()) {
        ret = rects.takeFirst();
        for (auto rect : rects)
            ret = ret.united(rect);
    }
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
QList<QPolygonF> QQuickPdfSearchModel::currentPageBoundingPolygons() const
{
    return const_cast<QQuickPdfSearchModel *>(this)->boundingPolygonsOnPage(m_currentPage);
}

/*!
    \qmlmethod list<list<point>> PdfSearchModel::boundingPolygonsOnPage(int page)

    Returns a set of paths in a form that can be bound to the \c paths property of a
    \l {QtQuick::PathMultiline}{PathMultiline} instance, which is used to render a
    batch of rectangles around all the matching locations on the \a page:

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
QList<QPolygonF> QQuickPdfSearchModel::boundingPolygonsOnPage(int page)
{
    if (!document() || searchString().isEmpty() || page < 0 || page > document()->document()->pageCount())
        return {};

    updatePage(page);

    QList<QPolygonF> ret;
    const auto m = QPdfSearchModel::resultsOnPage(page);
    for (const auto &result : m) {
        for (const auto &rect : result.rectangles())
            ret << QPolygonF(rect);
    }

    return ret;
}

/*!
    \qmlproperty int PdfSearchModel::currentPage

    The page on which \l currentResultBoundingPolygons should provide filtered
    search results.
*/
void QQuickPdfSearchModel::setCurrentPage(int currentPage)
{
    if (m_currentPage == currentPage)
        return;

    const auto pageCount = document()->document()->pageCount();
    if (currentPage < 0)
        currentPage = pageCount - 1;
    else if (currentPage >= pageCount)
        currentPage = 0;

    m_currentPage = currentPage;
    if (!m_suspendSignals) {
        emit currentPageChanged();
        onResultsChanged();
    }
}

/*!
    \qmlproperty int PdfSearchModel::currentResult

    The result index within the whole set of search results, for which
    \l currentResultBoundingPolygons should provide the regions to highlight
    if currentPage matches \c currentResultLink.page.
*/
void QQuickPdfSearchModel::setCurrentResult(int currentResult)
{
    if (m_currentResult == currentResult)
        return;

    const int currentResultWas = m_currentResult;
    const int currentPageWas = m_currentPage;
    const int resultCount = rowCount({});

    // wrap around at the ends
    if (currentResult >= resultCount) {
        currentResult = 0;
    } else if (currentResult < 0) {
        currentResult = resultCount - 1;
    }

    const QPdfLink link = resultAtIndex(currentResult);
    if (link.isValid()) {
        setCurrentPage(link.page());
        m_currentResult = currentResult;
        emit currentResultChanged();
        emit currentResultLinkChanged();
        emit currentResultBoundingPolygonsChanged();
        emit currentResultBoundingRectChanged();
        qCDebug(qLcSearch) << "currentResult was" << currentResultWas
                      << "requested" << currentResult << "on page" << currentPageWas
                      << "->" << m_currentResult << "on page" << m_currentPage;
    } else {
        qWarning() << "failed to find result" << currentResult << "in range 0 ->" << resultCount;
    }
}

/*!
    \qmlproperty QPdfLink PdfSearchModel::currentResultLink

    The result at index \l currentResult.
*/
QPdfLink QQuickPdfSearchModel::currentResultLink() const
{
    return resultAtIndex(m_currentResult);
}

/*!
    \qmlproperty string PdfSearchModel::searchString

    The string to search for.
*/

QT_END_NAMESPACE

#include "moc_qquickpdfsearchmodel_p.cpp"
