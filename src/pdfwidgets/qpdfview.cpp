// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdfview.h"
#include "qpdfview_p.h"

#include <QtPdf/qpdflink.h>
#include "qpdfpagerenderer.h"
#include <QtPdf/private/qtpdfglobal_p.h>

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QPainter>
#include <QPaintEvent>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfSearchModel>
#include <QScreen>
#include <QScrollBar>

QT_BEGIN_NAMESPACE

Q_PDF_LOGGING_CATEGORY(qLcWLink, "qt.pdf.widgets.links")
//#define DEBUG_LINKS

static const QColor SearchResultHighlight("#80B0C4DE");
static const QColor CurrentSearchResultHighlight(Qt::cyan);
static const int CurrentSearchResultWidth(2);
// minimum margin between jump destination and viewport edge (as in QScrollArea::ensureVisible)
static const int MinimumScrollMargin(50);

QPdfViewPrivate::QPdfViewPrivate(QPdfView *q)
    : q_ptr(q)
    , m_document(nullptr)
    , m_pageNavigator(nullptr)
    , m_pageRenderer(nullptr)
    , m_pageMode(QPdfView::PageMode::SinglePage)
    , m_zoomMode(QPdfView::ZoomMode::Custom)
    , m_zoomFactor(1.0)
    , m_pageSpacing(3)
    , m_documentMargins(6, 6, 6, 6)
    , m_blockPageScrolling(false)
    , m_pageCacheLimit(20)
    , m_screenResolution(QGuiApplication::primaryScreen()->logicalDotsPerInch() / 72.0)
{
}

void QPdfViewPrivate::init()
{
    Q_Q(QPdfView);

    m_pageNavigator = new QPdfPageNavigator(q);
    m_pageRenderer = new QPdfPageRenderer(q);
    m_pageRenderer->setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);
}

void QPdfViewPrivate::documentStatusChanged(QPdfDocument::Status status)
{
    if (status == QPdfDocument::Status::Ready)
        updateDocumentLayout();
    invalidatePageCache();
}

void QPdfViewPrivate::currentPageChanged(int currentPage)
{
    Q_Q(QPdfView);

    if (m_blockPageScrolling)
        return;

    q->verticalScrollBar()->setValue(yPositionForPage(currentPage));

    if (m_pageMode == QPdfView::PageMode::SinglePage)
        invalidateDocumentLayout();
}

void QPdfViewPrivate::currentZoomChanged(qreal currentZoom)
{
    Q_Q(QPdfView);
    // zero means a destination (e.g. search result) doesn't set zoom
    if (currentZoom > 0)
        q->setZoomFactor(currentZoom);
}

void QPdfViewPrivate::calculateViewport()
{
    Q_Q(QPdfView);

    const int x = q->horizontalScrollBar()->value();
    const int y = q->verticalScrollBar()->value();
    const int width = q->viewport()->width();
    const int height = q->viewport()->height();

    setViewport(QRect(x, y, width, height));
}

void QPdfViewPrivate::setViewport(QRect viewport)
{
    if (m_viewport == viewport)
        return;

    const QSize oldSize = m_viewport.size();

    m_viewport = viewport;

    if (oldSize != m_viewport.size()) {
        updateDocumentLayout();

        if (m_zoomMode != QPdfView::ZoomMode::Custom) {
            invalidatePageCache();
        }
    }

    if (m_pageMode == QPdfView::PageMode::MultiPage) {
        // An imaginary, 2px height line at the upper half of the viewport, which is used to
        // determine which page is currently located there -> we propagate that as 'current' page
        // to the QPdfPageNavigator object
        const QRect currentPageLine(m_viewport.x(), m_viewport.y() + m_viewport.height() * 0.4, m_viewport.width(), 2);

        int currentPage = 0;
        for (auto it = m_documentLayout.pageGeometryAndScale.cbegin();
             it != m_documentLayout.pageGeometryAndScale.cend(); ++it) {
            const QRect pageGeometry = it.value().first;
            if (pageGeometry.intersects(currentPageLine)) {
                currentPage = it.key();
                break;
            }
        }

        if (currentPage != m_pageNavigator->currentPage()) {
            m_blockPageScrolling = true;
            // ΤODO give location on the page
            m_pageNavigator->jump(currentPage, {}, m_zoomFactor);
            m_blockPageScrolling = false;
        }
    }
}

