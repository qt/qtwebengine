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
#include "net/ssl/ssl_config_service_defaults.h"
#include "services/network/public/cpp/cors/origin_access_list.h"

#include "net/client_cert_override.h"
#include "net/client_cert_store_data.h"
#include "net/cookie_monster_delegate_qt.h"
#include "net/system_network_context_manager.h"
#include "profile_qt.h"
#include "resource_context_qt.h"
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
    m_resourceContext.reset(new ResourceContextQt(this));
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
                content::BrowserContext::GetBrowsingDataRemover(m_profileAdapter->profile());
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
            content::BrowserContext::GetBrowsingDataRemover(m_profileAdapter->profile());
    remover->RemoveObserver(&m_removerObserver);
}

BrowsingDataRemoverObserverQt::BrowsingDataRemoverObserverQt(ProfileIODataQt *profileIOData)
    : m_profileIOData(profileIOData)
{
}

void BrowsingDataRemoverObserverQt::OnBrowsingDataRemoverDone()
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
    m_useForGlobalCertificateVerification = m_profileAdapter->isUsedForGlobalCertificateVerification();
    m_dataPath = m_profileAdapter->dataPath();
    m_storageName = m_profileAdapter->storageName();
    m_inMemoryOnly = m_profileAdapter->isOffTheRecord() || m_storageName.isEmpty();
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
    return std::unique_ptr<net::ClientCertStore>(new ClientCertOverrideStore(m_clientCertificateStoreData));
#else
    return std::unique_ptr<net::ClientCertStore>(new ClientCertOverrideStore(nullptr));
#endif
}

network::mojom::NetworkContextParamsPtr ProfileIODataQt::CreateNetworkContextParams()
{
    setFullConfiguration();

    network::mojom::NetworkContextParamsPtr network_context_params =
             SystemNetworkContextManager::GetInstance()->CreateDefaultNetworkContextParams();

    network_context_params->context_name = m_storageName.toStdString();
    network_context_params->user_agent = m_httpUserAgent.toStdString();
    network_context_params->accept_language = m_httpAcceptLanguage.toStdString();

    network_context_params->enable_referrers = true;
    // Encrypted cookies requires os_crypt, which currently has issues for us on Linux.
    network_context_params->enable_encrypted_cookies = false;

    network_context_params->http_cache_enabled = m_httpCacheType != ProfileAdapter::NoCache;
    network_context_params->http_cache_max_size = m_httpCacheMaxSize;
    if (m_httpCacheType == ProfileAdapter::DiskHttpCache && !m_httpCachePath.isEmpty())
        network_context_params->http_cache_path = toFilePath(m_httpCachePath);

    if (m_persistentCookiesPolicy != ProfileAdapter::NoPersistentCookies && !m_inMemoryOnly) {
        base::FilePath cookie_path = toFilePath(m_dataPath);
        cookie_path = cookie_path.AppendASCII("Cookies");
        network_context_params->cookie_path = cookie_path;

        network_context_params->restore_old_session_cookies = m_persistentCookiesPolicy == ProfileAdapter::ForcePersistentCookies;
        network_context_params->persist_session_cookies = m_persistentCookiesPolicy != ProfileAdapter::NoPersistentCookies;
    }
    if (!m_inMemoryOnly) {
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

    // Should be initialized with existing per-profile CORS access lists.
    network_context_params->cors_origin_access_list =
        m_profile->GetSharedCorsOriginAccessList()->GetOriginAccessList().CreateCorsOriginAccessPatternsList();

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

} // namespace QtWebEngineCore
