/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DEV_TOOLS_HTTP_HANDLER_DELEGATE_QT_H
#define DEV_TOOLS_HTTP_HANDLER_DELEGATE_QT_H

#include "components/devtools_http_handler/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_manager_delegate.h"

#include <QString>
#include <QtCore/qcompilerdetection.h> // needed for Q_DECL_OVERRIDE

namespace content {
class BrowserContext;
}

namespace devtools_http_handler {
class DevToolsHttpHandler;
}

namespace QtWebEngineCore {

scoped_ptr<devtools_http_handler::DevToolsHttpHandler> createDevToolsHttpHandler();

class DevToolsHttpHandlerDelegateQt : public devtools_http_handler::DevToolsHttpHandlerDelegate {
public:
    DevToolsHttpHandlerDelegateQt();

    bool isValid() const { return m_valid; }
    QString bindAddress() const { return m_bindAddress; }
    int port() const { return m_port; }

    // devtools_http_handler::DevToolsHttpHandlerDelegate Overrides
    void Initialized(const net::IPEndPoint *ip_address) Q_DECL_OVERRIDE;
    std::string GetDiscoveryPageHTML() Q_DECL_OVERRIDE;
    std::string GetFrontendResource(const std::string&)  Q_DECL_OVERRIDE;
    std::string GetPageThumbnailData(const GURL &url) Q_DECL_OVERRIDE;

private:
    QString m_bindAddress;
    int m_port;
    bool m_valid;
};

class DevToolsManagerDelegateQt : public content::DevToolsManagerDelegate {
public:
    void Inspect(content::BrowserContext *browser_context, content::DevToolsAgentHost *agent_host) Q_DECL_OVERRIDE { }
    void DevToolsAgentStateChanged(content::DevToolsAgentHost *agent_host, bool attached) Q_DECL_OVERRIDE { }
    base::DictionaryValue *HandleCommand(content::DevToolsAgentHost *agent_host, base::DictionaryValue *command) Q_DECL_OVERRIDE;
};

} // namespace QtWebEngineCore

#endif // DEV_TOOLS_HTTP_HANDLER_DELEGATE_QT_H