void QPdfViewPrivate::updateScrollBars()
{
    Q_Q(QPdfView);

    const QSize p = q->viewport()->size();
    const QSize v = m_documentLayout.documentSize;

    q->horizontalScrollBar()->setRange(0, v.width() - p.width());
    q->horizontalScrollBar()->setPageStep(p.width());
    q->verticalScrollBar()->setRange(0, v.height() - p.height());
    q->verticalScrollBar()->setPageStep(p.height());
}

/*! \internal
    Scroll lazily (the minimal distance) such that \a link's destination
    becomes completely visible within the viewport.
*/
void QPdfViewPrivate::scrollTo(const QPdfLink &link)
{
    Q_Q(QPdfView);

    // A search result's location is the corner of its first rectangle, and then
    // the whole rectangle is worth showing. A link's location is its
    // destination, which has nothing to do with its rectangles: those are the
    // area to click on, on the page the link is on. Some links have neither a
    // location nor rectangles, and then there is nowhere in particular to go.
    const QList<QRectF> rects = link.rectangles();
    QRectF onPage;
    if (!rects.isEmpty() && rects.constFirst().topLeft() == link.location())
        onPage = rects.constFirst();
    else if (!link.location().isNull())
        onPage = QRectF(link.location(), QSizeF(1, 1));
    else if (!rects.isEmpty())
        onPage = rects.constFirst();
    else
        return;

    // destination in the document: same transform and page position as in paintEvent()
    const QRect pageGeometry = m_documentLayout.pageGeometryAndScale.value(link.page()).first;
    const QRect target = screenScaleTransform(link.page()).mapRect(onPage)
                            .translated(pageGeometry.topLeft()).toAlignedRect();
    const QRect oldViewport = m_viewport;

    const int margin = qMin(MinimumScrollMargin,
                            qMin(m_viewport.width(), m_viewport.height()) / 4);
    const QRect wanted = target.marginsAdded(QMargins(margin, margin, margin, margin));

    if (m_viewport.contains(wanted))
        return;

    // Scroll the shortest distance that brings wanted into view; if it does not fit,
    // show the beginning. right() and bottom() are the last pixel inside the rectangle; the
    // viewport covers m_viewport.width() pixels starting at the scrollbar's value, hence the + 1.
    int sx = m_viewport.x();
    if (wanted.left() < m_viewport.left())
        sx = wanted.left();
    else if (wanted.right() > m_viewport.right())
        sx = qMin(wanted.left(), wanted.right() - m_viewport.width() + 1);
    int sy = m_viewport.y();
    if (wanted.top() < m_viewport.top())
        sy = wanted.top();
    else if (wanted.bottom() > m_viewport.bottom())
        sy = qMin(wanted.top(), wanted.bottom() - m_viewport.height() + 1);

    q->horizontalScrollBar()->setValue(sx);
    q->verticalScrollBar()->setValue(sy);

    qCDebug(qLcWLink) << "scrolled to page" << link.page() << "@" << link.location()
                      << "target region" << target << "scrollbars" << sx << sy
                      << "viewport" << oldViewport << "->" << m_viewport;
}

void QPdfViewPrivate::jumped(const QPdfLink &link)
{
    // When the current page changes because the user scrolled, the navigator is
    // told about it, with no location: there is nothing to scroll to then.
    if (m_blockPageScrolling)
        return;
    scrollTo(link);
}

void QPdfViewPrivate::pageRendered(int pageNumber, QSize imageSize, const QImage &image, quint64 requestId)
{
    Q_Q(QPdfView);

    Q_UNUSED(imageSize);
    Q_UNUSED(requestId);

    if (!m_cachedPagesLRU.contains(pageNumber)) {
        if (m_cachedPagesLRU.size() > m_pageCacheLimit)
            m_pageCache.remove(m_cachedPagesLRU.takeFirst());

        m_cachedPagesLRU.append(pageNumber);
    }

    m_pageCache.insert(pageNumber, image);

    q->viewport()->update();
}

