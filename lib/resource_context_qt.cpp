#include "resource_context_qt.h"

#include "net/url_request/url_request_context_getter.h"

#include "content/shell/shell_url_request_context_getter.h"

#include "browser_context_qt.h"

net::HostResolver *ResourceContextQt::GetHostResolver()
{
    CHECK(getter_);
    return getter_->host_resolver();
}

net::URLRequestContext* ResourceContextQt::GetRequestContext()
{
    if (getter_)
        return getter_->GetURLRequestContext();
    return context->GetRequestContext()->GetURLRequestContext();
}

void ResourceContextQt::set_url_request_context_getter(content::ShellURLRequestContextGetter *getter)
{
    getter_ = getter;
}
