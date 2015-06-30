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

#include "url_request_context_getter_qt.h"

#include "base/command_line.h"
#include "base/strings/string_util.h"
#include "base/threading/worker_pool.h"
#include "base/threading/sequenced_worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_crypto_delegate.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/common/content_switches.h"
#include "net/base/cache_type.h"
#include "net/cert/cert_verifier.h"
#include "net/dns/host_resolver.h"
#include "net/dns/mapped_host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/proxy/dhcp_proxy_script_fetcher_factory.h"
#include "net/proxy/proxy_script_fetcher_impl.h"
#include "net/proxy/proxy_service.h"
#include "net/proxy/proxy_service_v8.h"
#include "net/proxy/proxy_resolver_v8.h"
#include "net/ssl/channel_id_service.h"
#include "net/ssl/default_channel_id_store.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/ftp_protocol_handler.h"
#include "net/ftp/ftp_network_layer.h"

#include "browser_context_adapter.h"
#include "custom_protocol_handler.h"
#include "custom_url_scheme_handler.h"
#include "content_client_qt.h"
#include "network_delegate_qt.h"
#include "qrc_protocol_handler_qt.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

static const char kQrcSchemeQt[] = "qrc";

using content::BrowserThread;

URLRequestContextGetterQt::URLRequestContextGetterQt(BrowserContextAdapter *browserContext, content::ProtocolHandlerMap *protocolHandlers)
    : m_ignoreCertificateErrors(false)
    , m_browserContext(browserContext)
{
    std::swap(m_protocolHandlers, *protocolHandlers);

    updateStorageSettings();
}

URLRequestContextGetterQt::~URLRequestContextGetterQt()
{
    delete m_proxyConfigService.fetchAndStoreAcquire(0);
}

net::URLRequestContext *URLRequestContextGetterQt::GetURLRequestContext()
{
    if (!m_urlRequestContext) {
        m_urlRequestContext.reset(new net::URLRequestContext());

        m_networkDelegate.reset(new NetworkDelegateQt);
        m_urlRequestContext->set_network_delegate(m_networkDelegate.get());

        generateStorage();
        generateJobFactory();
    }

    return m_urlRequestContext.get();
}

void URLRequestContextGetterQt::updateStorageSettings()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    if (!m_proxyConfigService.loadAcquire()) {
        // We must create the proxy config service on the UI loop on Linux because it
        // must synchronously run on the glib message loop. This will be passed to
        // the URLRequestContextStorage on the IO thread in GetURLRequestContext().
        m_proxyConfigService = net::ProxyService::CreateSystemProxyConfigService(
            content::BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO),
            content::BrowserThread::GetMessageLoopProxyForThread(BrowserThread::FILE)
        );
        if (m_storage)
            content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestContextGetterQt::generateStorage, this));
    }
}