void QPdfViewPrivate::invalidateDocumentLayout()
{
    updateDocumentLayout();
    invalidatePageCache();
}

void QPdfViewPrivate::invalidatePageCache()
{
    Q_Q(QPdfView);

    m_pageCache.clear();
    q->viewport()->update();
}

QPdfViewPrivate::DocumentLayout QPdfViewPrivate::calculateDocumentLayout() const
{
    // The DocumentLayout describes a virtual layout where all pages are positioned inside
    //    - For SinglePage mode, this is just an area as large as the current page surrounded
    //      by the m_documentMargins.
    //    - For MultiPage mode, this is the area that is covered by all pages which are placed
    //      below each other, with m_pageSpacing inbetween and surrounded by m_documentMargins

    DocumentLayout documentLayout;

    if (!m_document || m_document->status() != QPdfDocument::Status::Ready)
        return documentLayout;

    QHash<int, QPair<QRect, qreal>> pageGeometryAndScale;

    const int pageCount = m_document->pageCount();

    int totalWidth = 0;

    const int startPage = (m_pageMode == QPdfView::PageMode::SinglePage ? m_pageNavigator->currentPage() : 0);
    const int endPage = (m_pageMode == QPdfView::PageMode::SinglePage ? m_pageNavigator->currentPage() + 1 : pageCount);

    // calculate page sizes
    for (int page = startPage; page < endPage; ++page) {
        QSize pageSize;
        qreal pageScale = m_zoomFactor;
        if (m_zoomMode == QPdfView::ZoomMode::Custom) {
            pageSize = QSizeF(m_document->pagePointSize(page) * m_screenResolution * m_zoomFactor).toSize();
        } else if (m_zoomMode == QPdfView::ZoomMode::FitToWidth) {
            pageSize = QSizeF(m_document->pagePointSize(page) * m_screenResolution).toSize();
            pageScale = (qreal(m_viewport.width() - m_documentMargins.left() - m_documentMargins.right()) /
                         qreal(pageSize.width()));
            pageSize *= pageScale;
        } else if (m_zoomMode == QPdfView::ZoomMode::FitInView) {
            const QSize viewportSize(m_viewport.size() +
                                     QSize(-m_documentMargins.left() - m_documentMargins.right(), -m_pageSpacing));

            pageSize = QSizeF(m_document->pagePointSize(page) * m_screenResolution).toSize();
            QSize scaledSize = pageSize.scaled(viewportSize, Qt::KeepAspectRatio);
            // because of KeepAspectRatio, the ratio of widths should be the same as the ratio of heights
            pageScale = qreal(scaledSize.width()) / qreal(pageSize.width());
            pageSize = scaledSize;
        }

        totalWidth = qMax(totalWidth, pageSize.width());

        pageGeometryAndScale[page] = {QRect(QPoint(0, 0), pageSize), pageScale};
    }

    totalWidth += m_documentMargins.left() + m_documentMargins.right();

    int pageY = m_documentMargins.top();

    // calculate page positions
    for (int page = startPage; page < endPage; ++page) {
        const QSize pageSize = pageGeometryAndScale[page].first.size();

        // center horizontal inside the viewport
        const int pageX = (qMax(totalWidth, m_viewport.width()) - pageSize.width()) / 2;

        pageGeometryAndScale[page].first.moveTopLeft(QPoint(pageX, pageY));

        pageY += pageSize.height() + m_pageSpacing;
    }

    pageY += m_documentMargins.bottom();

    documentLayout.pageGeometryAndScale = pageGeometryAndScale;

    // calculate overall document size
    documentLayout.documentSize = QSize(totalWidth, pageY);

    return documentLayout;
}

qreal QPdfViewPrivate::yPositionForPage(int pageNumber) const
{
    const auto it = m_documentLayout.pageGeometryAndScale.constFind(pageNumber);
    if (it == m_documentLayout.pageGeometryAndScale.cend())
        return 0.0;

    return (*it).first.y();
}

