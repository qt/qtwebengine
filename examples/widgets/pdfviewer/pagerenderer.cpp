/******************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt PDF Module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "pagerenderer.h"

#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QPainter>
#include <QPdfDocument>
#include <QUrl>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

PageRenderer::PageRenderer(QObject *parent)
    : QThread(parent)
    , m_document(nullptr)
    , m_page(0)
    , m_zoom(1.)
    , m_minRenderTime(1000000000.)
    , m_maxRenderTime(0.)
    , m_totalRenderTime(0.)
    , m_totalPagesRendered(0)
{
}

void PageRenderer::setDocument(QPdfDocument *document)
{
    m_document = document;
}

void PageRenderer::requestPage(int page, qreal zoom, Priority priority)
{
    // TODO maybe queue up the requests
    m_page = page;
    m_zoom = zoom;
    start(priority);
}

void PageRenderer::run()
{
    renderPage(m_page, m_zoom);
}

void PageRenderer::renderPage(int page, qreal zoom)
{
    if (!m_document || m_document->status() != QPdfDocument::Ready)
        return;

    const QSizeF size = m_document->pageSize(page) * m_zoom;

    QElapsedTimer timer;
    timer.start();

    const QImage &img = m_document->render(page, size);

    const qreal secs = timer.nsecsElapsed() / 1000000000.0;
    if (secs < m_minRenderTime)
        m_minRenderTime = secs;

    if (secs > m_maxRenderTime)
        m_maxRenderTime = secs;

    m_totalRenderTime += secs;
    ++m_totalPagesRendered;

    emit pageReady(page, zoom, img);

    qCDebug(lcExample) << "page" << page << "zoom" << m_zoom << "size" << size << "in" << secs <<
                          "secs; min" << m_minRenderTime <<
                          "avg" << m_totalRenderTime / m_totalPagesRendered <<
                          "max" << m_maxRenderTime;
}