void URLRequestContextGetterQt::generateStorage()
{
    Q_ASSERT(m_urlRequestContext);

    if (m_storage) {
        // We must stop all requests before deleting their backends.
        std::set<const net::URLRequest*>* url_requests = m_urlRequestContext->url_requests();
        std::set<const net::URLRequest*>::const_iterator it = url_requests->begin();
        std::set<const net::URLRequest*>::const_iterator end = url_requests->end();
        for ( ; it != end; ++it) {
            net::URLRequest* request = const_cast<net::URLRequest*>(*it);
            if (request)
                request->Cancel();
        }
    }

    m_storage.reset(new net::URLRequestContextStorage(m_urlRequestContext.get()));

    net::ProxyConfigService *proxyConfigService = m_proxyConfigService.fetchAndStoreAcquire(0);
    Q_ASSERT(proxyConfigService);

    generateCookieStore();
    generateUserAgent();

    m_storage->set_channel_id_service(new net::ChannelIDService(
        new net::DefaultChannelIDStore(NULL),
        base::WorkerPool::GetTaskRunner(true)));

    m_storage->set_cert_verifier(net::CertVerifier::CreateDefault());

    scoped_ptr<net::HostResolver> host_resolver(net::HostResolver::CreateDefaultResolver(NULL));

    // The System Proxy Resolver has issues on Windows with unconfigured network cards,
    // which is why we want to use the v8 one. However, V8ProxyResolver doesn't work reliably with
    // --single-process (See also the warnings in net/proxy/proxy_resolver_v8.h).
    base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
    if (command_line.HasSwitch(switches::kSingleProcess)) {
        m_storage->set_proxy_service(
                    net::ProxyService::CreateUsingSystemProxyResolver(proxyConfigService, 0, NULL));
    } else {
        if (!m_dhcpProxyScriptFetcherFactory)
            m_dhcpProxyScriptFetcherFactory.reset(new net::DhcpProxyScriptFetcherFactory);

        net::ProxyResolverV8::EnsureIsolateCreated();
        m_storage->set_proxy_service(net::CreateProxyServiceUsingV8ProxyResolver(
                                         proxyConfigService,
                                         new net::ProxyScriptFetcherImpl(m_urlRequestContext.get()),
                                         m_dhcpProxyScriptFetcherFactory->Create(m_urlRequestContext.get()),
                                         host_resolver.get(),
                                         NULL /* NetLog */,
                                         m_networkDelegate.get()));
    }
    m_storage->set_ssl_config_service(new net::SSLConfigServiceDefaults);
    m_storage->set_transport_security_state(new net::TransportSecurityState());

    m_storage->set_http_auth_handler_factory(net::HttpAuthHandlerFactory::CreateDefault(host_resolver.get()));
    m_storage->set_http_server_properties(scoped_ptr<net::HttpServerProperties>(new net::HttpServerPropertiesImpl));

     // Give |m_storage| ownership at the end in case it's |mapped_host_resolver|.
    m_storage->set_host_resolver(host_resolver.Pass());

    generateHttpCache();
}

void URLRequestContextGetterQt::updateCookieStore()
{
    if (m_urlRequestContext && !m_updateCookieStore && !m_proxyConfigService) {
        m_updateCookieStore = 1;
        content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestContextGetterQt::generateCookieStore, this));
    }
}

void URLRequestContextGetterQt::generateCookieStore()
{
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(m_storage);
    m_updateCookieStore = 0;

    // Unset it first to get a chance to destroy and flush the old cookie store before before opening a new on possibly the same file.
    m_storage->set_cookie_store(0);

    net::CookieStore* cookieStore = 0;
    switch (m_browserContext->persistentCookiesPolicy()) {
    case BrowserContextAdapter::NoPersistentCookies:
        cookieStore =
            content::CreateCookieStore(content::CookieStoreConfig(
                base::FilePath(),
                content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES,
                NULL, NULL)
            );
        break;
    case BrowserContextAdapter::AllowPersistentCookies:
        cookieStore =
            content::CreateCookieStore(content::CookieStoreConfig(
                toFilePath(m_browserContext->cookiesPath()),
                content::CookieStoreConfig::PERSISTANT_SESSION_COOKIES,
                NULL, NULL)
            );
        break;
    case BrowserContextAdapter::ForcePersistentCookies:
        cookieStore =
            content::CreateCookieStore(content::CookieStoreConfig(
                toFilePath(m_browserContext->cookiesPath()),
                content::CookieStoreConfig::RESTORED_SESSION_COOKIES,
                NULL, NULL)
            );
        break;
    }
    m_storage->set_cookie_store(cookieStore);
}

void URLRequestContextGetterQt::updateUserAgent()
{
    if (m_urlRequestContext && !m_proxyConfigService)
        content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestContextGetterQt::generateUserAgent, this));
}

void URLRequestContextGetterQt::generateUserAgent()
{
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(m_storage);

    m_storage->set_http_user_agent_settings(
        new net::StaticHttpUserAgentSettings("en-us,en", m_browserContext->httpUserAgent().toStdString()));
}

void URLRequestContextGetterQt::updateHttpCache()
{
    if (m_urlRequestContext && !m_updateHttpCache && !m_proxyConfigService) {
        m_updateHttpCache = 1;
        content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestContextGetterQt::generateHttpCache, this));
    }
}

