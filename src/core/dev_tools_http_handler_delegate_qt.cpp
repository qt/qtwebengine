/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dev_tools_http_handler_delegate_qt.h"

#include <QByteArray>
#include <QFile>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/content_switches.h"
#include "net/socket/stream_listen_socket.h"
#include "net/socket/tcp_server_socket.h"

using namespace content;

namespace {

const char kTargetTypePage[] = "page";

class TCPServerSocketFactory
    : public DevToolsHttpHandler::ServerSocketFactory {
public:
    TCPServerSocketFactory(const std::string& address, int port, int backlog)
        : DevToolsHttpHandler::ServerSocketFactory(address, port, backlog) {}
private:
    scoped_ptr<net::ServerSocket> Create() const override {
        return scoped_ptr<net::ServerSocket>(new net::TCPServerSocket(NULL, net::NetLog::Source()));
  }
  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

class Target : public content::DevToolsTarget {
public:
    explicit Target(WebContents* web_contents);

    virtual std::string GetId() const override { return id_; }
    virtual std::string GetParentId() const override { return std::string(); }
    virtual std::string GetType() const override { return kTargetTypePage; }
    virtual std::string GetTitle() const override { return title_; }
    virtual std::string GetDescription() const override { return std::string(); }
    virtual GURL GetURL() const override { return url_; }
    virtual GURL GetFaviconURL() const override { return favicon_url_; }
    virtual base::TimeTicks GetLastActivityTime() const override {
        return last_activity_time_;
    }
    virtual bool IsAttached() const override {
        return agent_host_->IsAttached();
    }
    virtual scoped_refptr<DevToolsAgentHost> GetAgentHost() const override {
        return agent_host_;
    }
    virtual bool Activate() const override;
    virtual bool Close() const override;

private:
    scoped_refptr<DevToolsAgentHost> agent_host_;
    std::string id_;
    std::string title_;
    GURL url_;
    GURL favicon_url_;
    base::TimeTicks last_activity_time_;
};

Target::Target(WebContents* web_contents) {
    agent_host_ = DevToolsAgentHost::GetOrCreateFor(web_contents);
    id_ = agent_host_->GetId();
    title_ = base::UTF16ToUTF8(web_contents->GetTitle());
    url_ = web_contents->GetURL();
    content::NavigationController& controller = web_contents->GetController();
    content::NavigationEntry* entry = controller.GetActiveEntry();
    if (entry != NULL && entry->GetURL().is_valid())
        favicon_url_ = entry->GetFavicon().url;
    last_activity_time_ = web_contents->GetLastActiveTime();
}

bool Target::Activate() const {
    WebContents *web_contents = agent_host_->GetWebContents();
    if (!web_contents)
        return false;
    web_contents->GetDelegate()->ActivateContents(web_contents);
    return true;
}

bool Target::Close() const {
    WebContents *web_contents = agent_host_->GetWebContents();
    if (!web_contents)
        return false;
    RenderViewHost* rvh = web_contents->GetRenderViewHost();
    if (!rvh)
        return false;
    rvh->ClosePage();
    return true;
}

}  // namespace

DevToolsHttpHandlerDelegateQt::DevToolsHttpHandlerDelegateQt(BrowserContext* browser_context)
    : m_browserContext(browser_context)
{
    const int defaultPort = 1337;
    int listeningPort = defaultPort;
    const CommandLine &commandLine = *CommandLine::ForCurrentProcess();
    if (commandLine.HasSwitch(switches::kRemoteDebuggingPort)) {
        std::string portString =
            commandLine.GetSwitchValueASCII(switches::kRemoteDebuggingPort);
        int portInt = 0;
        if (base::StringToInt(portString, &portInt) && portInt > 0 && portInt < 65535)
            listeningPort = portInt;
    }
    scoped_ptr<content::DevToolsHttpHandler::ServerSocketFactory> factory(new TCPServerSocketFactory("0.0.0.0", listeningPort, 1));
    m_devtoolsHttpHandler = DevToolsHttpHandler::Start(factory.Pass(), std::string(), this, base::FilePath());
}

DevToolsHttpHandlerDelegateQt::~DevToolsHttpHandlerDelegateQt()
{
    m_devtoolsHttpHandler->Stop();
}

std::string DevToolsHttpHandlerDelegateQt::GetDiscoveryPageHTML()
{
    static std::string html;
    if (html.empty()) {
        QFile html_file(":/data/discovery_page.html");
        html_file.open(QIODevice::ReadOnly);
        QByteArray contents = html_file.readAll();
        html = contents.data();
    }
    return html;
}

bool DevToolsHttpHandlerDelegateQt::BundlesFrontendResources()
{
    return true;
}

base::FilePath DevToolsHttpHandlerDelegateQt::GetDebugFrontendDir()
{
    return base::FilePath();
}

scoped_ptr<net::StreamListenSocket> DevToolsHttpHandlerDelegateQt::CreateSocketForTethering(net::StreamListenSocket::Delegate* delegate, std::string* name)
{
    return scoped_ptr<net::StreamListenSocket>();
}
