// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "profile_io_data_qt.h"

#include "content/browser/storage_partition_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browsing_data_remover.h"
#include "content/public/browser/network_service_instance.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/shared_cors_origin_access_list.h"
#include "content/public/common/content_features.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "services/cert_verifier/cert_verifier_creation.h"
#include "services/cert_verifier/public/mojom/cert_verifier_service_factory.mojom.h"
#include "services/network/public/cpp/cors/origin_access_list.h"
#include "services/network/public/mojom/cert_verifier_service.mojom.h"

#include "net/client_cert_qt.h"
#include "net/client_cert_store_data.h"
#include "net/cookie_monster_delegate_qt.h"
#include "net/system_network_context_manager.h"
#include "profile_qt.h"
#include "type_conversion.h"

#include <QDebug>
#include <mutex>

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

    if (m_clearHttpCacheInProgress) {
        m_clearHttpCacheInProgress = false;
        content::BrowsingDataRemover *remover =
                m_profileAdapter->profile()->GetBrowsingDataRemover();
        remover->RemoveObserver(&m_removerObserver);
    }

    bool posted = content::BrowserThread::DeleteSoon(content::BrowserThread::IO, FROM_HERE, this);
    if (!posted) {
        qWarning() << "Could not delete ProfileIODataQt on io thread !";
        delete this;
    }
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

base::WeakPtr<ProfileIODataQt> ProfileIODataQt::getWeakPtrOnIOThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    return m_weakPtrFactory.GetWeakPtr();
}

void ProfileIODataQt::initializeOnUIThread()
{
    m_profileAdapter = m_profile->profileAdapter();
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    m_resourceContext.reset(new content::ResourceContext());
    m_cookieDelegate = new CookieMonsterDelegateQt();
    m_cookieDelegate->setClient(m_profile->profileAdapter()->cookieStore());
    m_proxyConfigMonitor.reset(new ProxyConfigMonitor(m_profile->GetPrefs()));
}

void ProfileIODataQt::clearHttpCache()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    if (!m_clearHttpCacheInProgress) {
        m_clearHttpCacheInProgress = true;
        content::BrowsingDataRemover *remover =
                m_profileAdapter->profile()->GetBrowsingDataRemover();
        remover->AddObserver(&m_removerObserver);
        remover->RemoveAndReply(base::Time(), base::Time::Max(),
            content::BrowsingDataRemover::DATA_TYPE_CACHE,
            content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB |
                        content::BrowsingDataRemover::ORIGIN_TYPE_PROTECTED_WEB,
            &m_removerObserver);
    }
}

void ProfileIODataQt::removeBrowsingDataRemoverObserver()
{
    content::BrowsingDataRemover *remover =
            m_profileAdapter->profile()->GetBrowsingDataRemover();
    remover->RemoveObserver(&m_removerObserver);
}

BrowsingDataRemoverObserverQt::BrowsingDataRemoverObserverQt(ProfileIODataQt *profileIOData)
    : m_profileIOData(profileIOData)
{
}

void BrowsingDataRemoverObserverQt::OnBrowsingDataRemoverDone(uint64_t)
{
    Q_ASSERT(m_profileIOData->m_clearHttpCacheInProgress);
    m_profileIOData->removeBrowsingDataRemoverObserver();
    m_profileIOData->m_clearHttpCacheInProgress = false;
    m_profileIOData->resetNetworkContext();
}

void ProfileIODataQt::setFullConfiguration()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    m_persistentCookiesPolicy = m_profileAdapter->persistentCookiesPolicy();
    m_httpAcceptLanguage = m_profileAdapter->httpAcceptLanguage();
    m_httpUserAgent = m_profileAdapter->httpUserAgent();
    m_httpCacheType = m_profileAdapter->httpCacheType();
    m_httpCachePath = m_profileAdapter->httpCachePath();
    m_httpCacheMaxSize = m_profileAdapter->httpCacheMaxSize();
    m_dataPath = m_profileAdapter->dataPath();
    m_storageName = m_profileAdapter->storageName();
    m_inMemoryOnly = m_profileAdapter->isOffTheRecord() || m_storageName.isEmpty();
}

