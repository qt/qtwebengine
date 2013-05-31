#include "content_browser_client_qt.h"

#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/common/main_function_params.h"
#include "content/public/browser/render_process_host.h"

// To be cleaned up
#include "content/shell/shell_browser_main_parts.h"

#include "browser_context_qt.h"
#include "web_contents_view_qt.h"


content::WebContentsViewPort* ContentBrowserClientQt::OverrideCreateWebContentsView(content::WebContents* web_contents, content::RenderViewHostDelegateView** render_view_host_delegate_view)
{
    fprintf(stderr, "OverrideCreateWebContentsView\n");
    WebContentsViewQt* rv = new WebContentsViewQt(web_contents);
    *render_view_host_delegate_view = rv;
    return rv;
}

content::BrowserMainParts *ContentBrowserClientQt::CreateBrowserMainParts(const content::MainFunctionParams &parameters)
{
    m_browserMainParts = new content::ShellBrowserMainParts(parameters);
    m_browser_context = new BrowserContextQt();
    return m_browserMainParts;
}


BrowserContextQt* ContentBrowserClientQt::browser_context() {

    return m_browser_context;
}

net::URLRequestContextGetter* ContentBrowserClientQt::CreateRequestContext(content::BrowserContext* content_browser_context, content::ProtocolHandlerMap* protocol_handlers)
{
    if (content_browser_context != browser_context())
        fprintf(stderr, "Warning: off the record browser context not implemented !\n");
    return browser_context()->CreateRequestContext(protocol_handlers);
}
