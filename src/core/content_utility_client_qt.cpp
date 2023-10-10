// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content_utility_client_qt.h"

#include "base/no_destructor.h"
#include "mojo/public/cpp/bindings/service_factory.h"
#include "services/proxy_resolver/proxy_resolver_factory_impl.h"

namespace QtWebEngineCore {

ContentUtilityClientQt::ContentUtilityClientQt()
{
}

ContentUtilityClientQt::~ContentUtilityClientQt() = default;

auto RunProxyResolver(mojo::PendingReceiver<proxy_resolver::mojom::ProxyResolverFactory> receiver)
{
    return std::make_unique<proxy_resolver::ProxyResolverFactoryImpl>(std::move(receiver));
}

void ContentUtilityClientQt::RegisterIOThreadServices(mojo::ServiceFactory &services)
{
    services.Add(RunProxyResolver);
}

} // namespace
