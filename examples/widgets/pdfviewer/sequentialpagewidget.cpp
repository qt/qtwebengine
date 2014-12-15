#include "sequentialpagewidget.h"
#include "pagecache.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPdfDocument>
#include <QGuiApplication>
#include <QScreen>
#include <QLoggingCategory>
#include <QElapsedTimer>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

SequentialPageWidget::SequentialPageWidget(QWidget *parent)
    : QWidget(parent)
    , m_doc(Q_NULLPTR)
    , m_pageCache(Q_NULLPTR)
    , m_background(Qt::darkGray)
    , m_pageSpacing(3)
    , m_topPageShowing(0)
    , m_zoom(1.)
    , m_screenResolution(QGuiApplication::primaryScreen()->logicalDotsPerInch() / 72.0)
{
}

SequentialPageWidget::~SequentialPageWidget()
{
}

void SequentialPageWidget::setDocument(QPdfDocument *doc)
{
    m_doc = doc;
    m_topPageShowing = 0;
    if (m_pageCache)
        delete m_pageCache;
    m_pageCache = new PageCache(doc, m_screenResolution * m_zoom);
    connect(m_pageCache, SIGNAL(pageReady(int)), this, SLOT(update()));
    invalidate();
}

void SequentialPageWidget::setZoom(qreal factor)
{
    m_zoom = factor;
    emit zoomChanged(factor);
    invalidate();
}

QSizeF SequentialPageWidget::pageSize(int page)
{
    return m_doc->pageSize(page) * m_screenResolution * m_zoom;
}

void SequentialPageWidget::invalidate()
{
    if (m_pageCache)
        delete m_pageCache;
    if (m_doc)
        m_pageCache = new PageCache(m_doc, m_screenResolution * m_zoom);
    connect(m_pageCache, SIGNAL(pageReady(int)), this, SLOT(update()));
    QSizeF totalSize(0, m_pageSpacing);
    for (int page = 0; page < m_doc->pageCount(); ++page) {
        QSizeF size = pageSize(page);
        qDebug() << "page" << page << "size" << size;
        totalSize.setHeight(totalSize.height() + size.height());
        if (size.width() > totalSize.width())
            totalSize.setWidth(size.width());
    }
    m_totalSize = totalSize.toSize();
    setMinimumSize(m_totalSize);
    emit zoomChanged(m_zoom);
    qCDebug(lcExample) << "total size" << m_totalSize;
    update();
}

void SequentialPageWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    if (!m_doc) {
        painter.drawText(rect(), Qt::AlignCenter, tr("no document loaded"));
        return;
    }

    painter.fillRect(event->rect(), m_background);

    // Find the first page that needs to be rendered
    int page = 0;
    int y = 0;
    while (page < m_doc->pageCount()) {
        QSizeF size = pageSize(page);
        int height = size.toSize().height();
        if (y + height >= event->rect().top())
            break;
        y += height + m_pageSpacing;
        ++page;
    }
    y += m_pageSpacing;
    m_topPageShowing = page;
    if (!m_pageCache) return;

    // Actually render pages
    while (y < event->rect().bottom() && page < m_doc->pageCount()) {
        const QPixmap &pm = m_pageCache->get(page);
        painter.drawPixmap((width() - pm.width()) / 2, y, pm);
        y += pm.height() + m_pageSpacing;
        ++page;
    }
    m_bottomPageShowing = page - 1;
    emit showingPageRange(m_topPageShowing, m_bottomPageShowing);
}

qreal SequentialPageWidget::yForPage(int endPage)
{
    // TODO maybe put this loop into a page iterator class
    int y = m_pageSpacing;
    for (int page = 0; page < m_doc->pageCount() && page < endPage; ++page) {
        QSizeF size = pageSize(page);
        int height = size.toSize().height();
        y += height + m_pageSpacing;
    }
    return y;
}
