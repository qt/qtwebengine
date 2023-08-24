// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "profile_qt.h"

#include "profile_adapter.h"
#include "browsing_data_remover_delegate_qt.h"
#include "client_hints.h"
#include "download_manager_delegate_qt.h"
#include "file_system_access/file_system_access_permission_context_factory_qt.h"
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
#include "base/files/file_util.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/user_prefs/user_prefs.h"
#include "components/profile_metrics/browser_profile_type.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "chrome/browser/push_messaging/push_messaging_app_identifier.h"
#include "chrome/browser/push_messaging/push_messaging_service_factory.h"
#include "chrome/browser/push_messaging/push_messaging_service_impl.h"
#include "chrome/common/pref_names.h"
#if QT_CONFIG(webengine_spellchecker)
#include "chrome/browser/spellchecker/spellcheck_service.h"
#include "components/spellcheck/browser/pref_names.h"
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "base/command_line.h"
#include "components/guest_view/browser/guest_view_manager.h"
#include "extensions/browser/extension_pref_value_map_factory.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/pref_names.h"
#include "extensions/browser/process_manager.h"
#include "extensions/common/constants.h"

#include "extensions/extension_system_qt.h"
#endif

#if defined(Q_OS_WIN)
#include "components/os_crypt/os_crypt.h"
#endif

namespace QtWebEngineCore {

ProfileQt::ProfileQt(ProfileAdapter *profileAdapter)
    : m_profileIOData(new ProfileIODataQt(this))
    , m_profileAdapter(profileAdapter)
#if BUILDFLAG(ENABLE_EXTENSIONS)
    , m_extensionSystem(nullptr)
#endif // BUILDFLAG(ENABLE_EXTENSIONS)
{
    profile_metrics::SetBrowserProfileType(this, IsOffTheRecord()
        ? profile_metrics::BrowserProfileType::kIncognito
        : profile_metrics::BrowserProfileType::kRegular);

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
    BrowserContextDependencyManager::GetInstance()->DestroyBrowserContextServices(this);
    // Remembering push subscriptions and not persisting notification permissions would
    // confuse most of web applications.
    PushMessagingAppIdentifier::DeleteAllFromPrefs(this);
    ShutdownStoragePartitions();
    m_profileIOData->shutdownOnUIThread();
    //Should be deleted by IO Thread
    m_profileIOData.release();
}

void ProfileQt::DoFinalInit()
{
    PushMessagingServiceImpl::InitializeForProfile(this);
}

PrefService* ProfileQt::GetPrefs()
{
    return m_prefServiceAdapter.prefService();
}

const PrefService* ProfileQt::GetPrefs() const
{
    return m_prefServiceAdapter.prefService();
}

bool ProfileQt::IsNewProfile() const
{
    return GetPrefs()->GetInitializationStatus() == PrefService::INITIALIZATION_STATUS_CREATED_NEW_PREF_STORE;
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
    if (m_profileAdapter->pushServiceEnabled())
        return PushMessagingServiceFactory::GetForProfile(this);
    else
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
    return ClientHintsFactory::GetForBrowserContext(this);
}

content::StorageNotificationService *ProfileQt::GetStorageNotificationService()
{
    return nullptr;
}

content::ReduceAcceptLanguageControllerDelegate *ProfileQt::GetReduceAcceptLanguageControllerDelegate()
{
    return nullptr;
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

content::FileSystemAccessPermissionContext *ProfileQt::GetFileSystemAccessPermissionContext()
{
    return FileSystemAccessPermissionContextFactoryQt::GetForProfile(this);
}

void ProfileQt::setupPrefService()
{
    const bool recreation = m_prefServiceAdapter.prefService() != nullptr;
    profile_metrics::SetBrowserProfileType(this,
                                           IsOffTheRecord()
                                               ? profile_metrics::BrowserProfileType::kIncognito
                                               : profile_metrics::BrowserProfileType::kRegular);

    // Remove previous handler before we set a new one or we will assert
    // TODO: Remove in Qt6
    if (recreation) {
        user_prefs::UserPrefs::Remove(this);
        m_prefServiceAdapter.commit();
    }
    m_prefServiceAdapter.setup(*m_profileAdapter);
    user_prefs::UserPrefs::Set(this, m_prefServiceAdapter.prefService());

#if BUILDFLAG(ENABLE_EXTENSIONS)
    if (recreation) {
        // Recreate ExtensionPrefs to update its pointer to the new PrefService
        extensions::ExtensionsBrowserClient *client = extensions::ExtensionsBrowserClient::Get();
        std::vector<extensions::EarlyExtensionPrefsObserver *> prefsObservers;
        client->GetEarlyExtensionPrefsObservers(this, &prefsObservers);
        extensions::ExtensionPrefs *extensionPrefs = extensions::ExtensionPrefs::Create(
            this, client->GetPrefServiceForContext(this),
            this->GetPath().AppendASCII(extensions::kInstallDirectoryName),
            ExtensionPrefValueMapFactory::GetForBrowserContext(this),
            client->AreExtensionsDisabled(*base::CommandLine::ForCurrentProcess(), this),
            prefsObservers);
        extensions::ExtensionPrefsFactory::GetInstance()->SetInstanceForTesting(this, base::WrapUnique(extensionPrefs));
    }
#endif
}

PrefServiceAdapter &ProfileQt::prefServiceAdapter()
{
    return m_prefServiceAdapter;
}

const PrefServiceAdapter &ProfileQt::prefServiceAdapter() const
{
    return m_prefServiceAdapter;
}

content::PlatformNotificationService *ProfileQt::GetPlatformNotificationService()
{
    if (!m_platformNotificationService)
        m_platformNotificationService = std::make_unique<PlatformNotificationServiceQt>(this);
    return m_platformNotificationService.get();
}

} // namespace QtWebEngineCore