QPdfLink QPdfViewPrivate::pagePosition(const QPointF &viewPosition)
{
    for (auto it = m_documentLayout.pageGeometryAndScale.cbegin();
         it != m_documentLayout.pageGeometryAndScale.cend(); ++it) {
        const int page = it.key();
        const QTransform screenInvTransform = screenScaleTransform(page).inverted();
        const QRect pageGeometry = it.value().first;
        if (pageGeometry.contains(viewPosition.toPoint())) {
            QPointF posInPoints = screenInvTransform.map(viewPosition - pageGeometry.topLeft()
                                                         + m_viewport.topLeft());
            return QPdfLink(page, posInPoints, 1);
        }
    }
    return {}; // invalid: not found
}

QTransform QPdfViewPrivate::screenScaleTransform(int page) const
{
    qreal scale = m_screenResolution * m_zoomFactor;
    switch (m_zoomMode) {
    case QPdfView::ZoomMode::FitToWidth:
    case QPdfView::ZoomMode::FitInView:
        scale = m_screenResolution * m_documentLayout.pageGeometryAndScale[page].second;
        break;
    default:
        break;
    }

    return QTransform::fromScale(scale, scale);
}

void QPdfViewPrivate::updateDocumentLayout()
{
    m_documentLayout = calculateDocumentLayout();

    updateScrollBars();
}

/*!
    \class QPdfView
    \inmodule QtPdf
    \brief A PDF viewer widget.

    QPdfView is a PDF viewer widget that offers a user experience similar to
    many common PDF viewer applications, with two \l {pageMode}{modes}.
    In the \c MultiPage mode, it supports flicking through the pages in the
    entire document, with narrow gaps between the page images.
    In the \c SinglePage mode, it shows one page at a time.
*/

/*!
    Constructs a PDF viewer with parent widget \a parent.
*/
QPdfView::QPdfView(QWidget *parent)
    : QAbstractScrollArea(parent)
    , d_ptr(new QPdfViewPrivate(this))
{
    Q_D(QPdfView);

    d->init();

    connect(d->m_pageNavigator, &QPdfPageNavigator::currentPageChanged, this,
            [d](int page){ d->currentPageChanged(page); });
    connect(d->m_pageNavigator, &QPdfPageNavigator::jumped, this,
            [d](const QPdfLink &link){ d->jumped(link); });
    connect(d->m_pageNavigator, &QPdfPageNavigator::currentZoomChanged, this,
            [d](qreal zoom){ d->currentZoomChanged(zoom); });

    connect(d->m_pageRenderer, &QPdfPageRenderer::pageRendered, this,
            [d](int pageNumber, QSize imageSize, const QImage &image, QPdfDocumentRenderOptions, quint64 requestId) {
                d->pageRendered(pageNumber, imageSize, image, requestId); });

    verticalScrollBar()->setSingleStep(20);
    horizontalScrollBar()->setSingleStep(20);

    setMouseTracking(true);
    d->calculateViewport();
}

/*!
    Destroys the PDF viewer.
*/
QPdfView::~QPdfView()
{
    Q_D(QPdfView);
    if (d->m_document)
        disconnect(d->m_documentStatusChangedConnection);
}

/*!
    \property QPdfView::document

    This property holds the document to be viewed.
*/
void QPdfView::setDocument(QPdfDocument *document)
{
    Q_D(QPdfView);

    if (d->m_document == document)
        return;

    if (d->m_document)
        disconnect(d->m_documentStatusChangedConnection);

    d->m_document = document;
    emit documentChanged(d->m_document);

    if (d->m_document)
        d->m_documentStatusChangedConnection =
                connect(d->m_document.data(), &QPdfDocument::statusChanged, this,
                        [d](QPdfDocument::Status s){ d->documentStatusChanged(s); });

    d->m_pageRenderer->setDocument(d->m_document);
    d->m_linkModel.setDocument(d->m_document);

    const auto docStatus = document ? document->status() : QPdfDocument::Status::Null;
    d->documentStatusChanged(docStatus);
}

