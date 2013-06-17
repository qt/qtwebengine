#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WIDGET_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WIDGET_H

#include "shared/render_widget_host_view_qt_delegate.h"

#include <QWidget>

class BackingStoreQt;
class QWindow;

namespace content {
    class RenderWidgetHostViewQt;
}

class RenderWidgetHostViewQtDelegateWidget : public QWidget, public RenderWidgetHostViewQtDelegate
{
public:
    RenderWidgetHostViewQtDelegateWidget(content::RenderWidgetHostViewQt* view, QWidget *parent = 0);

    virtual void setBackingStore(BackingStoreQt* backingStore);
    virtual QRectF screenRect() const;
    virtual void show();
    virtual void hide();
    virtual bool isVisible() const;
    virtual QWindow* window() const;
    virtual void update(const QRect& rect = QRect());

    QPainter* painter();

protected:
    void paintEvent(QPaintEvent * event);
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *resizeEvent);

private:
    BackingStoreQt* m_backingStore;
    QPainter* m_painter;
    content::RenderWidgetHostViewQt *m_view;
};

#endif
