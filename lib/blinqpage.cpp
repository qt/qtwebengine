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
#include "base/threading/thread_restrictions.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_main_delegate.h"
#include "content/shell/shell_content_browser_client.h"
#include "content/browser/web_contents/web_contents_impl.h"

#include <QByteArray>
#include <QWindow>
#include <QCoreApplication>
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

class BrowserClient : public content::ShellContentBrowserClient
{
public:
    virtual content::WebContentsViewPort* OverrideCreateWebContentsView(content::WebContents* web_contents, content::RenderViewHostDelegateView** render_view_host_delegate_view)
    {
        return 0;
    }
};

class MainDelegate : public content::ShellMainDelegate
{
public:
    virtual content::ContentBrowserClient* CreateContentBrowserClient()
    {
        browserClient.reset(new BrowserClient);
        return browserClient.get();
    };

private:
    scoped_ptr<BrowserClient> browserClient;
};

}

class BlinqPagePrivate
{
public:
    scoped_ptr<content::BrowserContext> context;
    scoped_ptr<content::WebContents> contents;
};

BlinqPage::BlinqPage(int argc, char **argv)
{
    CommandLine::Init(argc, argv);

    static content::ContentMainRunner *runner = 0;
    if (!runner) {
        runner = content::ContentMainRunner::Create();
        runner->Initialize(0, 0, new MainDelegate);
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
    d->contents.reset(content::WebContents::Create(p));
    // d->contents->GetView()->GetNativeView()->Show();

    d->contents->GetController().LoadURL(GURL(std::string("http://qt-project.org/")),
                                         content::Referrer(),
                                         content::PAGE_TRANSITION_TYPED,
                                         std::string());
}

BlinqPage::~BlinqPage()
{
}

QWindow *BlinqPage::window()
{
  return 0;
}

