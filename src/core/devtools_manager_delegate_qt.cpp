// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on content/shell/browser/shell_devtools_manager_delegate.cc:
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "devtools_manager_delegate_qt.h"
#include "qtwebengine/grit/qt_webengine_resources.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "content/browser/devtools/devtools_http_handler.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_socket_factory.h"
#include "content/public/common/content_switches.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/socket/tcp_server_socket.h"
#include "ui/base/resource/resource_bundle.h"

using content::DevToolsAgentHost;

namespace {

class TCPServerSocketFactory : public content::DevToolsSocketFactory {
public:
    TCPServerSocketFactory(const std::string& address, int port, int backlog)
        : m_address(address), m_port(port), m_backlog(backlog)
    {}
private:
    std::unique_ptr<net::ServerSocket> CreateForHttpServer() override {
        std::unique_ptr<net::ServerSocket> socket(new net::TCPServerSocket(nullptr, net::NetLogSource()));
        if (socket->ListenWithAddressAndPort(m_address, m_port, m_backlog) != net::OK)
            return std::unique_ptr<net::ServerSocket>();

        return socket;
    }
    std::unique_ptr<net::ServerSocket> CreateForTethering(std::string* out_name) override
    {
          return nullptr;
    }

    const std::string m_address;
    int m_port;
    int m_backlog;
};

}  // namespace

namespace QtWebEngineCore {

DevToolsServerQt::DevToolsServerQt()
    : m_bindAddress(QLatin1String("127.0.0.1"))
    , m_port(0)
    , m_valid(false)
    , m_isStarted(false)
{ }

DevToolsServerQt::~DevToolsServerQt()
{
    stop();
}

void DevToolsServerQt::parseAddressAndPort()
{
    const QString inspectorEnv = qEnvironmentVariable("QTWEBENGINE_REMOTE_DEBUGGING");
    const base::CommandLine &commandLine = *base::CommandLine::ForCurrentProcess();
    QString portStr;

    if (commandLine.HasSwitch(switches::kRemoteDebuggingPort)) {
        portStr = QString::fromStdString(commandLine.GetSwitchValueASCII(switches::kRemoteDebuggingPort));
    } else if (!inspectorEnv.isEmpty()) {
        int portColonPos = inspectorEnv.lastIndexOf(':');
        if (portColonPos != -1) {
            portStr = inspectorEnv.mid(portColonPos + 1);
            m_bindAddress = inspectorEnv.mid(0, portColonPos);
        } else
            portStr = inspectorEnv;
    } else
        return;

    m_port = portStr.toInt(&m_valid);
    m_valid = m_valid && (m_port > 0 && m_port < 65535);
    if (!m_valid)
        qWarning("Invalid port given for the inspector server \"%s\". Examples of valid input: \"12345\" or \"192.168.2.14:12345\" (with the address of one of this host's network interface).", qPrintable(portStr));
}

std::unique_ptr<content::DevToolsSocketFactory> DevToolsServerQt::CreateSocketFactory()
{
    if (!m_valid)
        return nullptr;
    return std::unique_ptr<content::DevToolsSocketFactory>(
        new TCPServerSocketFactory(m_bindAddress.toStdString(), m_port, 1));
}


void DevToolsServerQt::start()
{
    if (m_isStarted)
        return;

    if (!m_valid)
        parseAddressAndPort();

    std::unique_ptr<content::DevToolsSocketFactory> socketFactory = CreateSocketFactory();
    if (!socketFactory)
        return;

    m_isStarted = true;
    DevToolsAgentHost::StartRemoteDebuggingServer(
        std::move(socketFactory),
        base::FilePath(), base::FilePath());
}

void DevToolsServerQt::stop()
{
    DevToolsAgentHost::StopRemoteDebuggingServer();
    m_isStarted = false;
}

void DevToolsManagerDelegateQt::Initialized(const net::IPEndPoint *ip_address)
{
    if (ip_address && ip_address->address().size()) {
        QString addressAndPort = QString::fromStdString(ip_address->ToString());
        qWarning("Remote debugging server started successfully. Try pointing a Chromium-based browser to http://%s", qPrintable(addressAndPort));
    }
    else
        qWarning("Couldn't start the inspector server on bind address. In case of invalid input, try something like: \"12345\" or \"192.168.2.14:12345\" (with the address of one of this host's interface).");
}

std::string DevToolsManagerDelegateQt::GetDiscoveryPageHTML()
{
    return ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(IDR_DEVTOOLS_DISCOVERY_PAGE_HTML);
}

bool DevToolsManagerDelegateQt::HasBundledFrontendResources()
{
    return true;
}

} // namespace QtWebEngineCore
