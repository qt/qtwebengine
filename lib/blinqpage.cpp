#include "blinqpage.h"

// Needed to get access to content::GetContentClient()
#define CONTENT_IMPLEMENTATION

#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/app/content_main_delegate.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/main_function_params.h"
#include "net/url_request/url_request_context_getter.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/event_types.h"
#include "base/command_line.h"
#include "ui/gfx/insets.h"
#include "base/message_loop.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_conversions.h"
#include "ui/surface/transport_dib.h"
#include "base/threading/thread_restrictions.h"
#include "content/common/view_messages.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/app/shell_main_delegate.h"
#include "content/shell/shell_content_browser_client.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/renderer_host/render_view_host_factory.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_view_gtk.h"
#include "content/browser/renderer_host/backing_store.h"
#include "content/browser/renderer_host/backing_store_gtk.h"
#include "webkit/user_agent/user_agent_util.h"
#include "skia/ext/platform_canvas.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScreenInfo.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebInputEvent.h"

#include <QBackingStore>
#include <QByteArray>
#include <QWindow>
#include <QCoreApplication>
#include <QGuiApplication>
#include <qpa/qplatformwindow.h>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#include <QResizeEvent>
#include <qpa/qplatformnativeinterface.h>

#include <QDebug>


#include <X11/Xutil.h>

#define QT_NOT_YET_IMPLEMENTED fprintf(stderr, "function %s not implemented! - %s:%d\n", __func__, __FILE__, __LINE__);

namespace {

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

class Context;

void GetScreenInfoFromNativeWindow(QWindow* window, WebKit::WebScreenInfo* results)
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

class ResourceContext : public content::ResourceContext
{
public:
    ResourceContext(Context *ctx)
        : context(ctx)
    {}

    virtual net::HostResolver* GetHostResolver() { return 0; }

    inline virtual net::URLRequestContext* GetRequestContext();

private:
    Context *context;
};


static inline base::FilePath::StringType qStringToStringType(const QString &str)
{
#if defined(OS_POSIX)
    return str.toStdString();
#elif defined(OS_WIN)
    return str.toStdWString();
#endif
}

static QByteArray blinqProcessPath() {
    static bool initialized = false;
#ifdef BLINQ_PROCESS_PATH
    static QByteArray processPath(BLINQ_PROCESS_PATH);
#else
    static QByteArray processPath;
#endif
    if (initialized)
        return processPath;
    // Allow overriding at runtime for the time being.
    const QByteArray fromEnv = qgetenv("BLINQ_PROCESS_PATH");
    if (!fromEnv.isEmpty())
        processPath = fromEnv;
    if (processPath.isEmpty())
        qFatal("BLINQ_PROCESS_PATH environment variable not set or empty.");
    initialized = true;
    return processPath;
}

static void initializeBlinkPaths()
{
    static bool initialized = false;
    if (initialized)
        return;

    PathService::Override(content::CHILD_PROCESS_EXE, base::FilePath(qStringToStringType(QString(blinqProcessPath()))));
}

class Context : public content::BrowserContext
{
public:
   Context()
   {
       tempBasePath.CreateUniqueTempDir();
       resourceContext.reset(new ResourceContext(this));
   }
   virtual ~Context() {}

   virtual base::FilePath GetPath()
   {
       return tempBasePath.path();
   }

   virtual bool IsOffTheRecord() const
   {
       return false;
   }

   virtual net::URLRequestContextGetter* GetRequestContext()
   {
       return GetDefaultStoragePartition(this)->GetURLRequestContext();
   }
   virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(int) { return GetRequestContext(); }
   virtual net::URLRequestContextGetter* GetMediaRequestContext() { return GetRequestContext(); }
   virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(int) { return GetRequestContext(); }
   virtual net::URLRequestContextGetter* GetMediaRequestContextForStoragePartition(const base::FilePath&, bool) { return GetRequestContext(); }

