#include "blinqpage.h"

// Needed to get access to content::GetContentClient()
#define CONTENT_IMPLEMENTATION

#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents_view.h"
#include "net/url_request/url_request_context_getter.h"
#include "base/files/scoped_temp_dir.h"
#include "base/event_types.h"
#include "ui/gfx/insets.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_conversions.h"
#include "ui/surface/transport_dib.h"
#include "content/common/view_messages.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/renderer_host/render_view_host_factory.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/backing_store.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_content_browser_client.h"
#include "skia/ext/platform_canvas.h"

#include "backing_store_qt.h"
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
        SetView(new content::RenderWidgetHostViewQt(this));
    }
};

}

class BlinqPagePrivate
{
public:
    scoped_ptr<content::BrowserContext> context;
    scoped_ptr<content::WebContents> contents;
};

BlinqPage::BlinqPage()
{
    d.reset(new BlinqPagePrivate);

    d->context.reset(static_cast<content::ShellContentBrowserClient*>(content::GetContentClient()->browser())->browser_context());
    content::WebContents::CreateParams p(d->context.get());
//    d->contents.reset(content::WebContents::Create(p));

//    d->contents->GetController().LoadURL(GURL(std::string("http://qt-project.org/")),
//                                         content::Referrer(),
//                                         content::PAGE_TRANSITION_TYPED,
//                                         std::string());
}

BlinqPage::~BlinqPage()
{
}

QWindow *BlinqPage::window()
{
  return 0;
}

