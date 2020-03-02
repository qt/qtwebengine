/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "profile_io_data_qt.h"

#include "base/task/post_task.h"
#include "content/browser/storage_partition_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browsing_data_remover.h"
#include "content/public/browser/shared_cors_origin_access_list.h"
#include "content/public/common/content_features.h"
#include "chrome/browser/net/chrome_mojo_proxy_resolver_factory.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "net/proxy_resolution/proxy_resolution_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "services/network/proxy_service_mojo.h"
#include "services/network/public/cpp/features.h"
#include "services/network/public/cpp/cors/origin_access_list.h"

#include "net/client_cert_override.h"
#include "net/client_cert_store_data.h"
#include "net/cookie_monster_delegate_qt.h"
#include "net/proxy_config_service_qt.h"
#include "net/system_network_context_manager.h"
#include "profile_qt.h"
#include "resource_context_qt.h"
#include "type_conversion.h"

#include <mutex>
#include <QVariant>

namespace QtWebEngineCore {

ProfileIODataQt::ProfileIODataQt(ProfileQt *profile)
    : m_profile(profile),
#if QT_CONFIG(ssl)
      m_clientCertificateStoreData(new ClientCertificateStoreData),
#endif
      m_removerObserver(this),
      m_weakPtrFactory(this)
{
    if (content::BrowserThread::IsThreadInitialized(content::BrowserThread::UI))
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

ProfileIODataQt::~ProfileIODataQt()
{
    if (content::BrowserThread::IsThreadInitialized(content::BrowserThread::IO))
        DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    m_resourceContext.reset();
    delete m_proxyConfigService.fetchAndStoreAcquire(0);
}

QPointer<ProfileAdapter> ProfileIODataQt::profileAdapter()
{
    return m_profileAdapter;
}

void ProfileIODataQt::shutdownOnUIThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if QT_CONFIG(ssl)
    delete m_clientCertificateStoreData;
    m_clientCertificateStoreData = nullptr;
#endif
    if (m_cookieDelegate)
        m_cookieDelegate->unsetMojoCookieManager();
    m_proxyConfigMonitor.reset();
    bool posted = content::BrowserThread::DeleteSoon(content::BrowserThread::IO, FROM_HERE, this);
    if (!posted) {
        qWarning() << "Could not delete ProfileIODataQt on io thread !";
        delete this;
    }
}

net::URLRequestContext *ProfileIODataQt::urlRequestContext()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_initialized)
        initializeOnIOThread();
    return nullptr;
}

content::ResourceContext *ProfileIODataQt::resourceContext()
{
    return m_resourceContext.get();
}

#if BUILDFLAG(ENABLE_EXTENSIONS)
extensions::ExtensionSystemQt* ProfileIODataQt::GetExtensionSystem()
{
    return m_profile->GetExtensionSystem();
}
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

base::WeakPtr<ProfileIODataQt> ProfileIODataQt::getWeakPtrOnUIThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    DCHECK(m_initialized);
    return m_weakPtr;
}

base::WeakPtr<ProfileIODataQt> ProfileIODataQt::getWeakPtrOnIOThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    return m_weakPtrFactory.GetWeakPtr();
}

void ProfileIODataQt::initializeOnIOThread()
{
    // this binds factory to io thread
    m_weakPtr = m_weakPtrFactory.GetWeakPtr();
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    generateAllStorage();
//    generateJobFactory();
    setGlobalCertificateVerification();
    m_initialized = true;
}

void ProfileIODataQt::initializeOnUIThread()
{
    m_profileAdapter = m_profile->profileAdapter();
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    m_resourceContext.reset(new ResourceContextQt(this));
    m_cookieDelegate = new CookieMonsterDelegateQt();
    m_cookieDelegate->setClient(m_profile->profileAdapter()->cookieStore());
    if (base::FeatureList::IsEnabled(network::features::kNetworkService))
        m_proxyConfigMonitor.reset(new ProxyConfigMonitor(m_profile->GetPrefs()));
    else
        createProxyConfig();
}

void ProfileIODataQt::generateAllStorage()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    m_updateAllStorage = false;
}

