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

#include "backing_store_qt.h"
#include "raster_window.h"
#include "render_widget_host_view_qt.h"

#include <QByteArray>
#include <QWindow>
#include <QCoreApplication>
#include <QGuiApplication>
#include <qpa/qplatformwindow.h>
#include <QLabel>
#include <QPainter>
#include <qpa/qplatformnativeinterface.h>

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

