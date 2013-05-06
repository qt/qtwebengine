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
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_main_delegate.h"
#include "content/shell/shell_content_browser_client.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/renderer_host/render_view_host_factory.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_view_gtk.h"
#include "content/browser/renderer_host/backing_store.h"
#include "content/browser/renderer_host/backing_store_gtk.h"
#include "webkit/user_agent/user_agent_util.h"
#include "skia/ext/platform_canvas.h"

#include <QByteArray>
#include <QWindow>
#include <QCoreApplication>
#include <qpa/qplatformwindow.h>
#include <QLabel>
#include <QPainter>

namespace {

class Context;

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

class BackingStoreQt : public QLabel
                     , public content::BackingStore
{
public:
    BackingStoreQt(content::RenderWidgetHost *host, const gfx::Size &size)
        : content::BackingStore(host, size)
        , m_pixelBuffer(size.width(), size.height())
    {
        // FIXME: remove QLabel inheritance
        resize(size.width(), size.height());
        show();
        setWindowTitle(QStringLiteral("BackingStoreQt"));
        // FISME: remove QLabel inheritance
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

        // gfx::Rect pixel_bitmap_rect = gfx::ToEnclosedRect(gfx::ScaleRect(bitmap_rect, scale_factor));
        gfx::Rect pixel_bitmap_rect = bitmap_rect;


        uint8_t* bitmapData = static_cast<uint8_t*>(dib->memory());
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

        // FIXME: remove QLabel inheritance
        setPixmap(m_pixelBuffer);
        repaint();
        // FIXME: remove QLabel inheritance
    }

    virtual void ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size)
    {
        DCHECK(delta.x() == 0 || delta.y() == 0);

        m_pixelBuffer.scroll(delta.x(), delta.y(), clip_rect.x(), clip_rect.y(), clip_rect.width(), clip_rect.height());
    }

    virtual bool CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output)
    {
        const int width = std::min(m_pixelBuffer.width(), rect.width());
        const int height = std::min(m_pixelBuffer.height(), rect.height());

        if (!output->Allocate(width, height, true))
            return false;

        // This code assumes a visual mode where a pixel is
        // represented using a 32-bit unsigned int, with a byte per component.
        const SkBitmap& bitmap = output->GetBitmap();
        SkAutoLockPixels alp(bitmap);

        QPixmap cpy = m_pixelBuffer.copy(rect.x(), rect.y(), rect.width(), rect.height());
        QImage img = cpy.toImage();

        // Convert the format and remove transparency.
        if (img.format() != QImage::Format_RGB32)
            img = img.convertToFormat(QImage::Format_RGB32);

        const uint8_t* src = img.bits();
        uint8_t* dst = reinterpret_cast<uint8_t*>(bitmap.getAddr32(0,0));
        memcpy(dst, src, width*height*32);

        return true;
    }

private:
    QPainter m_painter;
    QPixmap m_pixelBuffer;
};

class BackingStore : public QLabel,
                     public content::BackingStoreGtk
{
public:
    BackingStore(content::RenderWidgetHost *host, const gfx::Size &size, content::RenderWidgetHostView *view)
        : content::BackingStoreGtk(host, size,
                                   ui::GetVisualFromGtkWidget(view->GetNativeView()),
                                   gdk_visual_get_depth(gtk_widget_get_visual(view->GetNativeView())))
        , m_size(size)
        , m_backingStoreQt(host, size)
    {
        resize(size.width(), size.height());
        // show();
    }

    virtual void PaintToBackingStore(content::RenderProcessHost *process,
                                     TransportDIB::Id bitmap,
                                     const gfx::Rect &bitmap_rect,
                                     const std::vector<gfx::Rect> &copy_rects,
                                     float scale_factor,
                                     const base::Closure &completion_callback,
                                     bool *scheduled_completion_callback)
    {
        *scheduled_completion_callback = false;
        TransportDIB* dib = process->GetTransportDIB(bitmap);
        if (!dib)
          return;

        scoped_ptr<SkCanvas> canvas(dib->GetPlatformCanvas(bitmap_rect.width(), bitmap_rect.height()));
        SkISize size = canvas->getDeviceSize();

        QImage img(size.fWidth, size.fHeight, QImage::Format_ARGB32);
        img.fill(Qt::green);
        SkBitmap bm;
        bm.setConfig(SkBitmap::kARGB_8888_Config, size.fWidth, size.fHeight);
        bm.setPixels(img.bits());
        canvas->readPixels(&bm, 0, 0);
        setPixmap(QPixmap::fromImage(img));

        BackingStoreGtk::PaintToBackingStore(process, bitmap, bitmap_rect, copy_rects, scale_factor, completion_callback, scheduled_completion_callback);

        m_backingStoreQt.PaintToBackingStore(process, bitmap, bitmap_rect, copy_rects, scale_factor, completion_callback, scheduled_completion_callback);
    }

    virtual void ScrollBackingStore(const gfx::Vector2d &delta, const gfx::Rect &clip_rect, const gfx::Size &view_size)
    {
        BackingStoreGtk::ScrollBackingStore(delta, clip_rect, view_size);

        m_backingStoreQt.ScrollBackingStore(delta, clip_rect, view_size);
    }

    virtual bool CopyFromBackingStore(const gfx::Rect &rect, skia::PlatformBitmap *output)
    {
        bool ret = BackingStoreGtk::CopyFromBackingStore(rect, output);
        m_backingStoreQt.CopyFromBackingStore(rect, output);
        return ret;
    }

private:
    gfx::Size m_size;

    // Temporary Qt members
    BackingStoreQt m_backingStoreQt;
};

class RenderWidgetHostView : public content::RenderWidgetHostViewGtk
{
public:
    RenderWidgetHostView(content::RenderWidgetHost* widget)
        : content::RenderWidgetHostViewGtk(widget)
        , m_host(widget)
    {
    }

    virtual content::BackingStore *AllocBackingStore(const gfx::Size &size)
    {
        return new BackingStore(m_host, size, this);
    }

private:
    content::RenderWidgetHost *m_host;
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

