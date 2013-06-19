#include "render_widget_host_view_qt_delegate_quick.h"

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"

#include "content/browser/renderer_host/render_view_host_impl.h"

#include <QQuickWindow>
#include <QWindow>

RenderWidgetHostViewQtDelegateQuick::RenderWidgetHostViewQtDelegateQuick(content::RenderWidgetHostViewQt* view, QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_backingStore(0)
    , m_view(view)
{
    setFocus(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

QRectF RenderWidgetHostViewQtDelegateQuick::screenRect() const
{
    QPointF pos = mapToScene(QPointF(0,0));
    return QRectF(pos.x(), pos.y(), width(), height());
}

void RenderWidgetHostViewQtDelegateQuick::show()
{
    setVisible(true);
}

void RenderWidgetHostViewQtDelegateQuick::hide()
{
    setVisible(false);
}

bool RenderWidgetHostViewQtDelegateQuick::isVisible() const
{
    return QQuickPaintedItem::isVisible();
}

QWindow* RenderWidgetHostViewQtDelegateQuick::window() const
{
    return QQuickPaintedItem::window();
}

void RenderWidgetHostViewQtDelegateQuick::update(const QRect& rect)
{
    polish();
    QQuickPaintedItem::update(rect);
}

void RenderWidgetHostViewQtDelegateQuick::paint(QPainter *painter)
{
    if (!m_backingStore)
        return;

    m_backingStore->paintToTarget(painter, boundingRect());
}

void RenderWidgetHostViewQtDelegateQuick::updatePolish()
{
    // paint will be called from the scene graph thread and this doesn't play well
    // with chromium's use of TLS while getting the backing store.
    // updatePolish() should be called from the GUI thread right before the rendering thread starts.
    m_backingStore = m_view->GetBackingStore();
}

void RenderWidgetHostViewQtDelegateQuick::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);

    m_view->GetRenderWidgetHost()->WasResized();
}

void RenderWidgetHostViewQtDelegateQuick::focusInEvent(QFocusEvent *event)
{
    m_view->handleFocusEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::focusOutEvent(QFocusEvent *event)
{
    m_view->handleFocusEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mousePressEvent(QMouseEvent *event)
{
    setFocus(true);
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseMoveEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseReleaseEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::keyPressEvent(QKeyEvent *event)
{
    m_view->handleKeyEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::keyReleaseEvent(QKeyEvent *event)
{
    m_view->handleKeyEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::wheelEvent(QWheelEvent *event)
{
    m_view->handleWheelEvent(event);
}
