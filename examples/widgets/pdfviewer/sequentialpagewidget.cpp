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
    , m_top(0.)
    , m_screenResolution(QGuiApplication::primaryScreen()->logicalDotsPerInch() / 72.0)
{
}

SequentialPageWidget::~SequentialPageWidget()
{

}

void SequentialPageWidget::setDocument(QPdfDocument *doc)
{
    m_doc = doc;
}

void SequentialPageWidget::setZoom(qreal factor)
{
    m_zoom = factor;
    m_pageCache.clear();
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
    qCDebug(lcExample) << event->rect() << m_doc->pageCount();

    int page = 0; // qFloor(m_top);
    int y = 0; // TODO event->rect().top();
    while (y < event->rect().bottom() && page < m_doc->pageCount()) {
        QSizeF size = m_doc->pageSize(page) * m_screenResolution * m_zoom;
        if (!m_pageCache.contains(page)) {
            qCDebug(lcExample) << "rendering page" << page << "to size" << size;
            m_pageCache.insert(page, QPixmap::fromImage(m_doc->render(page, size)));
        }
        const QPixmap &pm = m_pageCache[page];
        painter.drawPixmap(0, y, pm);
        y += pm.height() + m_pageSpacing;
        ++page;
    }
}
