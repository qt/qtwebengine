#ifndef CONTENT_BROWSER_RENDERER_HOST_BACKING_STORE_QT_H_
#define CONTENT_BROWSER_RENDERER_HOST_BACKING_STORE_QT_H_

#include "content/browser/renderer_host/backing_store.h"

#include <QPainter>
#include <QPixmap>

class BackingStoreQt : public content::BackingStore
{
public:
    BackingStoreQt(content::RenderWidgetHost *host, const gfx::Size &size, QWindow* parent);
    ~BackingStoreQt();

    void resize(const QSize& size);
    void paintToTarget(QPainter*, const QRect& rect);

    virtual void PaintToBackingStore(content::RenderProcessHost *process, TransportDIB::Id bitmap, const gfx::Rect &bitmap_rect,
                                     const std::vector<gfx::Rect> &copy_rects, float scale_factor, const base::Closure &completion_callback,
                                     bool *scheduled_completion_callback);

    virtual void ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size);
    virtual bool CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output);

private:
    QPainter m_painter;
    content::RenderWidgetHost* m_host;
    QPixmap m_pixelBuffer;
    bool m_isValid;
};

#endif