QPdfDocument *QPdfView::document() const
{
    Q_D(const QPdfView);

    return d->m_document;
}

/*!
    \since 6.6
    \property QPdfView::searchModel

    If this property is set, QPdfView draws highlight rectangles over the
    search results provided by \l QPdfSearchModel::resultsOnPage(). By default
    it is \c nullptr.
*/
void QPdfView::setSearchModel(QPdfSearchModel *searchModel)
{
    Q_D(QPdfView);
    if (d->m_searchModel == searchModel)
        return;

    if (d->m_searchModel)
        d->m_searchModel->disconnect(this);

    d->m_searchModel = searchModel;
    emit searchModelChanged(searchModel);

    if (searchModel) {
        connect(searchModel, &QPdfSearchModel::dataChanged, this,
                [this](const QModelIndex &, const QModelIndex &, const QList<int> &) { update(); });
    }
    setCurrentSearchResultIndex(-1);
}

QPdfSearchModel *QPdfView::searchModel() const
{
    Q_D(const QPdfView);
    return d->m_searchModel;
}

/*!
    \since 6.6
    \property QPdfView::currentSearchResultIndex

    If this property is set to a positive number, and \l searchModel is set,
    QPdfView draws a frame around the search result provided by
    \l QPdfSearchModel at the given index. For example, if QPdfSearchModel is
    used as the model for a QListView, you can keep this property updated by
    connecting QItemSelectionModel::currentChanged() from
    QListView::selectionModel() to a function that will in turn call this function.

    By default it is \c -1, so that no search results are framed.
*/
void QPdfView::setCurrentSearchResultIndex(int currentResult)
{
    Q_D(QPdfView);
    if (d->m_currentSearchResultIndex == currentResult)
        return;

    d->m_currentSearchResultIndex = currentResult;
    emit currentSearchResultIndexChanged(currentResult);
    viewport()->update(); //update();
}

int QPdfView::currentSearchResultIndex() const
{
    Q_D(const QPdfView);
    return d->m_currentSearchResultIndex;
}

/*!
    This accessor returns the navigation stack that will handle back/forward navigation.
*/
QPdfPageNavigator *QPdfView::pageNavigator() const
{
    Q_D(const QPdfView);

    return d->m_pageNavigator;
}

/*!
    \enum QPdfView::PageMode

    This enum describes the overall behavior of the PDF viewer:

    \value SinglePage   Show one page at a time.
    \value MultiPage    Allow scrolling through all pages in the document.
*/

/*!
    \property QPdfView::pageMode

    This property holds whether to show one page at a time, or all pages in the
    document. The default is \c SinglePage.
*/
QPdfView::PageMode QPdfView::pageMode() const
{
    Q_D(const QPdfView);

    return d->m_pageMode;
}

void QPdfView::setPageMode(PageMode mode)
{
    Q_D(QPdfView);

    if (d->m_pageMode == mode)
        return;

    d->m_pageMode = mode;
    d->invalidateDocumentLayout();

    emit pageModeChanged(d->m_pageMode);
}

/*!
    \enum QPdfView::ZoomMode

    This enum describes the magnification behavior of the PDF viewer:

    \value Custom       Use \l zoomFactor only.
    \value FitToWidth   Automatically choose a zoom factor so that
                        the width of the page fits in the view.
    \value FitInView    Automatically choose a zoom factor so that
                        the entire page fits in the view.
*/

/*!
    \property QPdfView::zoomMode

    This property indicates whether to use a custom size for the page(s),
    or zoom them to fit to the view. The default is \c CustomZoom.
*/
QPdfView::ZoomMode QPdfView::zoomMode() const
{
    Q_D(const QPdfView);

    return d->m_zoomMode;
}

void QPdfView::setZoomMode(ZoomMode mode)
{
    Q_D(QPdfView);

    if (d->m_zoomMode == mode)
        return;

    d->m_zoomMode = mode;
    d->invalidateDocumentLayout();

    emit zoomModeChanged(d->m_zoomMode);
}

