// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DEV_TOOLS_HTTP_HANDLER_DELEGATE_QT_H
#define DEV_TOOLS_HTTP_HANDLER_DELEGATE_QT_H

#include "content/public/browser/devtools_manager_delegate.h"

#include <QString>

namespace content {
class DevToolsSocketFactory;
}

namespace QtWebEngineCore {

class DevToolsServerQt {
public:
    DevToolsServerQt();
    ~DevToolsServerQt();

    bool isValid() const { return m_valid; }
    QString bindAddress() const { return m_bindAddress; }
    int port() const { return m_port; }

    void start();
    void stop();
    bool isStarted() const { return m_isStarted; }

private:
    void parseAddressAndPort();
    std::unique_ptr<content::DevToolsSocketFactory> CreateSocketFactory();

    QString m_bindAddress;
    int m_port;
    bool m_valid;
    bool m_isStarted;
};


class DevToolsManagerDelegateQt : public content::DevToolsManagerDelegate {
public:
    std::string GetDiscoveryPageHTML() override;
    bool HasBundledFrontendResources() override;

    void Initialized(const net::IPEndPoint *ip_address) override;
};

} // namespace QtWebEngineCore

#endif // DEV_TOOLS_HTTP_HANDLER_DELEGATE_QT_H
