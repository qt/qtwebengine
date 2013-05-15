#include "render_widget_host_view_qt.h"

#include "backing_store_qt.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/common/gpu/gpu_messages.h"
#include "raster_window.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScreenInfo.h"

#include <QEvent>
#include <QMouseEvent>
#include <QScreen>

#include <QDebug>

#define QT_NOT_YET_IMPLEMENTED fprintf(stderr, "function %s not implemented! - %s:%d\n", __func__, __FILE__, __LINE__);

static WebKit::WebMouseEvent::Button mouseButtonForEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || (event->buttons() & Qt::LeftButton))
        return WebKit::WebMouseEvent::ButtonLeft;
    else if (event->button() == Qt::RightButton || (event->buttons() & Qt::RightButton))
        return WebKit::WebMouseEvent::ButtonRight;
    else if (event->button() == Qt::MidButton || (event->buttons() & Qt::MidButton))
        return WebKit::WebMouseEvent::ButtonMiddle;
    return WebKit::WebMouseEvent::ButtonNone;
}

static void GetScreenInfoFromNativeWindow(QWindow* window, WebKit::WebScreenInfo* results)
{
    QScreen* screen = window->screen();

    WebKit::WebScreenInfo r;
    r.deviceScaleFactor = screen->devicePixelRatio();
    r.depthPerComponent = 8;
    r.depth = screen->depth();
    r.isMonochrome = (r.depth == 1);

    QRect virtualGeometry = screen->virtualGeometry();
    r.rect = WebKit::WebRect(virtualGeometry.x(), virtualGeometry.y(), virtualGeometry.width(), virtualGeometry.height());
    QRect available = screen->availableGeometry();
    r.availableRect = WebKit::WebRect(available.x(), available.y(), available.width(), available.height());
    *results = r;
}

RenderWidgetHostView::RenderWidgetHostView(content::RenderWidgetHost* widget)
    : m_host(content::RenderWidgetHostImpl::From(widget))
    , m_view(0)
{
}

bool RenderWidgetHostView::handleEvent(QEvent* event) {

    switch(event->type()) {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
        handleMouseEvent(static_cast<QMouseEvent*>(event));
        break;
//        case QEvent::KeyPress:
//            handleKeyEvent(event);
//            break;
    default:
        Q_ASSERT(false); // not reached
    }
    return true;
}

content::BackingStore *RenderWidgetHostView::AllocBackingStore(const gfx::Size &size)
{
    if (m_view)
        return new BackingStoreQt(m_host, size, m_view);
    return 0;
}

RenderWidgetHostView* RenderWidgetHostView::CreateViewForWidget(content::RenderWidgetHost* widget)
{
    return new RenderWidgetHostView(widget);
}

void RenderWidgetHostView::InitAsChild(gfx::NativeView parent_view)
{
    m_view = new RasterWindow(this);
}

void RenderWidgetHostView::InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&)
{
    m_view = new RasterWindow(this);
}

void RenderWidgetHostView::InitAsFullscreen(content::RenderWidgetHostView*)
{
    m_view = new RasterWindow(this);
}

content::RenderWidgetHost* RenderWidgetHostView::GetRenderWidgetHost() const
{
    return m_host;
}

void RenderWidgetHostView::SetSize(const gfx::Size& size)
{
    int width = size.width();
    int height = size.height();
    // int width = std::min(size.width(), kMaxWindowWidth);
    // int height = std::min(size.height(), kMaxWindowHeight);
    if (IsPopup()) {
        // We're a popup, honor the size request.
        m_view->resize(width,height);
    }

    if (m_requestedSize.width() != width ||
        m_requestedSize.height() != height) {
        m_requestedSize = gfx::Size(width, height);
        // m_host->SendScreenRects();
        m_host->WasResized();
    }
}

void RenderWidgetHostView::SetBounds(const gfx::Rect& rect)
{
    // This is called when webkit has sent us a Move message.
    if (IsPopup())
        m_view->setGeometry(rect.x(), rect.y(), rect.width(), rect.height());

    SetSize(rect.size());
}

