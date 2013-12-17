/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "dev_tools_http_handler_delegate_qt.h"

#include <QByteArray>
#include <QFile>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/strings/string_number_conversions.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/common/content_switches.h"
#include "net/socket/stream_listen_socket.h"
#include "net/socket/tcp_listen_socket.h"

DevToolsHttpHandlerDelegateQt::DevToolsHttpHandlerDelegateQt(content::BrowserContext* browser_context)
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
    m_devtoolsHttpHandler = content::DevToolsHttpHandler::Start(new net::TCPListenSocketFactory("0.0.0.0", listeningPort), std::string(), this);
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

std::string DevToolsHttpHandlerDelegateQt::GetPageThumbnailData(const GURL& url)
{
    return std::string();
}

content::RenderViewHost* DevToolsHttpHandlerDelegateQt::CreateNewTarget()
{
    return NULL;
}

content::DevToolsHttpHandlerDelegate::TargetType DevToolsHttpHandlerDelegateQt::GetTargetType(content::RenderViewHost*)
{
    return kTargetTypeTab;
}

std::string DevToolsHttpHandlerDelegateQt::GetViewDescription(content::RenderViewHost*)
{
    return std::string();
}

scoped_ptr<net::StreamListenSocket> DevToolsHttpHandlerDelegateQt::CreateSocketForTethering(net::StreamListenSocket::Delegate* delegate, std::string* name)
{
    return scoped_ptr<net::StreamListenSocket>();
}
