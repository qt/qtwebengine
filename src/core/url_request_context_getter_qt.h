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
#include "net/http/http_network_session.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "net/proxy/dhcp_proxy_script_fetcher_factory.h"

#include "cookie_monster_delegate_qt.h"
#include "network_delegate_qt.h"
#include "browser_context_adapter.h"

#include <QtCore/qatomic.h>
#include <QtCore/qmutex.h>
#include <QtCore/qsharedpointer.h>

namespace net {
class MappedHostResolver;
class ProxyConfigService;
}

namespace QtWebEngineCore {

// FIXME: This class should be split into a URLRequestContextGetter and a ProfileIOData, similar to what chrome does.
class URLRequestContextGetterQt : public net::URLRequestContextGetter {
public:
    URLRequestContextGetterQt(QSharedPointer<BrowserContextAdapter> browserContext, content::ProtocolHandlerMap *protocolHandlers, content::URLRequestInterceptorScopedVector request_interceptors);

    virtual net::URLRequestContext *GetURLRequestContext() Q_DECL_OVERRIDE;
    virtual scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const Q_DECL_OVERRIDE;

    // Called on the UI thread:
    void updateStorageSettings();
    void updateUserAgent();
    void updateCookieStore();
    void updateHttpCache();
    void updateJobFactory();
    void updateRequestInterceptor();

private:
    virtual ~URLRequestContextGetterQt();

    // Called on the IO thread:
    void generateAllStorage();
    void generateStorage();
    void generateCookieStore();
    void generateHttpCache();
    void generateUserAgent();
    void generateJobFactory();
    void regenerateJobFactory();
    void cancelAllUrlRequests();
    net::HttpNetworkSession::Params generateNetworkSessionParams();

    void setFullConfiguration(QSharedPointer<BrowserContextAdapter> browserContext);

    bool m_ignoreCertificateErrors;

    QMutex m_mutex;
    bool m_contextInitialized;
    bool m_updateAllStorage;
    bool m_updateCookieStore;
    bool m_updateHttpCache;
    bool m_updateJobFactory;
    bool m_updateUserAgent;

    QWeakPointer<BrowserContextAdapter> m_browserContext;
    content::ProtocolHandlerMap m_protocolHandlers;

    QAtomicPointer<net::ProxyConfigService> m_proxyConfigService;
    scoped_ptr<net::URLRequestContext> m_urlRequestContext;
    scoped_ptr<NetworkDelegateQt> m_networkDelegate;
    scoped_ptr<net::URLRequestContextStorage> m_storage;
    scoped_ptr<net::URLRequestJobFactory> m_jobFactory;
    net::URLRequestJobFactoryImpl *m_baseJobFactory;
    scoped_ptr<net::DhcpProxyScriptFetcherFactory> m_dhcpProxyScriptFetcherFactory;
    scoped_refptr<CookieMonsterDelegateQt> m_cookieDelegate;
    content::URLRequestInterceptorScopedVector m_requestInterceptors;

    QList<QByteArray> m_installedCustomSchemes;
    QWebEngineUrlRequestInterceptor* m_requestInterceptor;

    // Configuration values to setup URLRequestContext in IO thread, copied from browserContext
    // FIXME: Should later be moved to a separate ProfileIOData class.
    BrowserContextAdapter::PersistentCookiesPolicy m_persistentCookiesPolicy;
    QString m_cookiesPath;
    QString m_httpAcceptLanguage;
    QString m_httpUserAgent;
    BrowserContextAdapter::HttpCacheType m_httpCacheType;
    QString m_httpCachePath;
    int m_httpCacheMaxSize;
    QList<QByteArray> m_customUrlSchemes;

    friend class NetworkDelegateQt;
};

} // namespace QtWebEngineCore

#endif // URL_REQUEST_CONTEXT_GETTER_QT_H
