#include "pagecache.h"
#include <QPainter>
#include <QPdfDocument>
#include <QLoggingCategory>
#include <QElapsedTimer>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

PageCache::PageCache(QPdfDocument *doc, qreal zoom)
    : QThread(Q_NULLPTR)
    , m_doc(doc)
    , m_zoom(zoom)
    , m_placeholderIcon(":icons/images/busy.png")
    , m_placeholderBackground(Qt::white)
    , m_minRenderTime(1000000000.)
    , m_maxRenderTime(0.)
    , m_totalRenderTime(0.)
    , m_totalPagesRendered(0)
{
}

PageCache::~PageCache()
{
}

/*!
   Get the result from the cache,
   or a placeholder pixmap if unavailable.
 */
QPixmap PageCache::get(int page)
{
    m_lastPageRequested = page;
    if (m_pageCache.contains(page))
        return m_pageCache[page];
    if (!isRunning())
        start(QThread::LowestPriority);
    QSizeF sizeF = m_doc->pageSize(page);
    if (!sizeF.isValid())
        return QPixmap();
    QSize size = QSizeF(sizeF * m_zoom).toSize();
    QPixmap placeholder(size);
    {
        QPainter painter(&placeholder);
        painter.fillRect(QRect(QPoint(), size), m_placeholderBackground);
        painter.drawPixmap((size.width() - m_placeholderIcon.width()) / 2,
            (size.height() - m_placeholderIcon.height()) / 2, m_placeholderIcon);
    }
    return placeholder;
}

void PageCache::run()
{
    int lastPageRequested = m_lastPageRequested;
    int forward = lastPageRequested;
    int backward = lastPageRequested - 1;
    while (true) {
        bool done = true;
        if (lastPageRequested != m_lastPageRequested) {
            lastPageRequested = m_lastPageRequested;
            forward = lastPageRequested;
            backward = lastPageRequested - 1;
        }
        if (forward < m_doc->pageCount()) {
            if (!m_pageCache.contains(forward))
                insertPage(forward);
            done = false;
        }
        if (backward >= 0) {
            if (!m_pageCache.contains(backward))
                insertPage(backward);
            done = false;
        }
        ++forward;
        --backward;
        if (done)
            return;
    }
}

void PageCache::insertPage(int page)
{
    if (!m_pageCache.contains(page)) {
        QSizeF size = m_doc->pageSize(page) * m_zoom;
        QElapsedTimer timer; timer.start();
        const QImage &img = m_doc->render(page, size);
        qreal secs = timer.nsecsElapsed() / 1000000000.0;
        if (secs < m_minRenderTime)
            m_minRenderTime = secs;
        if (secs > m_maxRenderTime)
            m_maxRenderTime = secs;
        m_totalRenderTime += secs;
        ++m_totalPagesRendered;
        m_pageCache.insert(page, QPixmap::fromImage(img));
        emit pageReady(page);

        qCDebug(lcExample) << "page" << page << "zoom" << m_zoom << "size" << size << "in" << secs <<
                              "secs; min" << m_minRenderTime <<
                              "avg" << m_totalRenderTime / m_totalPagesRendered <<
                              "max" << m_maxRenderTime;
    }
}
