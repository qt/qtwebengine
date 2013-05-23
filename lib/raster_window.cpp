#include "raster_window.h"

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"
#include <QResizeEvent>
#include <QShowEvent>
#include <QPaintEvent>

RasterWindow::RasterWindow(content::RenderWidgetHostViewQt* view, QWidget *parent)
    : QWidget(parent)
    , m_painter(0)
    , m_backingStore(0)
    , m_view(view)
{
    setFocusPolicy(Qt::ClickFocus);
}

void RasterWindow::setBackingStore(BackingStoreQt* backingStore)
{
    m_backingStore = backingStore;
}

void RasterWindow::paintEvent(QPaintEvent * event)
{
    if (!m_backingStore)
        return;
    QPainter painter(this);
    m_backingStore->paintToTarget(&painter, event->rect());
}

QPainter* RasterWindow::painter()
{
    if (!m_painter)
        m_painter = new QPainter(this);
    return m_painter;
}

void RasterWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    if (m_backingStore)
        m_backingStore->resize(resizeEvent->size());
    update();
}


bool RasterWindow::event(QEvent *event)
{
    if (!m_view || !m_view->handleEvent(event))
        return QWidget::event(event);
    return true;
}
