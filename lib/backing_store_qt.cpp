#include "backing_store_qt.h"

#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/render_process_host.h"
#include "raster_window.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_conversions.h"

#include <QPainter>

BackingStoreQt::BackingStoreQt(content::RenderWidgetHost *host, const gfx::Size &size, QWindow* parent)
    : m_host(content::RenderWidgetHostImpl::From(host))
    , m_pixelBuffer(size.width(), size.height())
    , content::BackingStore(host, size)
    , m_isValid(false)
{
    int width = size.width();
    int height = size.height();
    resize(QSize(width, height));
}

BackingStoreQt::~BackingStoreQt()
{
}

void BackingStoreQt::resize(const QSize& size)
{
    m_isValid = false;
    if (size != m_pixelBuffer.size()) {
        m_pixelBuffer = QPixmap(size);
        m_host->WasResized();
    }
}

void BackingStoreQt::paintToTarget(QPainter* painter, const QRect& rect)
{
    if (m_pixelBuffer.isNull())
        return;
    painter->drawPixmap(rect, m_pixelBuffer);
}

void BackingStoreQt::PaintToBackingStore(content::RenderProcessHost *process,
                                 TransportDIB::Id bitmap,
                                 const gfx::Rect &bitmap_rect,
                                 const std::vector<gfx::Rect> &copy_rects,
                                 float scale_factor,
                                 const base::Closure &completion_callback,
                                 bool *scheduled_completion_callback)
{
    if (bitmap_rect.IsEmpty())
        return;

    *scheduled_completion_callback = false;
    TransportDIB* dib = process->GetTransportDIB(bitmap);
    if (!dib)
      return;

    gfx::Rect pixel_bitmap_rect = bitmap_rect;

    uint8_t* bitmapData = static_cast<uint8_t*>(dib->memory());
    int width = m_pixelBuffer.size().width();
    int height = m_pixelBuffer.size().height();
    QImage img(bitmapData, pixel_bitmap_rect.width(), pixel_bitmap_rect.height(), QImage::Format_ARGB32);

    m_painter.begin(&m_pixelBuffer);

    for (size_t i = 0; i < copy_rects.size(); ++i) {
        gfx::Rect copy_rect = gfx::ToEnclosedRect(gfx::ScaleRect(copy_rects[i], scale_factor));

        QRect source = QRect( copy_rect.x() - pixel_bitmap_rect.x()
                            , copy_rect.y() - pixel_bitmap_rect.y()
                            , pixel_bitmap_rect.width()
                            , pixel_bitmap_rect.height());

        QRect destination = QRect( copy_rect.x()
                                 , copy_rect.y()
                                 , copy_rect.width()
                                 , copy_rect.height());

        m_painter.drawPixmap(destination, QPixmap::fromImage(img), source);
    }

    m_painter.end();
}

void BackingStoreQt::ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size)
{
    // DCHECK(delta.x() == 0 || delta.y() == 0);

    // m_pixelBuffer.scroll(delta.x(), delta.y(), clip_rect.x(), clip_rect.y(), clip_rect.width(), clip_rect.height());
}

bool BackingStoreQt::CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output)
{
    // const int width = std::min(m_pixelBuffer.width(), rect.width());
    // const int height = std::min(m_pixelBuffer.height(), rect.height());

    // if (!output->Allocate(width, height, true))
    //     return false;

    // // This code assumes a visual mode where a pixel is
    // // represented using a 32-bit unsigned int, with a byte per component.
    // const SkBitmap& bitmap = output->GetBitmap();
    // SkAutoLockPixels alp(bitmap);

    // QPixmap cpy = m_pixelBuffer.copy(rect.x(), rect.y(), rect.width(), rect.height());
    // QImage img = cpy.toImage();

    // // Convert the format and remove transparency.
    // if (img.format() != QImage::Format_RGB32)
    //     img = img.convertToFormat(QImage::Format_RGB32);

    // const uint8_t* src = img.bits();
    // uint8_t* dst = reinterpret_cast<uint8_t*>(bitmap.getAddr32(0,0));
    // memcpy(dst, src, width*height*32);

    // return true;
}