void URLRequestContextGetterQt::generateHttpCache()
{
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(m_storage);
    m_updateHttpCache = 0;

    net::HttpCache::DefaultBackend* main_backend = 0;
    switch (m_browserContext->httpCacheType()) {
    case BrowserContextAdapter::MemoryHttpCache:
        main_backend =
            new net::HttpCache::DefaultBackend(
                net::MEMORY_CACHE,
                net::CACHE_BACKEND_DEFAULT,
                base::FilePath(),
                m_browserContext->httpCacheMaxSize(),
                BrowserThread::GetMessageLoopProxyForThread(BrowserThread::CACHE)
            );
        break;
    case BrowserContextAdapter::DiskHttpCache:
        main_backend =
            new net::HttpCache::DefaultBackend(
                net::DISK_CACHE,
                net::CACHE_BACKEND_DEFAULT,
                toFilePath(m_browserContext->httpCachePath()),
                m_browserContext->httpCacheMaxSize(),
                BrowserThread::GetMessageLoopProxyForThread(BrowserThread::CACHE)
            );
        break;
    }

    net::HttpNetworkSession::Params network_session_params;
    network_session_params.transport_security_state     = m_urlRequestContext->transport_security_state();
    network_session_params.cert_verifier                = m_urlRequestContext->cert_verifier();
    network_session_params.channel_id_service           = m_urlRequestContext->channel_id_service();
    network_session_params.proxy_service                = m_urlRequestContext->proxy_service();
    network_session_params.ssl_config_service           = m_urlRequestContext->ssl_config_service();
    network_session_params.http_auth_handler_factory    = m_urlRequestContext->http_auth_handler_factory();
    network_session_params.network_delegate             = m_networkDelegate.get();
    network_session_params.http_server_properties       = m_urlRequestContext->http_server_properties();
    network_session_params.ignore_certificate_errors    = m_ignoreCertificateErrors;
    network_session_params.host_resolver                = m_urlRequestContext->host_resolver();

    m_storage->set_http_transaction_factory(new net::HttpCache(network_session_params, main_backend));
}

void URLRequestContextGetterQt::generateJobFactory()
{
    Q_ASSERT(m_urlRequestContext);
    Q_ASSERT(!m_jobFactory);
    m_jobFactory.reset(new net::URLRequestJobFactoryImpl());

    // Chromium has a few protocol handlers ready for us, only pick blob: and throw away the rest.
    content::ProtocolHandlerMap::iterator it = m_protocolHandlers.find(url::kBlobScheme);
    Q_ASSERT(it != m_protocolHandlers.end());
    m_jobFactory->SetProtocolHandler(it->first, it->second.release());
    m_protocolHandlers.clear();

    m_jobFactory->SetProtocolHandler(url::kDataScheme, new net::DataProtocolHandler());
    m_jobFactory->SetProtocolHandler(url::kFileScheme, new net::FileProtocolHandler(
        content::BrowserThread::GetBlockingPool()->GetTaskRunnerWithShutdownBehavior(
            base::SequencedWorkerPool::SKIP_ON_SHUTDOWN)));
    m_jobFactory->SetProtocolHandler(kQrcSchemeQt, new QrcProtocolHandlerQt());
    m_jobFactory->SetProtocolHandler(url::kFtpScheme,
        new net::FtpProtocolHandler(new net::FtpNetworkLayer(m_urlRequestContext->host_resolver())));

    Q_FOREACH (CustomUrlSchemeHandler* handler, m_browserContext->customUrlSchemeHandlers()) {
        m_jobFactory->SetProtocolHandler(handler->scheme().toStdString(), handler->createProtocolHandler());
    }

    m_urlRequestContext->set_job_factory(m_jobFactory.get());
}

scoped_refptr<base::SingleThreadTaskRunner> URLRequestContextGetterQt::GetNetworkTaskRunner() const
{
    return content::BrowserThread::GetMessageLoopProxyForThread(content::BrowserThread::IO);
}

} // namespace QtWebEngineCore
