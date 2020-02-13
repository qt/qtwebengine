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

#include "profile_qt.h"

#include "profile_adapter.h"
#include "browsing_data_remover_delegate_qt.h"
#include "download_manager_delegate_qt.h"
#include "net/ssl_host_state_delegate_qt.h"
#include "permission_manager_qt.h"
#include "platform_notification_service_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "type_conversion.h"
#include "web_engine_library_info.h"
#include "web_engine_context.h"

#include "base/barrier_closure.h"
#include "base/time/time.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cors_origin_pattern_setter.h"
#include "content/public/browser/shared_cors_origin_access_list.h"
#include "content/public/browser/storage_partition.h"

#include "base/base_paths.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/user_prefs/user_prefs.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "chrome/common/pref_names.h"
#if QT_CONFIG(webengine_spellchecker)
#include "chrome/browser/spellchecker/spellcheck_service.h"
#include "components/spellcheck/browser/pref_names.h"
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "components/guest_view/browser/guest_view_manager.h"
#include "extensions/browser/pref_names.h"
#include "extensions/browser/process_manager.h"
#include "extensions/common/constants.h"

#include "extensions/extension_system_qt.h"
#endif

namespace QtWebEngineCore {

ProfileQt::ProfileQt(ProfileAdapter *profileAdapter)
    : m_sharedCorsOriginAccessList(content::SharedCorsOriginAccessList::Create())
    , m_profileIOData(new ProfileIODataQt(this))
    , m_profileAdapter(profileAdapter)
#if BUILDFLAG(ENABLE_EXTENSIONS)
    , m_extensionSystem(nullptr)
#endif // BUILDFLAG(ENABLE_EXTENSIONS)
{
    setupPrefService();

    // Mark the context as live. This prevents the use-after-free DCHECK in
    // AssertBrowserContextWasntDestroyed from being triggered when a new
    // ProfileQt object is allocated at the same address as a previously
    // destroyed one. Needs to be called after WebEngineContext initialization.
    BrowserContextDependencyManager::GetInstance()->MarkBrowserContextLive(this);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    m_extensionSystem = static_cast<extensions::ExtensionSystemQt*>(extensions::ExtensionSystem::Get(this));
    m_extensionSystem->InitForRegularProfile(true);
#endif
}

ProfileQt::~ProfileQt()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    m_prefServiceAdapter.commit();
    content::BrowserContext::NotifyWillBeDestroyed(this);
    BrowserContextDependencyManager::GetInstance()->DestroyBrowserContextServices(this);
    ShutdownStoragePartitions();
    m_profileIOData->shutdownOnUIThread();
    //Should be deleted by IO Thread
    m_profileIOData.release();
}

PrefService* ProfileQt::GetPrefs()
{
    return m_prefServiceAdapter.prefService();
}

const PrefService* ProfileQt::GetPrefs() const
{
    return m_prefServiceAdapter.prefService();
}

base::FilePath ProfileQt::GetPath()
{
    return toFilePath(m_profileAdapter->dataPath());
}

base::FilePath ProfileQt::GetCachePath() const
{
    return toFilePath(m_profileAdapter->cachePath());
}

bool ProfileQt::IsOffTheRecord()
{
    return m_profileAdapter->isOffTheRecord();
}

content::ResourceContext *ProfileQt::GetResourceContext()
{
    return m_profileIOData->resourceContext();
}

content::DownloadManagerDelegate *ProfileQt::GetDownloadManagerDelegate()
{
    return m_profileAdapter->downloadManagerDelegate();
}

content::BrowserPluginGuestManager *ProfileQt::GetGuestManager()
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    return guest_view::GuestViewManager::FromBrowserContext(this);
#else
    return nullptr;
#endif
}

storage::SpecialStoragePolicy *ProfileQt::GetSpecialStoragePolicy()
{
    // matches android_webview and chromecast
    return nullptr;
}

content::PushMessagingService *ProfileQt::GetPushMessagingService()
{
    return nullptr;
}

content::SSLHostStateDelegate* ProfileQt::GetSSLHostStateDelegate()
{
    if (!m_sslHostStateDelegate)
        m_sslHostStateDelegate.reset(new SSLHostStateDelegateQt());
    return m_sslHostStateDelegate.get();
}

