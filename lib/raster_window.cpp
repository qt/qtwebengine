#include "raster_window.h"

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"
#include <QResizeEvent>

RasterWindow::RasterWindow(RenderWidgetHostView* view, QWindow *parent)
    : QWindow(parent)
    , m_backingStore(0)
    , m_view(view)
{
}

void RasterWindow::setBackingStore(BackingStoreQt* backingStore)
{
    m_backingStore = backingStore;
}

void RasterWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed()) {
        renderNow();
    }
}

void RasterWindow::renderNow()
{
    if (!isExposed() || !m_backingStore)
        return;
    QRect rect(0, 0, width(), height());
    m_backingStore->displayBuffer();
}

void RasterWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    if (m_backingStore)
        m_backingStore->resize(resizeEvent->size());
    if (isExposed())
        renderNow();
}


bool RasterWindow::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
//    case QEvent::KeyPress:
        if (m_view)
            return m_view->handleEvent(event);
    }
    return QWindow::event(event);
}
