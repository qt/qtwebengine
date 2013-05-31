#include "content/public/browser/browser_context.h"

#include "base/files/scoped_temp_dir.h"
#include "base/time.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "net/url_request/url_request_context_getter.h"

#include "content/shell/shell_url_request_context_getter.h"

#include "resource_context_qt.h"

#ifndef BROWSER_CONTEXT_QT
#define BROWSER_CONTEXT_QT

class BrowserContextQt : public content::BrowserContext
{
public:
   explicit BrowserContextQt()
   {
       tempBasePath.CreateUniqueTempDir();
       resourceContext.reset(new ResourceContextQt(this));
   }
   virtual ~BrowserContextQt() {}

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

   net::URLRequestContextGetter *CreateRequestContext(content::ProtocolHandlerMap* protocol_handlers)
   {
       url_request_getter_ = new content::ShellURLRequestContextGetter(/*ignore_certificate_errors = */ false, GetPath(), content::BrowserThread::UnsafeGetMessageLoopForThread(content::BrowserThread::IO)
                                                                       , content::BrowserThread::UnsafeGetMessageLoopForThread(content::BrowserThread::FILE), protocol_handlers);
       static_cast<ResourceContextQt*>(resourceContext.get())->set_url_request_context_getter(url_request_getter_.get());
       return url_request_getter_.get();
   }

private:
   scoped_ptr<content::ResourceContext> resourceContext;
   base::ScopedTempDir tempBasePath; // ### Should become permanent location.
   scoped_refptr<content::ShellURLRequestContextGetter> url_request_getter_;

   DISALLOW_COPY_AND_ASSIGN(BrowserContextQt);
};

#endif //BROWSER_CONTEXT_QT
