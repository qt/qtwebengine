#include "blinqpage.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/app/content_main_runner.h"
#include "net/url_request/url_request_context_getter.h"
#include "base/files/scoped_temp_dir.h"

#include <QWindow>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

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

}

class BlinqPagePrivate
{
public:
    QWindow *window;
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

    d.reset(new BlinqPagePrivate);
    d->window = 0;
    d->context.reset(new Context);
    d->contents.reset(content::WebContents::Create(content::WebContents::CreateParams(d->context.get())));
}

BlinqPage::~BlinqPage()
{
}

QWindow *BlinqPage::window()
{
    if (!d->window) {
        gfx::NativeView view = d->contents->GetView()->GetNativeView();
        printf("view %p\n", view);
        gfx::NativeWindow window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
        gtk_container_add(GTK_CONTAINER(window), view);
        gtk_widget_show_all(GTK_WIDGET(window));
        d->window = QWindow::fromWinId(GDK_DRAWABLE_XID(gtk_widget_get_window(GTK_WIDGET(view))));
    }
    return d->window;
}

