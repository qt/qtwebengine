#include "pagerenderer.h"
#include <QPainter>
#include <QPdfDocument>
#include <QLoggingCategory>
#include <QElapsedTimer>
#include <QUrl>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

PageRenderer::PageRenderer()
    : QThread(Q_NULLPTR)
    , m_doc(new QPdfDocument(this))
    , m_page(0)
    , m_zoom(1.)
    , m_minRenderTime(1000000000.)
    , m_maxRenderTime(0.)
    , m_totalRenderTime(0.)
    , m_totalPagesRendered(0)
{
}

PageRenderer::~PageRenderer()
{
}

QVector<QSizeF> PageRenderer::openDocument(const QUrl &location)
{
    if (location.isLocalFile())
        m_doc.load(location.toLocalFile());
    else {
        qCWarning(lcExample, "non-local file loading is not implemented");
        return QVector<QSizeF>();
    }
    // TODO maybe do in run() if it takes too long
    QVector<QSizeF> pageSizes;
    for (int page = 0; page < m_doc.pageCount(); ++page)
        pageSizes.append(m_doc.pageSize(page));
    return pageSizes;
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
    QSizeF size = m_doc.pageSize(page) * m_zoom;
    QElapsedTimer timer; timer.start();
    const QImage &img = m_doc.render(page, size);
    qreal secs = timer.nsecsElapsed() / 1000000000.0;
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