/*!
    \property QPdfView::zoomFactor

    This property holds the ratio of pixels to points. The default is \c 1,
    meaning one point (1/72 of an inch) equals 1 logical pixel.
*/
qreal QPdfView::zoomFactor() const
{
    Q_D(const QPdfView);

    return d->m_zoomFactor;
}

void QPdfView::setZoomFactor(qreal factor)
{
    Q_D(QPdfView);

    if (d->m_zoomFactor == factor)
        return;

    d->m_zoomFactor = factor;
    d->invalidateDocumentLayout();

    emit zoomFactorChanged(d->m_zoomFactor);
}

/*!
    \property QPdfView::pageSpacing

    This property holds the size of the padding between pages in the \l MultiPage
    \l {pageMode}{mode}.
*/
int QPdfView::pageSpacing() const
{
    Q_D(const QPdfView);

    return d->m_pageSpacing;
}

void QPdfView::setPageSpacing(int spacing)
{
    Q_D(QPdfView);

    if (d->m_pageSpacing == spacing)
        return;

    d->m_pageSpacing = spacing;
    d->invalidateDocumentLayout();

    emit pageSpacingChanged(d->m_pageSpacing);
}

/*!
    \property QPdfView::documentMargins

    This property holds the margins around the page view.
*/
QMargins QPdfView::documentMargins() const
{
    Q_D(const QPdfView);

    return d->m_documentMargins;
}

void QPdfView::setDocumentMargins(QMargins margins)
{
    Q_D(QPdfView);

    if (d->m_documentMargins == margins)
        return;

    d->m_documentMargins = margins;
    d->invalidateDocumentLayout();

    emit documentMarginsChanged(d->m_documentMargins);
}

void QPdfView::paintEvent(QPaintEvent *event)
{
    Q_D(QPdfView);

    QPainter painter(viewport());
    painter.fillRect(event->rect(), palette().brush(QPalette::Dark));
    painter.translate(-d->m_viewport.x(), -d->m_viewport.y());

    for (auto it = d->m_documentLayout.pageGeometryAndScale.cbegin();
         it != d->m_documentLayout.pageGeometryAndScale.cend(); ++it) {
        const QRect pageGeometry = it.value().first;
        if (pageGeometry.intersects(d->m_viewport)) { // page needs to be painted
            painter.fillRect(pageGeometry, Qt::white);

            const int page = it.key();
            const auto pageIt = d->m_pageCache.constFind(page);
            if (pageIt != d->m_pageCache.cend()) {
                const QImage &img = pageIt.value();
                painter.drawImage(pageGeometry, img);
            } else {
                d->m_pageRenderer->requestPage(page, pageGeometry.size() * devicePixelRatioF());
            }

            const QTransform scaleTransform = d->screenScaleTransform(page);
#ifdef DEBUG_LINKS
            const QString fmt = u"page %1 @ %2, %3"_s;
            d->m_linkModel.setPage(page);
            const int linkCount = d->m_linkModel.rowCount({});
            for (int i = 0; i < linkCount; ++i) {
                const QRectF linkBounds = scaleTransform.mapRect(
                            d->m_linkModel.data(d->m_linkModel.index(i),
                                                int(QPdfLinkModel::Role::Rect)).toRectF())
                        .translated(pageGeometry.topLeft());
                painter.setPen(Qt::blue);
                painter.drawRect(linkBounds);
                painter.setPen(Qt::red);
                const QPoint loc = d->m_linkModel.data(d->m_linkModel.index(i),
                                                       int(QPdfLinkModel::Role::Location)).toPoint();
                // TODO maybe draw destination URL if that's what it is
                painter.drawText(linkBounds.bottomLeft() + QPoint(2, -2),
                                 fmt.arg(d->m_linkModel.data(d->m_linkModel.index(i),
                                                             int(QPdfLinkModel::Role::Page)).toInt())
                                 .arg(loc.x()).arg(loc.y()));
            }
#endif
            if (d->m_searchModel) {
                // Don't cover the text below
                const auto w = CurrentSearchResultWidth / 2;
                QMargins highlightRectOffset(w, w, w, w);

                for (const QPdfLink &result : d->m_searchModel->resultsOnPage(page)) {
                    for (const QRectF &rect : result.rectangles()) {
                        const auto r = rect.marginsAdded(highlightRectOffset);
                        painter.fillRect(scaleTransform.mapRect(r).translated(pageGeometry.topLeft()), SearchResultHighlight);
                    }
                }

                if (d->m_currentSearchResultIndex >= 0 && d->m_currentSearchResultIndex < d->m_searchModel->rowCount({})) {
                    const QPdfLink &cur = d->m_searchModel->resultAtIndex(d->m_currentSearchResultIndex);
                    if (cur.page() == page) {
                        painter.setPen({CurrentSearchResultHighlight, CurrentSearchResultWidth});
                        for (const auto &rect : cur.rectangles()) {
                            const auto r = rect.marginsAdded(highlightRectOffset);
                            painter.drawRect(scaleTransform.mapRect(r).translated(pageGeometry.topLeft()));
                        }
                    }
                }
            }
        }
    }
}

