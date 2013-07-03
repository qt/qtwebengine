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

#include "url_request_context_getter_qt.h"

#include "base/strings/string_util.h"
#include "base/threading/worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/cache_type.h"
#include "net/cert/cert_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/dns/mapped_host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/default_server_bound_cert_store.h"
#include "net/ssl/server_bound_cert_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"

#include "network_delegate_qt.h"

using content::BrowserThread;

URLRequestContextGetterQt::URLRequestContextGetterQt(const base::FilePath &basePath)
    : m_ignoreCertificateErrors(false)
    , m_basePath(basePath)
{

    // We must create the proxy config service on the UI loop on Linux because it
    // must synchronously run on the glib message loop. This will be passed to
    // the URLRequestContextStorage on the IO thread in GetURLRequestContext().
//#ifdef Q_OS_LINUX
    m_proxyConfigService.reset(net::ProxyService::CreateSystemProxyConfigService(BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO)->message_loop_proxy()
                                                                                 , BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE)));
//#endif
}

net::URLRequestContext *URLRequestContextGetterQt::GetURLRequestContext()
{
    if (!m_urlRequestContext) {

        m_urlRequestContext.reset(new net::URLRequestContext());
        m_networkDelegate.reset(new NetworkDelegateQt);

        m_urlRequestContext->set_network_delegate(m_networkDelegate.get());
        m_storage.reset(new net::URLRequestContextStorage(m_urlRequestContext.get()));
        m_storage->set_cookie_store(new net::CookieMonster(NULL, NULL));
        m_storage->set_server_bound_cert_service(new net::ServerBoundCertService(
            new net::DefaultServerBoundCertStore(NULL),
            base::WorkerPool::GetTaskRunner(true)));
        m_storage->set_http_user_agent_settings(
            new net::StaticHttpUserAgentSettings("en-us,en", EmptyString()));

        scoped_ptr<net::HostResolver> host_resolver(
            net::HostResolver::CreateDefaultResolver(NULL));

        m_storage->set_cert_verifier(net::CertVerifier::CreateDefault());

        m_storage->set_proxy_service(net::ProxyService::CreateUsingSystemProxyResolver(m_proxyConfigService.release(), 0, NULL));

        m_storage->set_ssl_config_service(new net::SSLConfigServiceDefaults);
        m_storage->set_transport_security_state(new net::TransportSecurityState());

        m_storage->set_http_auth_handler_factory(
            net::HttpAuthHandlerFactory::CreateDefault(host_resolver.get()));
        m_storage->set_http_server_properties(new net::HttpServerPropertiesImpl);

        base::FilePath cache_path = m_basePath.Append(FILE_PATH_LITERAL("Cache"));
        net::HttpCache::DefaultBackend* main_backend =
            new net::HttpCache::DefaultBackend(
                net::DISK_CACHE,
                net::CACHE_BACKEND_DEFAULT,
                cache_path,
                0,
                BrowserThread::GetMessageLoopProxyForThread(
                    BrowserThread::CACHE));

        net::HttpNetworkSession::Params network_session_params;
        network_session_params.transport_security_state =
            m_urlRequestContext->transport_security_state();
        network_session_params.cert_verifier =
            m_urlRequestContext->cert_verifier();
        network_session_params.server_bound_cert_service =
            m_urlRequestContext->server_bound_cert_service();
        network_session_params.proxy_service =
            m_urlRequestContext->proxy_service();
        network_session_params.ssl_config_service =
            m_urlRequestContext->ssl_config_service();
        network_session_params.http_auth_handler_factory =
            m_urlRequestContext->http_auth_handler_factory();
        network_session_params.network_delegate =
            m_networkDelegate.get();
        network_session_params.http_server_properties =
            m_urlRequestContext->http_server_properties();
        network_session_params.ignore_certificate_errors =
            m_ignoreCertificateErrors;

        // Give |m_storage| ownership at the end in case it's |mapped_host_resolver|.
        m_storage->set_host_resolver(host_resolver.Pass());
        network_session_params.host_resolver =
            m_urlRequestContext->host_resolver();

        net::HttpCache* main_cache = new net::HttpCache(
            network_session_params, main_backend);
        m_storage->set_http_transaction_factory(main_cache);

        // FIXME: add protocol handling

        m_jobFactory.reset(new net::URLRequestJobFactoryImpl());
        m_jobFactory->SetProtocolHandler(chrome::kDataScheme, new net::DataProtocolHandler());
        m_jobFactory->SetProtocolHandler(chrome::kFileScheme, new net::FileProtocolHandler());
        m_urlRequestContext->set_job_factory(m_jobFactory.get());
    }

    return m_urlRequestContext.get();
}


scoped_refptr<base::SingleThreadTaskRunner> URLRequestContextGetterQt::GetNetworkTaskRunner() const
{
    return content::BrowserThread::GetMessageLoopProxyForThread(content::BrowserThread::IO);
}
