#include "content/public/browser/resource_context.h"

#ifndef RESOURCE_CONTEXT_QT
#define RESOURCE_CONTEXT_QT

namespace content {
class ShellURLRequestContextGetter;
}

class BrowserContextQt;

class ResourceContextQt : public content::ResourceContext
{
public:
    ResourceContextQt(BrowserContextQt *ctx)
        : context(ctx)
        , getter_(0)
    {}

    virtual net::HostResolver* GetHostResolver();

    virtual net::URLRequestContext* GetRequestContext();

    void set_url_request_context_getter(content::ShellURLRequestContextGetter* getter);

private:
    BrowserContextQt *context;
    content::ShellURLRequestContextGetter* getter_;

    DISALLOW_COPY_AND_ASSIGN(ResourceContextQt);
};

#endif //RESOURCE_CONTEXT_QT
