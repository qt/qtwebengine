#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H

#include "shared/render_widget_host_view_qt_delegate.h"

#include <QQuickPaintedItem>

class BackingStoreQt;
class QWindow;
class QQuickItem;
class QFocusEvent;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;

namespace content {
    class RenderWidgetHostViewQt;
}

class RenderWidgetHostViewQtDelegateQuick : public QQuickPaintedItem, public RenderWidgetHostViewQtDelegate
{
    Q_OBJECT
public:
    RenderWidgetHostViewQtDelegateQuick(content::RenderWidgetHostViewQt* view, QQuickItem *parent = 0);

    virtual void setBackingStore(BackingStoreQt* backingStore);
    virtual QRectF screenRect() const;
    virtual void show();
    virtual void hide();
    virtual bool isVisible() const;
    virtual QWindow* window() const;
    virtual void update(const QRect& rect = QRect());

    void paint(QPainter *painter);

    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void wheelEvent(QWheelEvent*);

protected Q_SLOTS:
    void resizeBackingStore();

protected:
    QSGNode* updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData * data);
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private:
    BackingStoreQt* m_backingStore;
    content::RenderWidgetHostViewQt *m_view;

};

#endif
