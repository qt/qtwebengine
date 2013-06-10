#include "native_view_qt.h"

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"
#include <QResizeEvent>
#include <QShowEvent>
#include <QPaintEvent>
#include <QQuickWindow>
#include <QWindow>

QWidgetNativeView::QWidgetNativeView(content::RenderWidgetHostViewQt* view, QWidget *parent)
    : QWidget(parent)
    , m_painter(0)
    , m_backingStore(0)
    , m_view(view)
{
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

QRectF QWidgetNativeView::screenRect() const
{
    return QRectF(x(), y(), width(), height());
}

void QWidgetNativeView::show()
{
    QWidget::show();
}

void QWidgetNativeView::hide()
{
    QWidget::hide();
}


bool QWidgetNativeView::isVisible() const
{
    return QWidget::isVisible();
}

QWindow* QWidgetNativeView::window() const
{
    return QWidget::windowHandle();
}

void QWidgetNativeView::update()
{
    QWidget::update();
}

void QWidgetNativeView::setBackingStore(BackingStoreQt* backingStore)
{
    m_backingStore = backingStore;
    if (m_backingStore)
        m_backingStore->resize(size());
}

void QWidgetNativeView::paintEvent(QPaintEvent * event)
{
    if (!m_backingStore)
        return;
    QPainter painter(this);
    m_backingStore->paintToTarget(&painter, event->rect());
}

QPainter* QWidgetNativeView::painter()
{
    if (!m_painter)
        m_painter = new QPainter(this);
    return m_painter;
}

void QWidgetNativeView::resizeEvent(QResizeEvent *resizeEvent)
{
    if (m_backingStore)
        m_backingStore->resize(resizeEvent->size());
    QWidget::update();
}

bool QWidgetNativeView::event(QEvent *event)
{
    if (!m_view || !m_view->handleEvent(event))
        return QWidget::event(event);
    return true;
}

QQuickNativeView::QQuickNativeView(content::RenderWidgetHostViewQt* view, QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_backingStore(0)
    , m_view(view)
{
    setFocus(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

QRectF QQuickNativeView::screenRect() const
{
    QPointF pos = mapToScene(QPointF(0,0));
    return QRectF(pos.x(), pos.y(), width(), height());
}

void QQuickNativeView::show()
{
    setVisible(true);
}

void QQuickNativeView::hide()
{
    setVisible(true);
}

bool QQuickNativeView::isVisible() const
{
    return QQuickPaintedItem::isVisible();
}

QWindow* QQuickNativeView::window() const
{
    return QQuickPaintedItem::window();
}

void QQuickNativeView::update()
{
    QQuickPaintedItem::update();
}

void QQuickNativeView::paint(QPainter *painter)
{
    if (!m_backingStore)
        return;

    m_backingStore->paintToTarget(painter, boundingRect());
}

void QQuickNativeView::setBackingStore(BackingStoreQt* backingStore)
{
    m_backingStore = backingStore;
    if (m_backingStore)
        m_backingStore->resize(QSize(width(), height()));
}

QSGNode * QQuickNativeView::updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData * data)
{
    return QQuickPaintedItem::updatePaintNode(oldNode, data);
}

void QQuickNativeView::resizeBackingStore()
{
    if (m_backingStore)
        m_backingStore->resize(QSize(width(), height()));
}

void QQuickNativeView::resize(int width, int height)
{
    resetWidth();
    resetHeight();
    setWidth(width);
    setHeight(height);
    resizeBackingStore();
    update();
}

void QQuickNativeView::focusInEvent(QFocusEvent *event)
{
    m_view->handleFocusEvent(event);
}

void QQuickNativeView::focusOutEvent(QFocusEvent *event)
{
    m_view->handleFocusEvent(event);
}

void QQuickNativeView::mousePressEvent(QMouseEvent *event)
{
    setFocus(true);
    m_view->handleMouseEvent(event);
}

void QQuickNativeView::mouseMoveEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void QQuickNativeView::mouseReleaseEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void QQuickNativeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void QQuickNativeView::keyPressEvent(QKeyEvent *event)
{
    m_view->handleKeyEvent(event);
}

void QQuickNativeView::keyReleaseEvent(QKeyEvent *event)
{
    m_view->handleKeyEvent(event);
}

void QQuickNativeView::wheelEvent(QWheelEvent *event)
{
    m_view->handleWheelEvent(event);
}
