/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
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

#include "qpdfview.h"
#include "qpdfview_p.h"

#include "qpdfpagerenderer.h"

#include <QGuiApplication>
#include <QPdfDocument>
#include <QPdfPageNavigation>
#include <QScreen>
#include <QScrollBar>
#include <QScroller>

QT_BEGIN_NAMESPACE

QPdfViewPrivate::QPdfViewPrivate()
    : QAbstractScrollAreaPrivate()
    , m_document(nullptr)
    , m_pageNavigation(nullptr)
    , m_pageRenderer(nullptr)
    , m_pageMode(QPdfView::SinglePage)
    , m_zoomMode(QPdfView::CustomZoom)
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

    m_pageNavigation = new QPdfPageNavigation(q);
    m_pageRenderer = new QPdfPageRenderer(q);
    m_pageRenderer->setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);
}

void QPdfViewPrivate::documentStatusChanged()
{
    updateDocumentLayout();
    invalidatePageCache();
}

void QPdfViewPrivate::currentPageChanged(int currentPage)
{
    Q_Q(QPdfView);

    if (m_blockPageScrolling)
        return;

    q->verticalScrollBar()->setValue(yPositionForPage(currentPage));

    if (m_pageMode == QPdfView::SinglePage)
        invalidateDocumentLayout();
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

        if (m_zoomMode != QPdfView::CustomZoom) {
            invalidatePageCache();
        }
    }

    if (m_pageMode == QPdfView::MultiPage) {
        // An imaginary, 2px height line at the upper half of the viewport, which is used to
        // determine which page is currently located there -> we propagate that as 'current' page
        // to the QPdfPageNavigation object
        const QRect currentPageLine(m_viewport.x(), m_viewport.y() + m_viewport.height() * 0.4, m_viewport.width(), 2);

        int currentPage = 0;
        for (auto it = m_documentLayout.pageGeometries.cbegin(); it != m_documentLayout.pageGeometries.cend(); ++it) {
            const QRect pageGeometry = it.value();
            if (pageGeometry.intersects(currentPageLine)) {
                currentPage = it.key();
                break;
            }
        }

        if (currentPage != m_pageNavigation->currentPage()) {
            m_blockPageScrolling = true;
            m_pageNavigation->setCurrentPage(currentPage);
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

void QPdfViewPrivate::pageRendered(int pageNumber, QSize imageSize, const QImage &image, quint64 requestId)
{
    Q_Q(QPdfView);

    Q_UNUSED(imageSize)
    Q_UNUSED(requestId)

    if (!m_cachedPagesLRU.contains(pageNumber)) {
        if (m_cachedPagesLRU.length() > m_pageCacheLimit)
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

    if (!m_document || m_document->status() != QPdfDocument::Ready)
        return documentLayout;

    QHash<int, QRect> pageGeometries;

    const int pageCount = m_document->pageCount();

    int totalWidth = 0;

    const int startPage = (m_pageMode == QPdfView::SinglePage ? m_pageNavigation->currentPage() : 0);
    const int endPage = (m_pageMode == QPdfView::SinglePage ? m_pageNavigation->currentPage() + 1 : pageCount);

    // calculate page sizes
    for (int page = startPage; page < endPage; ++page) {
        QSize pageSize;
        if (m_zoomMode == QPdfView::CustomZoom) {
            pageSize = QSizeF(m_document->pageSize(page) * m_screenResolution * m_zoomFactor).toSize();
        } else if (m_zoomMode == QPdfView::FitToWidth) {
            pageSize = QSizeF(m_document->pageSize(page) * m_screenResolution).toSize();
            const qreal factor = (qreal(m_viewport.width() - m_documentMargins.left() - m_documentMargins.right()) / qreal(pageSize.width()));
            pageSize *= factor;
        } else if (m_zoomMode == QPdfView::FitInView) {
            const QSize viewportSize(m_viewport.size() + QSize(-m_documentMargins.left() - m_documentMargins.right(), -m_pageSpacing));

            pageSize = QSizeF(m_document->pageSize(page) * m_screenResolution).toSize();
            pageSize = pageSize.scaled(viewportSize, Qt::KeepAspectRatio);
        }

        totalWidth = qMax(totalWidth, pageSize.width());

        pageGeometries[page] = QRect(QPoint(0, 0), pageSize);
    }

    totalWidth += m_documentMargins.left() + m_documentMargins.right();

    int pageY = m_documentMargins.top();

    // calculate page positions
    for (int page = startPage; page < endPage; ++page) {
        const QSize pageSize = pageGeometries[page].size();

        // center horizontal inside the viewport
        const int pageX = (qMax(totalWidth, m_viewport.width()) - pageSize.width()) / 2;

        pageGeometries[page].moveTopLeft(QPoint(pageX, pageY));

        pageY += pageSize.height() + m_pageSpacing;
    }

    pageY += m_documentMargins.bottom();

    documentLayout.pageGeometries = pageGeometries;

    // calculate overall document size
    documentLayout.documentSize = QSize(totalWidth, pageY);

    return documentLayout;
}

qreal QPdfViewPrivate::yPositionForPage(int pageNumber) const
{
    const auto it = m_documentLayout.pageGeometries.constFind(pageNumber);
    if (it == m_documentLayout.pageGeometries.cend())
        return 0.0;

    return (*it).y();
}

void QPdfViewPrivate::updateDocumentLayout()
{
    m_documentLayout = calculateDocumentLayout();

    updateScrollBars();
}


QPdfView::QPdfView(QWidget *parent)
    : QAbstractScrollArea(*new QPdfViewPrivate(), parent)
{
    Q_D(QPdfView);

    d->init();

    connect(d->m_pageNavigation, &QPdfPageNavigation::currentPageChanged, this, [d](int page){ d->currentPageChanged(page); });

    connect(d->m_pageRenderer, &QPdfPageRenderer::pageRendered,
            this, [d](int pageNumber, QSize imageSize, const QImage &image, QPdfDocumentRenderOptions, quint64 requestId){ d->pageRendered(pageNumber, imageSize, image, requestId); });

    verticalScrollBar()->setSingleStep(20);
    horizontalScrollBar()->setSingleStep(20);

    QScroller::grabGesture(this);

    d->calculateViewport();
}

/*!
  \internal
*/
QPdfView::QPdfView(QPdfViewPrivate &dd, QWidget *parent)
    : QAbstractScrollArea(dd, parent)
{
}

QPdfView::~QPdfView()
{
}

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
        d->m_documentStatusChangedConnection = connect(d->m_document.data(), &QPdfDocument::statusChanged, this, [d](){ d->documentStatusChanged(); });

    d->m_pageNavigation->setDocument(d->m_document);
    d->m_pageRenderer->setDocument(d->m_document);

    d->documentStatusChanged();
}

QPdfDocument *QPdfView::document() const
{
    Q_D(const QPdfView);

    return d->m_document;
}

QPdfPageNavigation *QPdfView::pageNavigation() const
{
    Q_D(const QPdfView);

    return d->m_pageNavigation;
}

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

    for (auto it = d->m_documentLayout.pageGeometries.cbegin(); it != d->m_documentLayout.pageGeometries.cend(); ++it) {
        const QRect pageGeometry = it.value();
        if (pageGeometry.intersects(d->m_viewport)) { // page needs to be painted
            painter.fillRect(pageGeometry, Qt::white);

            const int page = it.key();
            const auto pageIt = d->m_pageCache.constFind(page);
            if (pageIt != d->m_pageCache.cend()) {
                const QImage &img = pageIt.value();
                painter.drawImage(pageGeometry.topLeft(), img);
            } else {
                d->m_pageRenderer->requestPage(page, pageGeometry.size());
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

QT_END_NAMESPACE

#include "moc_qpdfview.cpp"