   virtual content::ResourceContext* GetResourceContext()
   {
       return resourceContext.get();
   }

   virtual content::DownloadManagerDelegate* GetDownloadManagerDelegate() { return 0; }
   virtual content::GeolocationPermissionContext* GetGeolocationPermissionContext() { return 0; }
   virtual content::SpeechRecognitionPreferences* GetSpeechRecognitionPreferences() { return 0; }
   virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() { return 0; }

private:
    scoped_ptr<content::ResourceContext> resourceContext;
    base::ScopedTempDir tempBasePath; // ### Should become permanent location.

    DISALLOW_COPY_AND_ASSIGN(Context);
};

inline net::URLRequestContext* ResourceContext::GetRequestContext()
{
    return context->GetRequestContext()->GetURLRequestContext();
}

class BackingStoreQt;
class RenderWidgetHostView;

class RasterWindow : public QWindow
{
public:
    RasterWindow(RenderWidgetHostView* view, QWindow *parent = 0)
        : QWindow(parent)
        , m_backingStore(0)
        , m_view(view)
    {
    }

    void renderNow();

    void setBackingStore(BackingStoreQt* backingStore)
    {
        m_backingStore = backingStore;
    }

protected:

    bool event(QEvent *event);

    void resizeEvent(QResizeEvent *resizeEvent);
    
    void exposeEvent(QExposeEvent *)
    {
        if (isExposed()) {
            renderNow();
        }
    }



private:
    BackingStoreQt* m_backingStore;
    RenderWidgetHostView *m_view;

};

class BackingStoreQt : public QBackingStore
                     , public content::BackingStore
{
public:
    BackingStoreQt(content::RenderWidgetHost *host, const gfx::Size &size, RasterWindow* surface)
        : QBackingStore(surface)
        , m_host(content::RenderWidgetHostImpl::From(host))
        , content::BackingStore(host, size)
        , m_surface(surface)
        , m_isValid(false)
    {
        int width = size.width();
        int height = size.height();
        m_surface->resize(width,height);
        resize(QSize(width, height));
        setStaticContents(QRect(0,0,size.width(), size.height()));
        m_surface->setBackingStore(this);
        m_surface->create();
    }

    ~BackingStoreQt()
    {
        if (m_surface)
            m_surface->setBackingStore(0);
    }

    void resize(const QSize& size)
    {
        m_isValid = false;
        QRect contentRect(0, 0, size.width(), size.height());
        QBackingStore::resize(size);
        setStaticContents(contentRect);

        m_host->WasResized();
    }

    void displayBuffer()
    {
        if (!m_surface->isExposed() || !m_isValid)
            return;

        int width = m_surface->width();
        int height = m_surface->height();
        QRect rect(0, 0, width, height);
        flush(rect);
    }

    virtual void PaintToBackingStore(content::RenderProcessHost *process,
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
        int width = QBackingStore::size().width();
        int height = QBackingStore::size().height();
        QImage img(bitmapData, pixel_bitmap_rect.width(), pixel_bitmap_rect.height(), QImage::Format_ARGB32);

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

            beginPaint(destination);
            m_isValid = true;
            QPaintDevice *device = paintDevice();
            if (device) {
                QPainter painter(device);
                painter.drawPixmap(destination, QPixmap::fromImage(img), source);
            }
            endPaint();
        }
    }

    virtual void ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size)
    {
        // DCHECK(delta.x() == 0 || delta.y() == 0);

        // m_pixelBuffer.scroll(delta.x(), delta.y(), clip_rect.x(), clip_rect.y(), clip_rect.width(), clip_rect.height());
    }

    virtual bool CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output)
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

private:
    RasterWindow* m_surface;
    content::RenderWidgetHost* m_host;
    bool m_isValid;
};

void RasterWindow::renderNow()
{
    if (!isExposed() || !m_backingStore)
        return;
    QRect rect(0, 0, width(), height());
    m_backingStore->displayBuffer();
}

void RasterWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    if (m_backingStore)
        m_backingStore->resize(resizeEvent->size());
    if (isExposed())
        renderNow();
}

class RenderWidgetHostView
    : public content::RenderWidgetHostViewBase
{
public:
    RenderWidgetHostView(content::RenderWidgetHost* widget)
        : m_host(content::RenderWidgetHostImpl::From(widget))
        , m_view(0)
    {
    }

    bool handleEvent(QEvent* event) {

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

    virtual content::BackingStore *AllocBackingStore(const gfx::Size &size)
    {
        if (m_view)
            return new BackingStoreQt(m_host, size, m_view);
        return 0;
    }

    static RenderWidgetHostView* CreateViewForWidget(content::RenderWidgetHost* widget)
    {
        return new RenderWidgetHostView(widget);
    }

    virtual void InitAsChild(gfx::NativeView parent_view)
    {
        m_view = new RasterWindow(this);
    }

    virtual void InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&)
    {
        m_view = new RasterWindow(this);
    }

    virtual void InitAsFullscreen(content::RenderWidgetHostView*)
    {
        m_view = new RasterWindow(this);
    }

    virtual content::RenderWidgetHost* GetRenderWidgetHost() const
    {
        return m_host;
    }

    virtual void SetSize(const gfx::Size& size)
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

    virtual void SetBounds(const gfx::Rect& rect)
    {
        // This is called when webkit has sent us a Move message.
        if (IsPopup())
            m_view->setGeometry(rect.x(), rect.y(), rect.width(), rect.height());

        SetSize(rect.size());
    }

    // FIXME: Should this really return a QWindow pointer?
    virtual gfx::NativeView GetNativeView() const
    {
        QT_NOT_YET_IMPLEMENTED
        // return m_view;
        return gfx::NativeView();
    }

    virtual QWindow* GetNativeViewQt() const OVERRIDE
    {
        return m_view;
    }

    virtual gfx::NativeViewId GetNativeViewId() const
    {
        QT_NOT_YET_IMPLEMENTED
        return gfx::NativeViewId();
    }

    virtual gfx::NativeViewAccessible GetNativeViewAccessible()
    {
        NOTIMPLEMENTED();
        return NULL;
    }

    // Set focus to the associated View component.
    virtual void Focus()
    {
        m_view->requestActivate();
    }

    virtual bool HasFocus() const
    {
        return m_view->isActive();
    }

    virtual bool IsSurfaceAvailableForCopy() const
    {
        return true;
    }

    virtual void Show()
    {
        m_view->show();
    }

    virtual void Hide()
    {
        m_view->hide();
    }

    virtual bool IsShowing()
    {
        return m_view->isVisible();
    }

    // Retrieve the bounds of the View, in screen coordinates.
    virtual gfx::Rect GetViewBounds() const
    {
        QRect rect = m_view->geometry();
        QPoint screenPos = m_view->mapToGlobal(QPoint(0,0));

        return gfx::Rect(screenPos.x(), screenPos.y(), rect.width(), rect.height());
    }

    // Subclasses should override this method to do what is appropriate to set
    // the custom background for their platform.
    virtual void SetBackground(const SkBitmap& background)
    {
        RenderWidgetHostViewBase::SetBackground(background);
        // Send(new ViewMsg_SetBackground(m_host->GetRoutingID(), background));
    }

    // Return value indicates whether the mouse is locked successfully or not.
    virtual bool LockMouse()
    {
        QT_NOT_YET_IMPLEMENTED
        return false;
    }
    virtual void UnlockMouse()
    {
        QT_NOT_YET_IMPLEMENTED
    }

    // Returns true if the mouse pointer is currently locked.
    virtual bool IsMouseLocked()
    {
        QT_NOT_YET_IMPLEMENTED
        return false;
    }

    // FIXME: remove TOOLKIT_GTK related things.
#if defined(TOOLKIT_GTK)
    // Gets the event for the last mouse down.
    virtual GdkEventButton* GetLastMouseDown()
    {
        return 0;
    }

    // Builds a submenu containing all the gtk input method commands.
    virtual gfx::NativeView BuildInputMethodsGtkMenu()
    {
    }
#endif  // defined(TOOLKIT_GTK)

    virtual void WasShown()
    {
        if (m_view->isVisible())
            return;

        m_host->WasShown();
    }

    virtual void WasHidden()
    {
        if (!m_view->isVisible())
            return;

        m_host->WasHidden();
    }

    virtual void MovePluginWindows(const gfx::Vector2d&, const std::vector<webkit::npapi::WebPluginGeometry>&)
    {
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void Blur()
    {
        m_host->Blur();
    }

    virtual void UpdateCursor(const WebCursor&)
    {
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void SetIsLoading(bool)
    {
        QT_NOT_YET_IMPLEMENTED
        // Give visual feedback for loading process.
    }

    virtual void TextInputStateChanged(const ViewHostMsg_TextInputState_Params&)
    {
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void ImeCancelComposition()
    {
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void ImeCompositionRangeChanged(const ui::Range&, const std::vector<gfx::Rect>&)
    {
        // FIXME: not implemented?
    }

    virtual void DidUpdateBackingStore(const gfx::Rect& scroll_rect, const gfx::Vector2d& scroll_delta, const std::vector<gfx::Rect>& copy_rects)
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

    virtual void RenderViewGone(base::TerminationStatus, int)
    {
        Destroy();
    }

    virtual void Destroy()
    {
        delete m_view;
        m_view = 0;
    }

    virtual void SetTooltipText(const string16&)
    {
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void SelectionBoundsChanged(const ViewHostMsg_SelectionBounds_Params&)
    {
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void ScrollOffsetChanged()
    {
        // FIXME: not implemented?
    }

    virtual void CopyFromCompositingSurface(const gfx::Rect& src_subrect, const gfx::Size& /* dst_size */, const base::Callback<void(bool, const SkBitmap&)>& callback)
    {
        // Grab the snapshot from the renderer as that's the only reliable way to
        // readback from the GPU for this platform right now.
        // FIXME: is this true?
        GetRenderWidgetHost()->GetSnapshotFromRenderer(src_subrect, callback);
    }

    virtual void CopyFromCompositingSurfaceToVideoFrame(const gfx::Rect& src_subrect, const scoped_refptr<media::VideoFrame>& target, const base::Callback<void(bool)>& callback)
    {
        NOTIMPLEMENTED();
        callback.Run(false);
    }

    virtual bool CanCopyToVideoFrame() const
    {
        return false;
    }

    virtual void OnAcceleratedCompositingStateChange()
    {
        // bool activated = m_host->is_accelerated_compositing_active();
        QT_NOT_YET_IMPLEMENTED
    }

    virtual void AcceleratedSurfaceBuffersSwapped(const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params, int gpu_host_id)
    {
        AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
        ack_params.sync_point = 0;
        content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
    }

    virtual void AcceleratedSurfacePostSubBuffer(const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params, int gpu_host_id)
    {
        AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
        ack_params.sync_point = 0;
        content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
    }

    virtual void AcceleratedSurfaceSuspend()
    {
        //FIXME: not implemented?
    }

    virtual void AcceleratedSurfaceRelease()
    {
        //FIXME: not implemented?
    }

    virtual bool HasAcceleratedSurface(const gfx::Size&)
    {
        return false;
    }

    virtual void GetScreenInfo(WebKit::WebScreenInfo* results)
    {
        GetScreenInfoFromNativeWindow(m_view, results);
    }

    virtual gfx::Rect GetBoundsInRootWindow()
    {
        QRect r = m_view->frameGeometry();
        return gfx::Rect(r.x(), r.y(), r.width(), r.height());
    }

    virtual gfx::GLSurfaceHandle GetCompositingSurface()
    {
        QT_NOT_YET_IMPLEMENTED
        return gfx::GLSurfaceHandle();
    }

    virtual void SetHasHorizontalScrollbar(bool) { }
    virtual void SetScrollOffsetPinning(bool, bool) { }
    virtual void OnAccessibilityNotifications(const std::vector<AccessibilityHostMsg_NotificationParams>&)
    {
        QT_NOT_YET_IMPLEMENTED
    }

private:
    void Paint(const gfx::Rect& scroll_rect)
    {
        bool force_create = !m_host->empty();
        BackingStoreQt* backing_store = static_cast<BackingStoreQt*>(m_host->GetBackingStore(force_create));
        if (backing_store && m_view)
            backing_store->displayBuffer();
    }

    bool IsPopup() const
    {
        return popup_type_ != WebKit::WebPopupTypeNone;
    }

    void handleMouseEvent(QMouseEvent* ev)
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

    content::RenderWidgetHostImpl *m_host;
    RasterWindow *m_view;
    gfx::Size m_requestedSize;
};

class RenderViewHost : public content::RenderViewHostImpl
{
public:
    RenderViewHost(
        content::SiteInstance* instance,
        content::RenderViewHostDelegate* delegate,
        content::RenderWidgetHostDelegate* widget_delegate,
        int routing_id,
        bool swapped_out,
        content::SessionStorageNamespace* session_storage_namespace)
        : content::RenderViewHostImpl(instance, delegate, widget_delegate, routing_id, swapped_out, session_storage_namespace)
    {
        SetView(new RenderWidgetHostView(this));
    }
};

class ViewHostFactory : public content::RenderViewHostFactory
{
public:
    ViewHostFactory()
    {
        content::RenderViewHostFactory::RegisterFactory(this);
    }
    ~ViewHostFactory()
    {
        content::RenderViewHostFactory::UnregisterFactory();
    }

    virtual content::RenderViewHost *CreateRenderViewHost(content::SiteInstance *instance,
                                                          content::RenderViewHostDelegate *delegate,
                                                          content::RenderWidgetHostDelegate *widget_delegate,
                                                          int routing_id,
                                                          bool swapped_out,
                                                          content::SessionStorageNamespace *session_storage_namespace)
    {
        content::RenderViewHost *vh = new RenderViewHost(instance, delegate, widget_delegate, routing_id, swapped_out, session_storage_namespace);
        vh->GetView()->InitAsChild(0);

        return vh;
    }
};

// Return a timeout suitable for the glib loop, -1 to block forever,
// 0 to return right away, or a timeout in milliseconds from now.
int GetTimeIntervalMilliseconds(const base::TimeTicks& from) {
  if (from.is_null())
    return -1;

  // Be careful here.  TimeDelta has a precision of microseconds, but we want a
  // value in milliseconds.  If there are 5.5ms left, should the delay be 5 or
  // 6?  It should be 6 to avoid executing delayed work too early.
  int delay = static_cast<int>(
      ceil((from - base::TimeTicks::Now()).InMillisecondsF()));

  // If this value is negative, then we need to run delayed work soon.
  return delay < 0 ? 0 : delay;
}

#if 0
class MessagePump : public QObject,
                    public base::MessagePump
{
public:
    struct DelayedWorkEvent : public QEvent
    {
    public:
        DelayedWorkEvent(int msecs)
            : QEvent(static_cast<QEvent::Type>(QEvent::User + 1))
            , m_secs(msecs)
        {}

        int m_secs;
    };

    MessagePump()
        : m_delegate(0)
    {
    }

    virtual void Run(Delegate *delegate)
    {
        m_delegate = delegate;
        printf("RUN\n");
    }

    virtual void Quit()
    {
        printf("QUIT?!\n");
    }

    virtual void ScheduleWork()
    {
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
        printf("ScheduleWork\n");
    }

    virtual void ScheduleDelayedWork(const base::TimeTicks &delayed_work_time)
    {
        printf("Schedule Delayed Work %d\n", GetTimeIntervalMilliseconds(delayed_work_time));
//        QCoreApplication::postEvent(this, new DelayedWorkEvent(GetTimeIntervalMilliseconds(delayed_work_time)));
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

protected:
    virtual void customEvent(QEvent *ev)
    {
        if (ev->type() == QEvent::User + 1) {
            startTimer(static_cast<DelayedWorkEvent*>(ev)->m_secs);
            return;
        }
        printf("customEvent\n");
        if (!m_delegate)
            return;
        if (m_delegate->DoWork())
            return;
        m_delegate->DoIdleWork();
    }

    virtual void timerEvent(QTimerEvent *ev)
    {
        printf("timerEvent\n");
        killTimer(ev->timerId());
        if (!m_delegate)
            return;
        base::TimeTicks next_delayed_work_time;
        if (!m_delegate->DoDelayedWork(&next_delayed_work_time))
            m_delegate->DoIdleWork();

        if (!next_delayed_work_time.is_null()) {
            QCoreApplication::postEvent(this, new QEvent(QEvent::User));
//            startTimer(GetTimeIntervalMilliseconds(next_delayed_work_time));
        }
    }

private:
    Delegate *m_delegate;
};

base::MessagePump* messagePumpFactory()
{
    return new MessagePump;
}
#endif

}

bool RasterWindow::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
//    case QEvent::KeyPress:
        if (m_view)
            return m_view->handleEvent(event);
    }
    return QWindow::event(event);
}

class BlinqPagePrivate
{
public:
    scoped_ptr<content::BrowserContext> context;
    scoped_ptr<content::WebContents> contents;
};

BlinqPage::BlinqPage(int argc, char **argv)
{
    {
        int myArgc = argc + 3;
        const char **myArgv = new const char *[myArgc];

        for (int i = 0; i < argc; ++i)
            myArgv[i] = argv[i];
        QByteArray subProcessPathOption("--browser-subprocess-path=");
        subProcessPathOption.append(blinqProcessPath());
        myArgv[argc] = subProcessPathOption.constData();
        myArgv[argc + 1] = "--no-sandbox";

        std::string ua = webkit_glue::BuildUserAgentFromProduct("Qrome/0.1");

        QByteArray userAgentParameter("--user-agent=");
        userAgentParameter.append(QString::fromStdString(ua).toUtf8());
        myArgv[argc + 2] = userAgentParameter.constData();

        CommandLine::Init(myArgc, myArgv);

        delete [] myArgv;

//        base::MessageLoop::InitMessagePumpForUIFactory(::messagePumpFactory);
    }

    static content::ContentMainRunner *runner = 0;
    if (!runner) {
        (void)new ViewHostFactory();

        runner = content::ContentMainRunner::Create();
        runner->Initialize(0, 0, new content::ShellMainDelegate);
    }

    initializeBlinkPaths();

    static content::BrowserMainRunner *browserRunner = 0;
    if (!browserRunner) {
        //CommandLine::Init(0, 0);

        browserRunner = content::BrowserMainRunner::Create();

        browserRunner->Initialize(content::MainFunctionParams(*CommandLine::ForCurrentProcess()));
    }

    base::ThreadRestrictions::SetIOAllowed(true);

    d.reset(new BlinqPagePrivate);

    d->context.reset(static_cast<content::ShellContentBrowserClient*>(content::GetContentClient()->browser())->browser_context());
    content::WebContents::CreateParams p(d->context.get());
//    d->contents.reset(content::WebContents::Create(p));

//    d->contents->GetController().LoadURL(GURL(std::string("http://qt-project.org/")),
//                                         content::Referrer(),
//                                         content::PAGE_TRANSITION_TYPED,
//                                         std::string());

    MessageLoopForUI::current()->Run();
}

BlinqPage::~BlinqPage()
{
}

QWindow *BlinqPage::window()
{
  return 0;
}