std::unique_ptr<content::ZoomLevelDelegate> ProfileQt::CreateZoomLevelDelegate(const base::FilePath&)
{
    return nullptr;
}

content::BackgroundFetchDelegate* ProfileQt::GetBackgroundFetchDelegate()
{
    return nullptr;
}

content::BackgroundSyncController* ProfileQt::GetBackgroundSyncController()
{
    return nullptr;
}

content::BrowsingDataRemoverDelegate *ProfileQt::GetBrowsingDataRemoverDelegate()
{
    if (!m_removerDelegate)
        m_removerDelegate.reset(new BrowsingDataRemoverDelegateQt);
    return m_removerDelegate.get();
}

content::PermissionControllerDelegate *ProfileQt::GetPermissionControllerDelegate()
{
    if (!m_permissionManager)
        m_permissionManager.reset(new PermissionManagerQt());
    return m_permissionManager.get();
}

content::ClientHintsControllerDelegate *ProfileQt::GetClientHintsControllerDelegate()
{
    return nullptr;
}

content::StorageNotificationService *ProfileQt::GetStorageNotificationService()
{
    return nullptr;
}

void ProfileQt::SetCorsOriginAccessListForOrigin(const url::Origin &source_origin,
                                                 std::vector<network::mojom::CorsOriginPatternPtr> allow_patterns,
                                                 std::vector<network::mojom::CorsOriginPatternPtr> block_patterns,
                                                 base::OnceClosure closure)
{
    auto barrier_closure = base::BarrierClosure(2, std::move(closure));

    // Keep profile storage partitions' NetworkContexts synchronized.
    auto profile_setter = base::MakeRefCounted<content::CorsOriginPatternSetter>(
                source_origin,
                content::CorsOriginPatternSetter::ClonePatterns(allow_patterns),
                content::CorsOriginPatternSetter::ClonePatterns(block_patterns),
                barrier_closure);
    ForEachStoragePartition(this,
                            base::BindRepeating(&content::CorsOriginPatternSetter::SetLists,
                                                base::RetainedRef(profile_setter.get())));

    m_sharedCorsOriginAccessList->SetForOrigin(source_origin,
                                               std::move(allow_patterns),
                                               std::move(block_patterns),
                                               barrier_closure);
}

content::SharedCorsOriginAccessList *ProfileQt::GetSharedCorsOriginAccessList()
{
    return m_sharedCorsOriginAccessList.get();
}

#if QT_CONFIG(webengine_spellchecker)
void ProfileQt::FailedToLoadDictionary(const std::string &language)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    LOG(WARNING) << "Could not load dictionary for:" << language;
    LOG(INFO) << "Make sure that correct bdic file is in:" << WebEngineLibraryInfo::getPath(base::DIR_APP_DICTIONARIES);
}
#endif // QT_CONFIG(webengine_spellchecker)

#if BUILDFLAG(ENABLE_EXTENSIONS)
extensions::ExtensionSystemQt* ProfileQt::GetExtensionSystem()
{
    return m_extensionSystem;
}
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

std::string ProfileQt::GetMediaDeviceIDSalt()
{
    return m_prefServiceAdapter.mediaDeviceIdSalt();
}

void ProfileQt::setupPrefService()
{
    // Remove previous handler before we set a new one or we will assert
    // TODO: Remove in Qt6
    if (m_prefServiceAdapter.prefService() != nullptr) {
        user_prefs::UserPrefs::Remove(this);
        m_prefServiceAdapter.commit();
    }
    m_prefServiceAdapter.setup(*m_profileAdapter);
    user_prefs::UserPrefs::Set(this, m_prefServiceAdapter.prefService());
}

PrefServiceAdapter &ProfileQt::prefServiceAdapter()
{
    return m_prefServiceAdapter;
}

const PrefServiceAdapter &ProfileQt::prefServiceAdapter() const
{
    return m_prefServiceAdapter;
}


content::PlatformNotificationService *ProfileQt::platformNotificationService()
{
    if (!m_platformNotificationService)
        m_platformNotificationService = std::make_unique<PlatformNotificationServiceQt>(this);
    return m_platformNotificationService.get();
}

} // namespace QtWebEngineCore