void ProfileIODataQt::resetNetworkContext()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    setFullConfiguration();
    m_profile->ForEachLoadedStoragePartition(
            base::BindRepeating([](content::StoragePartition *storage) {
                auto storage_impl = static_cast<content::StoragePartitionImpl *>(storage);
                storage_impl->ResetURLLoaderFactories();
                storage_impl->ResetNetworkContext();
            }));
}

bool ProfileIODataQt::canGetCookies(const QUrl &firstPartyUrl, const QUrl &url) const
{
    return m_cookieDelegate->canGetCookies(firstPartyUrl, url);
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
    return std::unique_ptr<net::ClientCertStore>(new ClientCertStoreQt(m_clientCertificateStoreData));
#else
    return std::unique_ptr<net::ClientCertStore>(new ClientCertStoreQt(nullptr));
#endif
}

void ProfileIODataQt::ConfigureNetworkContextParams(bool in_memory,
                                                    const base::FilePath &relative_partition_path,
                                                    network::mojom::NetworkContextParams *network_context_params,
                                                    cert_verifier::mojom::CertVerifierCreationParams *cert_verifier_creation_params)
{
    setFullConfiguration();

    SystemNetworkContextManager::GetInstance()->ConfigureDefaultNetworkContextParams(network_context_params, cert_verifier_creation_params);

    network_context_params->user_agent = m_httpUserAgent.toStdString();
    network_context_params->accept_language = m_httpAcceptLanguage.toStdString();

    network_context_params->enable_referrers = true;
    // Encrypted cookies requires os_crypt, which currently has issues for us on Linux.
    network_context_params->enable_encrypted_cookies = false;

    network_context_params->http_cache_enabled = m_httpCacheType != ProfileAdapter::NoCache;
    network_context_params->http_cache_max_size = m_httpCacheMaxSize;
    if (m_httpCacheType == ProfileAdapter::DiskHttpCache && !m_httpCachePath.isEmpty() && !m_inMemoryOnly && !in_memory)
        network_context_params->http_cache_directory = toFilePath(m_httpCachePath);

    network_context_params->persist_session_cookies = false;
    if (!m_inMemoryOnly && !in_memory) {
        network_context_params->file_paths =
            network::mojom::NetworkContextFilePaths::New();
        network_context_params->file_paths->data_directory = toFilePath(m_dataPath);
        network_context_params->file_paths->http_server_properties_file_name = base::FilePath::FromASCII("Network Persistent State");
        network_context_params->file_paths->transport_security_persister_file_name = base::FilePath::FromASCII("TransportSecurity");
        network_context_params->file_paths->trust_token_database_name = base::FilePath::FromASCII("Trust Tokens");
        if (m_persistentCookiesPolicy != ProfileAdapter::NoPersistentCookies) {
            network_context_params->file_paths->cookie_database_name = base::FilePath::FromASCII("Cookies");
            network_context_params->restore_old_session_cookies = m_persistentCookiesPolicy == ProfileAdapter::ForcePersistentCookies;
            network_context_params->persist_session_cookies = m_persistentCookiesPolicy != ProfileAdapter::NoPersistentCookies;
        }
    }

    network_context_params->enforce_chrome_ct_policy = false;

    // Should be initialized with existing per-profile CORS access lists.
    network_context_params->cors_origin_access_list =
        m_profile->GetSharedCorsOriginAccessList()->GetOriginAccessList().CreateCorsOriginAccessPatternsList();

    m_proxyConfigMonitor->AddToNetworkContextParams(network_context_params);
}

// static
ProfileIODataQt *ProfileIODataQt::FromBrowserContext(content::BrowserContext *browser_context)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    return static_cast<ProfileQt *>(browser_context)->m_profileIOData.get();
}

} // namespace QtWebEngineCore
