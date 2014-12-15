#include "sequentialpagewidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPdfDocument>
#include <QGuiApplication>
#include <QScreen>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

SequentialPageWidget::SequentialPageWidget(QWidget *parent)
    : QWidget(parent)
    , m_doc(Q_NULLPTR)
    , m_background(Qt::darkGray)
    , m_pageSpacing(3)
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
    invalidate();
}

void SequentialPageWidget::setZoom(qreal factor)
{
    m_zoom = factor;
    invalidate();
}

QSizeF SequentialPageWidget::pageSize(int page)
{
    return m_doc->pageSize(page) * m_screenResolution * m_zoom;
}

void SequentialPageWidget::invalidate()
{
    m_pageCache.clear();
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
        y += height;
        ++page;
    }
    y += m_pageSpacing;
    int startPage = page;

    // Actually render pages
    while (y < event->rect().bottom() && page < m_doc->pageCount()) {
        if (!m_pageCache.contains(page)) {
            QSizeF size = pageSize(page);
            qCDebug(lcExample) << "rendering page" << page << "to size" << size;
            m_pageCache.insert(page, QPixmap::fromImage(m_doc->render(page, size)));
        }
        const QPixmap &pm = m_pageCache[page];
        painter.drawPixmap((width() - pm.width()) / 2, y, pm);
        y += pm.height() + m_pageSpacing;
        ++page;
    }

    emit showingPageRange(startPage, page - 1);
}
