// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H
#define REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H

#include "register_protocol_handler_request_controller.h"

#include "content/public/browser/web_contents_observer.h"
#include "components/custom_handlers/protocol_handler.h"

namespace custom_handlers {
class ProtocolHandlerRegistry;
}

namespace QtWebEngineCore {

class RegisterProtocolHandlerRequestControllerImpl final : public RegisterProtocolHandlerRequestController,
                                                           private content::WebContentsObserver {
public:
    RegisterProtocolHandlerRequestControllerImpl(
        content::WebContents *webContents,
        custom_handlers::ProtocolHandler handler);

    ~RegisterProtocolHandlerRequestControllerImpl();

protected:
    void accepted() override;
    void rejected() override;

private:
    custom_handlers::ProtocolHandlerRegistry *protocolHandlerRegistry();
    custom_handlers::ProtocolHandler m_handler;
};

} // namespace QtWebEngineCore

#endif // REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_IMPL_H
