#include "render_widget_host_view_qt_delegate_quick.h"

#include "shared/backing_store_qt.h"
#include "shared/render_widget_host_view_qt.h"
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
    QQuickPaintedItem::update(rect);
}

void RenderWidgetHostViewQtDelegateQuick::paint(QPainter *painter)
{
    if (!m_backingStore)
        return;

    m_backingStore->paintToTarget(painter, boundingRect());
}

void RenderWidgetHostViewQtDelegateQuick::setBackingStore(BackingStoreQt* backingStore)
{
    m_backingStore = backingStore;
    if (m_backingStore)
        m_backingStore->resize(QSize(width(), height()));
}

QSGNode * RenderWidgetHostViewQtDelegateQuick::updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData * data)
{
    return QQuickPaintedItem::updatePaintNode(oldNode, data);
}

void RenderWidgetHostViewQtDelegateQuick::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);

    resizeBackingStore();
}

void RenderWidgetHostViewQtDelegateQuick::resizeBackingStore()
{
    if (m_backingStore)
        m_backingStore->resize(QSize(width(), height()));
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
