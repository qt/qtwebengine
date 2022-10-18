// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content_utility_client_qt.h"

#include "mojo/public/cpp/bindings/service_factory.h"
#include "services/proxy_resolver/proxy_resolver_factory_impl.h"

#if BUILDFLAG(IS_WIN)
#include "services/proxy_resolver_win/public/mojom/proxy_resolver_win.mojom.h"
#include "services/proxy_resolver_win/windows_system_proxy_resolver_impl.h"
#endif

namespace QtWebEngineCore {

ContentUtilityClientQt::ContentUtilityClientQt()
{
}

ContentUtilityClientQt::~ContentUtilityClientQt() = default;

auto RunProxyResolver(mojo::PendingReceiver<proxy_resolver::mojom::ProxyResolverFactory> receiver)
{
    return std::make_unique<proxy_resolver::ProxyResolverFactoryImpl>(std::move(receiver));
}

#if BUILDFLAG(IS_WIN)
auto RunWindowsSystemProxyResolver(
        mojo::PendingReceiver<proxy_resolver_win::mojom::WindowsSystemProxyResolver> receiver)
{
    return std::make_unique<proxy_resolver_win::WindowsSystemProxyResolverImpl>(
            std::move(receiver));
}
#endif

void ContentUtilityClientQt::RegisterIOThreadServices(mojo::ServiceFactory &services)
{
    services.Add(RunProxyResolver);
#if BUILDFLAG(IS_WIN)
    services.Add(RunWindowsSystemProxyResolver);
#endif
}

} // namespace