// FIXME: Should this really return a QWindow pointer?
gfx::NativeView RenderWidgetHostView::GetNativeView() const
{
    QT_NOT_YET_IMPLEMENTED
    // return m_view;
    return gfx::NativeView();
}

QWindow* RenderWidgetHostView::GetNativeViewQt() const OVERRIDE
{
    return m_view;
}

gfx::NativeViewId RenderWidgetHostView::GetNativeViewId() const
{
    QT_NOT_YET_IMPLEMENTED
    return gfx::NativeViewId();
}

gfx::NativeViewAccessible RenderWidgetHostView::GetNativeViewAccessible()
{
    NOTIMPLEMENTED();
    return NULL;
}

// Set focus to the associated View component.
void RenderWidgetHostView::Focus()
{
    m_view->requestActivate();
}

bool RenderWidgetHostView::HasFocus() const
{
    return m_view->isActive();
}

bool RenderWidgetHostView::IsSurfaceAvailableForCopy() const
{
    return true;
}

void RenderWidgetHostView::Show()
{
    m_view->show();
}

void RenderWidgetHostView::Hide()
{
    m_view->hide();
}

bool RenderWidgetHostView::IsShowing()
{
    return m_view->isVisible();
}

// Retrieve the bounds of the View, in screen coordinates.
gfx::Rect RenderWidgetHostView::GetViewBounds() const
{
    QRect rect = m_view->geometry();
    QPoint screenPos = m_view->mapToGlobal(QPoint(0,0));

    return gfx::Rect(screenPos.x(), screenPos.y(), rect.width(), rect.height());
}

// Subclasses should override this method to do what is appropriate to set
// the custom background for their platform.
void RenderWidgetHostView::SetBackground(const SkBitmap& background)
{
    RenderWidgetHostViewBase::SetBackground(background);
    // Send(new ViewMsg_SetBackground(m_host->GetRoutingID(), background));
}

// Return value indicates whether the mouse is locked successfully or not.
bool RenderWidgetHostView::LockMouse()
{
    QT_NOT_YET_IMPLEMENTED
    return false;
}
void RenderWidgetHostView::UnlockMouse()
{
    QT_NOT_YET_IMPLEMENTED
}

// Returns true if the mouse pointer is currently locked.
bool RenderWidgetHostView::IsMouseLocked()
{
    QT_NOT_YET_IMPLEMENTED
    return false;
}

// FIXME: remove TOOLKIT_GTK related things.
#if defined(TOOLKIT_GTK)
// Gets the event for the last mouse down.
GdkEventButton* RenderWidgetHostView::GetLastMouseDown()
{
    return 0;
}

// Builds a submenu containing all the gtk input method commands.
gfx::NativeView RenderWidgetHostView::BuildInputMethodsGtkMenu()
{
}
#endif  // defined(TOOLKIT_GTK)

void RenderWidgetHostView::WasShown()
{
    if (m_view->isVisible())
        return;

    m_host->WasShown();
}

void RenderWidgetHostView::WasHidden()
{
    if (!m_view->isVisible())
        return;

    m_host->WasHidden();
}

