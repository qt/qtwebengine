#include "content_browser_client_qt.h"

#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/common/main_function_params.h"
#include "content/public/browser/render_process_host.h"

// To be cleaned up
#include "content/shell/shell.h"
#include "content/shell/shell_browser_main_parts.h"
#include "content/shell/shell_browser_context.h"
#include "content/public/browser/browser_main_parts.h"
#include "net/base/net_module.h"
#include "net/base/net_util.h"
#include "browser_context_qt.h"
#include "web_contents_view_qt.h"

class BrowserMainPartsQt : public content::BrowserMainParts
{
public:
    BrowserMainPartsQt(const content::MainFunctionParams& parameters)
        : content::BrowserMainParts()
        , m_parameters(parameters)
        , m_runMessageLoop(true)
    { }

    void PreMainMessageLoopStart() { }
    void PostMainMessageLoopStart() { }
    void PreEarlyInitialization() { }

    void PreMainMessageLoopRun() {
        m_browserContext.reset(new content::ShellBrowserContext(false));
        m_offTheRecordBrowserContext.reset(new content::ShellBrowserContext(true));

        if (m_parameters.ui_task) {
            m_parameters.ui_task->Run();
            delete m_parameters.ui_task;
            m_runMessageLoop = false;
        }
    }

    bool MainMessageLoopRun(int* result_code)  {
        return !m_runMessageLoop;
    }

    void PostMainMessageLoopRun() {
        m_browserContext.reset();
        m_offTheRecordBrowserContext.reset();
    }

    content::ShellBrowserContext* browser_context() const {
        return m_browserContext.get();
    }

private:
    scoped_ptr<content::ShellBrowserContext> m_browserContext;
    scoped_ptr<content::ShellBrowserContext> m_offTheRecordBrowserContext;

    // For running content_browsertests.
    const content::MainFunctionParams& m_parameters;
    bool m_runMessageLoop;

    DISALLOW_COPY_AND_ASSIGN(BrowserMainPartsQt);
};


content::WebContentsViewPort* ContentBrowserClientQt::OverrideCreateWebContentsView(content::WebContents* web_contents, content::RenderViewHostDelegateView** render_view_host_delegate_view)
{
    fprintf(stderr, "OverrideCreateWebContentsView\n");
    WebContentsViewQt* rv = new WebContentsViewQt(web_contents);
    *render_view_host_delegate_view = rv;
    return rv;
}

content::BrowserMainParts *ContentBrowserClientQt::CreateBrowserMainParts(const content::MainFunctionParams &parameters)
{
    m_browserMainParts = new BrowserMainPartsQt(parameters);
    // FIXME: We don't seem to need it yet, the ShellBrowserContext was being used.
    // m_browser_context = new BrowserContextQt();
    return m_browserMainParts;
}


content::ShellBrowserContext* ContentBrowserClientQt::browser_context() {
    return m_browserMainParts->browser_context();
    // return m_browser_context;
}

net::URLRequestContextGetter* ContentBrowserClientQt::CreateRequestContext(content::BrowserContext* content_browser_context, content::ProtocolHandlerMap* protocol_handlers)
{
    if (content_browser_context != browser_context())
        fprintf(stderr, "Warning: off the record browser context not implemented !\n");
    return browser_context()->CreateRequestContext(protocol_handlers);
}
