#ifndef CONTENT_BROWSER_CLIENT_QT
#define CONTENT_BROWSER_CLIENT_QT

#include "content/public/browser/content_browser_client.h"

namespace net {
class URLRequestContextGetter;
}

namespace content {
class BrowserMainParts;
class RenderProcessHost;
class RenderViewHostDelegateView;
class ShellBrowserContext;
class ShellBrowserMainParts;
class WebContentsViewPort;
class WebContents;
struct MainFunctionParams;
}

class BrowserContextQt;
class BrowserMainPartsQt;

class ContentBrowserClientQt : public content::ContentBrowserClient {

public:
    virtual content::WebContentsViewPort* OverrideCreateWebContentsView(content::WebContents* , content::RenderViewHostDelegateView**) /*Q_DECL_OVERRIDE*/;
    virtual content::BrowserMainParts* CreateBrowserMainParts(const content::MainFunctionParams& parameters) /*Q_DECL_OVERRIDE*/;

    content::ShellBrowserContext *browser_context();

    net::URLRequestContextGetter *CreateRequestContext(content::BrowserContext *content_browser_context, content::ProtocolHandlerMap *protocol_handlers);

private:
    BrowserContextQt* m_browser_context;
    BrowserMainPartsQt* m_browserMainParts;
};

#endif // CONTENT_BROWSER_CLIENT_QT
