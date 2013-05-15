#ifndef QT_RASTER_WINDOW_H
#define QT_RASTER_WINDOW_H

#include <QWindow>

class BackingStoreQt;
class RenderWidgetHostView;

class RasterWindow : public QWindow
{
public:
    RasterWindow(RenderWidgetHostView* view, QWindow *parent = 0);

    void renderNow();
    void setBackingStore(BackingStoreQt* backingStore);

protected:

    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *resizeEvent);
    void exposeEvent(QExposeEvent *);

private:
    BackingStoreQt* m_backingStore;
    RenderWidgetHostView *m_view;

};

#endif