void ProfileIODataQt::regenerateJobFactory()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    m_updateJobFactory = false;

    if (m_customUrlSchemes == m_installedCustomSchemes)
        return;

    m_installedCustomSchemes = m_customUrlSchemes;
}

void ProfileIODataQt::setGlobalCertificateVerification()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
}

void ProfileIODataQt::setRequestContextData(content::ProtocolHandlerMap *protocolHandlers,
                                            content::URLRequestInterceptorScopedVector request_interceptors)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    Q_ASSERT(!m_initialized);
    m_requestInterceptors = std::move(request_interceptors);
    std::swap(m_protocolHandlers, *protocolHandlers);
}

void ProfileIODataQt::setFullConfiguration()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    m_persistentCookiesPolicy = m_profileAdapter->persistentCookiesPolicy();
    m_cookiesPath = m_profileAdapter->cookiesPath();
    m_httpAcceptLanguage = m_profileAdapter->httpAcceptLanguage();
    m_httpUserAgent = m_profileAdapter->httpUserAgent();
    m_httpCacheType = m_profileAdapter->httpCacheType();
    m_httpCachePath = m_profileAdapter->httpCachePath();
    m_httpCacheMaxSize = m_profileAdapter->httpCacheMaxSize();
    m_customUrlSchemes = m_profileAdapter->customUrlSchemes();
    m_useForGlobalCertificateVerification = m_profileAdapter->isUsedForGlobalCertificateVerification();
    m_dataPath = m_profileAdapter->dataPath();
}

void ProfileIODataQt::resetNetworkContext()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    setFullConfiguration();
    content::BrowserContext::ForEachStoragePartition(
            m_profile, base::BindRepeating([](content::StoragePartition *storage) {
                auto storage_impl = static_cast<content::StoragePartitionImpl *>(storage);
                storage_impl->ResetURLLoaderFactories();
                storage_impl->ResetNetworkContext();
            }));
}

void ProfileIODataQt::requestStorageGeneration() {
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    if (m_initialized && !m_updateAllStorage) {
        m_updateAllStorage = true;
        if (!base::FeatureList::IsEnabled(network::features::kNetworkService))
            createProxyConfig();
        base::PostTask(FROM_HERE, {content::BrowserThread::IO},
                       base::BindOnce(&ProfileIODataQt::generateAllStorage, m_weakPtr));
    }
}

// TODO(miklocek): mojofy ProxyConfigServiceQt
void ProfileIODataQt::createProxyConfig()
{
    Q_ASSERT(!base::FeatureList::IsEnabled(network::features::kNetworkService));
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    // We must create the proxy config service on the UI loop on Linux because it
    // must synchronously run on the glib message loop. This will be passed to
    // the URLRequestContextStorage on the IO thread in GetURLRequestContext().
    Q_ASSERT(m_proxyConfigService == 0);
    m_proxyConfigService =
            new ProxyConfigServiceQt(
                    m_profileAdapter->profile()->GetPrefs(),
                    base::CreateSingleThreadTaskRunner({content::BrowserThread::IO}));
    //pass interface to io thread
    m_proxyResolverFactoryInterface = ChromeMojoProxyResolverFactory::CreateWithSelfOwnedReceiver();
}

void ProfileIODataQt::updateStorageSettings()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    setFullConfiguration();

    if (!m_pendingStorageRequestGeneration)
        requestStorageGeneration();
}

void ProfileIODataQt::updateCookieStore()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    m_persistentCookiesPolicy = m_profileAdapter->persistentCookiesPolicy();
    m_cookiesPath = m_profileAdapter->cookiesPath();
    if (!m_pendingStorageRequestGeneration)
        requestStorageGeneration();
}

void ProfileIODataQt::updateUserAgent()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    m_httpAcceptLanguage = m_profileAdapter->httpAcceptLanguage();
    m_httpUserAgent = m_profileAdapter->httpUserAgent();
    if (!m_pendingStorageRequestGeneration)
        requestStorageGeneration();
}

