// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "custom_handlers/register_protocol_handler_request_controller_impl.h"

#include "components/custom_handlers/protocol_handler_registry.h"
#include "content/public/browser/web_contents.h"

#include "custom_handlers/protocol_handler_registry_factory.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

RegisterProtocolHandlerRequestControllerImpl::RegisterProtocolHandlerRequestControllerImpl(
    content::WebContents *webContents,
    custom_handlers::ProtocolHandler handler)
    : RegisterProtocolHandlerRequestController(
        toQt(handler.url()),
        toQt(handler.protocol()))
    , content::WebContentsObserver(webContents)
    , m_handler(handler)
{}

RegisterProtocolHandlerRequestControllerImpl::~RegisterProtocolHandlerRequestControllerImpl()
{
    reject();
}

custom_handlers::ProtocolHandlerRegistry *RegisterProtocolHandlerRequestControllerImpl::protocolHandlerRegistry()
{
    content::WebContents *webContents = web_contents();
    if (!webContents)
        return nullptr;
    content::BrowserContext *context = webContents->GetBrowserContext();
    return ProtocolHandlerRegistryFactory::GetForBrowserContext(context);
}

void RegisterProtocolHandlerRequestControllerImpl::accepted()
{
    if (custom_handlers::ProtocolHandlerRegistry *registry = protocolHandlerRegistry())
        registry->OnAcceptRegisterProtocolHandler(m_handler);
}

void RegisterProtocolHandlerRequestControllerImpl::rejected()
{
    if (custom_handlers::ProtocolHandlerRegistry *registry = protocolHandlerRegistry())
        registry->OnIgnoreRegisterProtocolHandler(m_handler);
}

} // namespace QtWebEngineCore
