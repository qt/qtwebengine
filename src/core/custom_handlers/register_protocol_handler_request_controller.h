// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_H
#define REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_H

#include "request_controller.h"

namespace QtWebEngineCore {

class RegisterProtocolHandlerRequestController : public RequestController {
public:
    RegisterProtocolHandlerRequestController(QUrl origin, QString scheme)
        : RequestController(std::move(origin))
        , m_scheme(std::move(scheme))
    {}

    QString scheme() const { return m_scheme; }

private:
    QString m_scheme;
};

} // namespace QtWebEngineCore

#endif // REGISTER_PROTOCOL_HANDLER_REQUEST_CONTROLLER_H