void ProfileIODataQt::updateHttpCache()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    m_httpCacheType = m_profileAdapter->httpCacheType();
    m_httpCachePath = m_profileAdapter->httpCachePath();
    m_httpCacheMaxSize = m_profileAdapter->httpCacheMaxSize();

    if (m_httpCacheType == ProfileAdapter::NoCache) {
        m_pendingStorageRequestGeneration = true;
        content::BrowsingDataRemover *remover =
                content::BrowserContext::GetBrowsingDataRemover(m_profileAdapter->profile());
        remover->AddObserver(&m_removerObserver);
        remover->RemoveAndReply(base::Time(), base::Time::Max(),
            content::BrowsingDataRemover::DATA_TYPE_CACHE,
            content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB |
                        content::BrowsingDataRemover::ORIGIN_TYPE_PROTECTED_WEB,
            &m_removerObserver);
        return;
    }
    if (!m_pendingStorageRequestGeneration)
        requestStorageGeneration();
}

void ProfileIODataQt::updateJobFactory()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);

    m_customUrlSchemes = m_profileAdapter->customUrlSchemes();

    if (m_initialized && !m_updateJobFactory) {
        m_updateJobFactory = true;
        base::PostTask(FROM_HERE, {content::BrowserThread::IO},
                       base::BindOnce(&ProfileIODataQt::regenerateJobFactory, m_weakPtr));
    }
}

void ProfileIODataQt::updateRequestInterceptor()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    m_requestInterceptor = m_profileAdapter->requestInterceptor();
    m_hasPageInterceptors = m_profileAdapter->hasPageRequestInterceptor();
    if (m_requestInterceptor)
        m_isInterceptorDeprecated = m_requestInterceptor->property("deprecated").toBool();
    else
        m_isInterceptorDeprecated = false;
    // We in this case do not need to regenerate any Chromium classes.
}

bool ProfileIODataQt::isInterceptorDeprecated() const
{
    return m_isInterceptorDeprecated;
}

QWebEngineUrlRequestInterceptor *ProfileIODataQt::acquireInterceptor()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    m_mutex.lock();
    return m_requestInterceptor;
}

QWebEngineUrlRequestInterceptor *ProfileIODataQt::requestInterceptor()
{
    return m_requestInterceptor;
}

bool ProfileIODataQt::hasPageInterceptors()
{
    // used in NetworkDelegateQt::OnBeforeURLRequest
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    return m_hasPageInterceptors;
}

void ProfileIODataQt::releaseInterceptor()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    m_mutex.unlock();
}

bool ProfileIODataQt::canSetCookie(const QUrl &firstPartyUrl, const QByteArray &cookieLine, const QUrl &url) const
{
    return m_cookieDelegate->canSetCookie(firstPartyUrl,cookieLine, url);
}

bool ProfileIODataQt::canGetCookies(const QUrl &firstPartyUrl, const QUrl &url) const
{
    return m_cookieDelegate->canGetCookies(firstPartyUrl, url);
}

void ProfileIODataQt::updateUsedForGlobalCertificateVerification()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    const std::lock_guard<QRecursiveMutex> lock(m_mutex);
    if (m_useForGlobalCertificateVerification == m_profileAdapter->isUsedForGlobalCertificateVerification())
        return;
    m_useForGlobalCertificateVerification = m_profileAdapter->isUsedForGlobalCertificateVerification();

    if (m_useForGlobalCertificateVerification)
        base::PostTask(FROM_HERE, {content::BrowserThread::IO},
                       base::BindOnce(&ProfileIODataQt::setGlobalCertificateVerification, m_weakPtr));
}

#if QT_CONFIG(ssl)
ClientCertificateStoreData *ProfileIODataQt::clientCertificateStoreData()
{
    return m_clientCertificateStoreData;
}
#endif

std::unique_ptr<net::ClientCertStore> ProfileIODataQt::CreateClientCertStore()
{
#if QT_CONFIG(ssl)
    return std::unique_ptr<net::ClientCertStore>(new ClientCertOverrideStore(m_clientCertificateStoreData));
#else
    return nullptr;
#endif
}

