// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H
#define REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H

#include "custom_handlers/register_protocol_handler_request_controller.h"

#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/custom_handlers/protocol_handler.h"

namespace custom_handlers {
class ProtocolHandlerRegistry;
}

namespace QtWebEngineCore {

class RegisterProtocolHandlerRequestControllerImpl final : public RegisterProtocolHandlerRequestController,
                                                           private content::WebContentsObserver {
public:
    RegisterProtocolHandlerRequestControllerImpl(
        content::WebContents *webContents,
        content::ProtocolHandler handler);

    ~RegisterProtocolHandlerRequestControllerImpl();

protected:
    void accepted() override;
    void rejected() override;

private:
    custom_handlers::ProtocolHandlerRegistry *protocolHandlerRegistry();
    content::ProtocolHandler m_handler;
};

} // namespace QtWebEngineCore

#endif // REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H
