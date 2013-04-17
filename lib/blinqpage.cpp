#include "blinqpage.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/common/content_paths.h"
#include "net/url_request/url_request_context_getter.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/event_types.h"
#include "ui/aura/root_window.h"
#include "ui/aura/root_window_host.h"
#include "ui/aura/root_window_host_delegate.h"
#include "ui/gfx/insets.h"
#include "base/message_loop.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/gfx/screen.h"

#include <QByteArray>
#include <QWindow>
#include <qpa/qplatformwindow.h>

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

static void initializeBlinkPaths()
{
    static bool initialized = false;
    if (initialized)
        return;
    QByteArray processPath = qgetenv("BLINQ_PROCESS_PATH");
    if (processPath.isEmpty())
        qFatal("BLINQ_PROCESS_PATH environment variable not set or empty.");

    PathService::Override(content::CHILD_PROCESS_EXE, base::FilePath(qStringToStringType(QString(processPath))));
}

class Context : public content::BrowserContext
{
public:
   Context()
   {
       tempBasePath.CreateUniqueTempDir();
       if (!content::NotificationService::current())
           content::NotificationService::Create();
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

class RootWindowHostQt : public aura::RootWindowHost
{
public:
    RootWindowHostQt()
    {
        m_window.reset(new QWindow);
        m_window->create();
        m_delegate = 0;
    }

    QWindow *window() { return m_window.get(); }

    virtual void SetDelegate(aura::RootWindowHostDelegate* delegate)
    {
        m_delegate = delegate;
    }

    virtual aura::RootWindow* GetRootWindow()
    {
        return m_delegate->AsRootWindow();
    }

    virtual gfx::AcceleratedWidget GetAcceleratedWidget()
    {
        // ###
        return m_window->handle()->winId();
    }

    virtual void Show()
    {
        m_window->show();
    }

    virtual void Hide()
    {
        m_window->hide();
    }

    virtual void ToggleFullScreen()
    {
        // ### Not sure we want that :)
    }

    virtual gfx::Rect GetBounds() const
    {
        return gfx::Rect(m_window->x(), m_window->y(), m_window->width(), m_window->height());
    }

    virtual void SetBounds(const gfx::Rect& bounds)
    {
        QRect r(bounds.x(), bounds.y(), bounds.width(), bounds.height());
        m_window->setGeometry(r);
    }

    virtual gfx::Insets GetInsets() const
    {
        // ####
        return gfx::Insets();
    }
    virtual void SetInsets(const gfx::Insets& insets)
    {
        // ###
    }

    virtual gfx::Point GetLocationOnNativeScreen() const
    {
        return gfx::Point(m_window->x(), m_window->y());
    }

    virtual void SetCapture()
    {
        // ###
    }

    virtual void ReleaseCapture()
    {
        // ###
    }

    virtual void SetCursor(gfx::NativeCursor cursor)
    {
        // ###
    }

    virtual bool QueryMouseLocation(gfx::Point* location_return)
    {
        // ###
        *location_return = gfx::Point();
    }

    // Clips the cursor to the bounds of the root window until UnConfineCursor().
    virtual bool ConfineCursorToRootWindow()
    {
        // ###
    }
    virtual void UnConfineCursor()
    {
        // ###
    }

    virtual void OnCursorVisibilityChanged(bool show)
    {
        // ###
    }

    virtual void MoveCursorTo(const gfx::Point& location)
    {
        // ###
    }

    virtual void SetFocusWhenShown(bool focus_when_shown)
    {
        // ###
    }

    virtual bool CopyAreaToSkCanvas(const gfx::Rect& source_bounds,
                                    const gfx::Point& dest_offset,
                                    SkCanvas* canvas)
    {
        // ### TODO
    }

    virtual bool GrabSnapshot(
        const gfx::Rect& snapshot_bounds,
        std::vector<unsigned char>* png_representation)
    {
        // ### TODO
    }

  #if !defined(OS_MACOSX)
    virtual void PostNativeEvent(const base::NativeEvent& native_event)
    {
        // ### Nothing to do?
    }
  #endif

    virtual void OnDeviceScaleFactorChanged(float device_scale_factor)
    {
        // ###
    }

    virtual void PrepareForShutdown()
    {
        // ###
    }

private:
    scoped_ptr<QWindow> m_window;
    aura::RootWindowHostDelegate *m_delegate;
};

}

class BlinqPagePrivate
{
public:
    scoped_ptr<RootWindowHostQt> rootWindowHost;
    scoped_ptr<aura::RootWindow> rootWindow;
    scoped_ptr<content::BrowserContext> context;
    scoped_ptr<content::WebContents> contents;
};

BlinqPage::BlinqPage()
{
    static content::ContentMainRunner *runner = 0;
    if (!runner) {
        runner = content::ContentMainRunner::Create();
        runner->Initialize(0, 0, 0);
    }
    if (!base::MessageLoop::current())
        (void)new base::MessageLoopForUI();

    static bool init = false;
    if (!init) {
        init = true;
        gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, views::CreateDesktopScreen());
    }

    d.reset(new BlinqPagePrivate);
    d->rootWindowHost.reset(new RootWindowHostQt);
    {
        aura::RootWindow::CreateParams params(gfx::Rect(0, 0, 100, 100));
        params.host = d->rootWindowHost.get();
        d->rootWindow.reset(new aura::RootWindow(params));
    }
    d->context.reset(new Context);
    d->contents.reset(content::WebContents::Create(content::WebContents::CreateParams(d->context.get())));
    d->rootWindow->Init();
    d->rootWindow->AddChild(d->contents->GetView()->GetNativeView());
}

BlinqPage::~BlinqPage()
{
}

QWindow *BlinqPage::window()
{
    return d->rootWindowHost.get()->window();
}

