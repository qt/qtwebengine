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

#ifndef URL_REQUEST_CONTEXT_GETTER_QT_H
#define URL_REQUEST_CONTEXT_GETTER_QT_H

#include "net/url_request/url_request_context_getter.h"

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/single_thread_task_runner.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/common/url_constants.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "net/proxy/dhcp_proxy_script_fetcher_factory.h"

#include "qglobal.h"
#include <qatomic.h>

namespace net {
class MappedHostResolver;
class NetworkDelegate;
class ProxyConfigService;
}

namespace QtWebEngineCore {

class BrowserContextAdapter;

class URLRequestContextGetterQt : public net::URLRequestContextGetter {
public:
    explicit URLRequestContextGetterQt(BrowserContextAdapter *browserContext, content::ProtocolHandlerMap *protocolHandlers);

    virtual net::URLRequestContext *GetURLRequestContext() Q_DECL_OVERRIDE;
    virtual scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const Q_DECL_OVERRIDE;

    // Called on the UI thread:
    void updateStorageSettings();
    void updateUserAgent();
    void updateCookieStore();
    void updateHttpCache();

private:
    virtual ~URLRequestContextGetterQt();

    // Called on the IO thread:
    void generateStorage();
    void generateCookieStore();
    void generateHttpCache();
    void generateUserAgent();
    void generateJobFactory();

    bool m_ignoreCertificateErrors;
    QAtomicInt m_updateCookieStore;
    QAtomicInt m_updateHttpCache;
    BrowserContextAdapter *m_browserContext;
    content::ProtocolHandlerMap m_protocolHandlers;

    QAtomicPointer<net::ProxyConfigService> m_proxyConfigService;
    scoped_ptr<net::URLRequestContext> m_urlRequestContext;
    scoped_ptr<net::NetworkDelegate> m_networkDelegate;
    scoped_ptr<net::URLRequestContextStorage> m_storage;
    scoped_ptr<net::URLRequestJobFactoryImpl> m_jobFactory;
    scoped_ptr<net::DhcpProxyScriptFetcherFactory> m_dhcpProxyScriptFetcherFactory;
};

} // namespace QtWebEngineCore

#endif // URL_REQUEST_CONTEXT_GETTER_QT_H