network::mojom::NetworkContextParamsPtr ProfileIODataQt::CreateNetworkContextParams()
{
    updateStorageSettings();

    network::mojom::NetworkContextParamsPtr network_context_params =
             SystemNetworkContextManager::GetInstance()->CreateDefaultNetworkContextParams();

    network_context_params->context_name = m_profile->profileAdapter()->storageName().toStdString();
    network_context_params->user_agent = m_httpUserAgent.toStdString();
    network_context_params->accept_language = m_httpAcceptLanguage.toStdString();

    network_context_params->enable_referrers = true;
    // Encrypted cookies requires os_crypt, which currently has issues for us on Linux.
    network_context_params->enable_encrypted_cookies = false;
//    network_context_params->proxy_resolver_factory = std::move(m_proxyResolverFactoryInterface);

    network_context_params->http_cache_enabled = m_httpCacheType != ProfileAdapter::NoCache;
    network_context_params->http_cache_max_size = m_httpCacheMaxSize;
    if (m_httpCacheType == ProfileAdapter::DiskHttpCache && !m_httpCachePath.isEmpty())
        network_context_params->http_cache_path = toFilePath(m_httpCachePath);

    if (m_persistentCookiesPolicy != ProfileAdapter::NoPersistentCookies && !m_dataPath.isEmpty()) {
        base::FilePath cookie_path = toFilePath(m_dataPath);
        cookie_path = cookie_path.AppendASCII("Cookies");
        network_context_params->cookie_path = cookie_path;

        network_context_params->restore_old_session_cookies = m_persistentCookiesPolicy == ProfileAdapter::ForcePersistentCookies;
        network_context_params->persist_session_cookies = m_persistentCookiesPolicy != ProfileAdapter::NoPersistentCookies;
    }
    if (!m_dataPath.isEmpty()) {
        network_context_params->http_server_properties_path = toFilePath(m_dataPath).AppendASCII("Network Persistent State");
        network_context_params->transport_security_persister_path = toFilePath(m_dataPath);
    }

#if !BUILDFLAG(DISABLE_FTP_SUPPORT)
    network_context_params->enable_ftp_url_support = true;
#endif  // !BUILDFLAG(DISABLE_FTP_SUPPORT)

//    network_context_params->enable_certificate_reporting = true;
//    network_context_params->enable_expect_ct_reporting = true;
    network_context_params->enforce_chrome_ct_policy = false;
    network_context_params->primary_network_context = m_useForGlobalCertificateVerification;

    if (base::FeatureList::IsEnabled(network::features::kNetworkService)) {
        // Should be initialized with existing per-profile CORS access lists.
        network_context_params->cors_origin_access_list =
            m_profile->GetSharedCorsOriginAccessList()->GetOriginAccessList().CreateCorsOriginAccessPatternsList();
    }

    m_proxyConfigMonitor->AddToNetworkContextParams(network_context_params.get());

    return network_context_params;
}

// static
ProfileIODataQt *ProfileIODataQt::FromBrowserContext(content::BrowserContext *browser_context)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    return static_cast<ProfileQt *>(browser_context)->m_profileIOData.get();
}

// static
ProfileIODataQt *ProfileIODataQt::FromResourceContext(content::ResourceContext *resource_context)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    return static_cast<ResourceContextQt *>(resource_context)->m_io_data;
}

void ProfileIODataQt::removeBrowsingDataRemoverObserver()
{
    content::BrowsingDataRemover *remover =
            content::BrowserContext::GetBrowsingDataRemover(m_profileAdapter->profile());
    remover->RemoveObserver(&m_removerObserver);
}

BrowsingDataRemoverObserverQt::BrowsingDataRemoverObserverQt(ProfileIODataQt *profileIOData)
    : m_profileIOData(profileIOData)
{
}

void BrowsingDataRemoverObserverQt::OnBrowsingDataRemoverDone()
{
    Q_ASSERT(m_profileIOData->m_pendingStorageRequestGeneration);
    m_profileIOData->requestStorageGeneration();
    m_profileIOData->removeBrowsingDataRemoverObserver();
    m_profileIOData->m_pendingStorageRequestGeneration = false;
}

} // namespace QtWebEngineCore