void QPdfView::resizeEvent(QResizeEvent *event)
{
    Q_D(QPdfView);

    QAbstractScrollArea::resizeEvent(event);

    d->updateScrollBars();
    d->calculateViewport();
}

void QPdfView::scrollContentsBy(int dx, int dy)
{
    Q_D(QPdfView);

    QAbstractScrollArea::scrollContentsBy(dx, dy);

    d->calculateViewport();
}

void QPdfView::mousePressEvent(QMouseEvent *event)
{
    Q_ASSERT(event->isAccepted());
}

void QPdfView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QPdfView);
    auto pagePos = d->pagePosition(event->position());
    if (pagePos.isValid()) {
        d->m_linkModel.setPage(pagePos.page());
        auto dest = d->m_linkModel.linkAt(pagePos.location());
        setCursor(dest.isValid() ? Qt::PointingHandCursor : Qt::ArrowCursor);
        if (dest.isValid())
            qCDebug(qLcWLink) << event->position() << ":" << pagePos.location() << "pt ->" << dest;
    }
}

void QPdfView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QPdfView);
    auto pagePos = d->pagePosition(event->position());
    if (pagePos.isValid()) {
        d->m_linkModel.setPage(pagePos.page());
        auto dest = d->m_linkModel.linkAt(pagePos.location());
        if (dest.isValid()) {
            qCDebug(qLcWLink) << event << ": jumping to" << dest;
            d->m_pageNavigator->jump(dest.page(), dest.location(), dest.zoom());
        }
    }
}

void QPdfView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();

        if (delta == 0) {
            event->ignore();
            return;
        }

        Q_D(QPdfView);
        auto calculateZoomStep = [](qreal currentZoomFactor, int sign) {
            constexpr qreal baseStep = 0.1;
            constexpr qreal ceilStep = 0.9;

            const auto factor = std::ceil(currentZoomFactor + sign * baseStep);
            return qMin(baseStep * factor, ceilStep);
        };
        qreal currentZoom = zoomFactor();
        qreal stepSize = calculateZoomStep(currentZoom, (delta > 0) ? +1 : -1);

        qreal zoomChange = (delta > 0) ? stepSize : -stepSize;
        qreal newZoomFactor = qBound(0.1, currentZoom + zoomChange, 10.0);
        const auto cursorPageAndPos = d->pagePosition(event->position());

        setZoomFactor(newZoomFactor);

        // After zoom, scroll so as to keep the same document position under the cursor
        if (cursorPageAndPos.isValid()) {
            const QRect newPageGeometry = d->m_documentLayout.pageGeometryAndScale.value(cursorPageAndPos.page()).first;
            const QPointF newPosInDoc = newPageGeometry.topLeft() +
                    d->screenScaleTransform(cursorPageAndPos.page()).map(cursorPageAndPos.location());
            horizontalScrollBar()->setValue(qRound(newPosInDoc.x() - event->position().x()));
            verticalScrollBar()->setValue(qRound(newPosInDoc.y() - event->position().y()));
        }

        event->accept();
    } else {
        QAbstractScrollArea::wheelEvent(event);
    }
}

QT_END_NAMESPACE

#include "moc_qpdfview.cpp"
