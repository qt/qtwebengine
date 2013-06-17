#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_H

#include <QRect>

class BackingStoreQt;
class QWindow;

class RenderWidgetHostViewQtDelegate {
public:
    virtual ~RenderWidgetHostViewQtDelegate() {}
    virtual void setBackingStore(BackingStoreQt* backingStore) = 0;
    virtual QRectF screenRect() const = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isVisible() const = 0;
    virtual QWindow* window() const = 0;
    virtual void update(const QRect& rect = QRect()) = 0;
};

#endif