void RenderWidgetHostView::MovePluginWindows(const gfx::Vector2d&, const std::vector<webkit::npapi::WebPluginGeometry>&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::Blur()
{
    m_host->Blur();
}

void RenderWidgetHostView::UpdateCursor(const WebCursor&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::SetIsLoading(bool)
{
    QT_NOT_YET_IMPLEMENTED
    // Give visual feedback for loading process.
}

void RenderWidgetHostView::TextInputStateChanged(const ViewHostMsg_TextInputState_Params&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::ImeCancelComposition()
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::ImeCompositionRangeChanged(const ui::Range&, const std::vector<gfx::Rect>&)
{
    // FIXME: not implemented?
}

void RenderWidgetHostView::DidUpdateBackingStore(const gfx::Rect& scroll_rect, const gfx::Vector2d& scroll_delta, const std::vector<gfx::Rect>& copy_rects)
{
    if (!m_view->isVisible())
        return;

    Paint(scroll_rect);

    for (size_t i = 0; i < copy_rects.size(); ++i) {
        gfx::Rect rect = gfx::SubtractRects(copy_rects[i], scroll_rect);
        if (rect.IsEmpty())
            continue;

        Paint(rect);
    }
}

void RenderWidgetHostView::RenderViewGone(base::TerminationStatus, int)
{
    Destroy();
}

void RenderWidgetHostView::Destroy()
{
    delete m_view;
    m_view = 0;
}

void RenderWidgetHostView::SetTooltipText(const string16&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::SelectionBoundsChanged(const ViewHostMsg_SelectionBounds_Params&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::ScrollOffsetChanged()
{
    // FIXME: not implemented?
}

void RenderWidgetHostView::CopyFromCompositingSurface(const gfx::Rect& src_subrect, const gfx::Size& /* dst_size */, const base::Callback<void(bool, const SkBitmap&)>& callback)
{
    // Grab the snapshot from the renderer as that's the only reliable way to
    // readback from the GPU for this platform right now.
    // FIXME: is this true?
    GetRenderWidgetHost()->GetSnapshotFromRenderer(src_subrect, callback);
}

void RenderWidgetHostView::CopyFromCompositingSurfaceToVideoFrame(const gfx::Rect& src_subrect, const scoped_refptr<media::VideoFrame>& target, const base::Callback<void(bool)>& callback)
{
    NOTIMPLEMENTED();
    callback.Run(false);
}

bool RenderWidgetHostView::CanCopyToVideoFrame() const
{
    return false;
}

void RenderWidgetHostView::OnAcceleratedCompositingStateChange()
{
    // bool activated = m_host->is_accelerated_compositing_active();
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::AcceleratedSurfaceBuffersSwapped(const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
}

void RenderWidgetHostView::AcceleratedSurfacePostSubBuffer(const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
}

void RenderWidgetHostView::AcceleratedSurfaceSuspend()
{
    //FIXME: not implemented?
}

void RenderWidgetHostView::AcceleratedSurfaceRelease()
{
    //FIXME: not implemented?
}

bool RenderWidgetHostView::HasAcceleratedSurface(const gfx::Size&)
{
    return false;
}

void RenderWidgetHostView::GetScreenInfo(WebKit::WebScreenInfo* results)
{
    GetScreenInfoFromNativeWindow(m_view, results);
}

gfx::Rect RenderWidgetHostView::GetBoundsInRootWindow()
{
    QRect r = m_view->frameGeometry();
    return gfx::Rect(r.x(), r.y(), r.width(), r.height());
}

gfx::GLSurfaceHandle RenderWidgetHostView::GetCompositingSurface()
{
    QT_NOT_YET_IMPLEMENTED
    return gfx::GLSurfaceHandle();
}

void RenderWidgetHostView::SetHasHorizontalScrollbar(bool) { }

void RenderWidgetHostView::SetScrollOffsetPinning(bool, bool) { }

void RenderWidgetHostView::OnAccessibilityNotifications(const std::vector<AccessibilityHostMsg_NotificationParams>&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostView::Paint(const gfx::Rect& scroll_rect)
{
    bool force_create = !m_host->empty();
    BackingStoreQt* backing_store = static_cast<BackingStoreQt*>(m_host->GetBackingStore(force_create));
    if (backing_store && m_view)
        backing_store->displayBuffer();
}

bool RenderWidgetHostView::IsPopup() const
{
    return popup_type_ != WebKit::WebPopupTypeNone;
}

void RenderWidgetHostView::handleMouseEvent(QMouseEvent* ev)
{
    qDebug() << ev << ev->pos();
    WebKit::WebMouseEvent webKitEvent;
    webKitEvent.x = ev->x();
    webKitEvent.y = ev->y();
    webKitEvent.globalX = ev->globalX();
    webKitEvent.globalY = ev->globalY();
    webKitEvent.clickCount = (ev->type() == QEvent::MouseButtonDblClick)? 2 : 1;
    webKitEvent.button = mouseButtonForEvent(ev);
    //FIXME: and window coordinates ?

    m_host->ForwardMouseEvent(webKitEvent);
}


