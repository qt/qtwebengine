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
#include "profile_io_data_qt.h"
#include "platform_notification_service_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "type_conversion.h"
#include "web_engine_library_info.h"

#include "base/base_paths.h"
#include "base/path_service.h"
#include "base/files/file_util.h"
#include "base/task/thread_pool.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "components/profile_metrics/browser_profile_type.h"
#include "content/public/browser/browser_thread.h"
#include "chrome/browser/push_messaging/push_messaging_app_identifier.h"
#include "chrome/browser/push_messaging/push_messaging_service_factory.h"
#include "chrome/browser/push_messaging/push_messaging_service_impl.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "base/command_line.h"
#include "components/guest_view/browser/guest_view_manager.h"
#include "extensions/browser/extension_pref_value_map_factory.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extensions_browser_client.h"

#include "extensions/extension_system_qt.h"
#endif

namespace QtWebEngineCore {

enum {
    PATH_QT_START = 1000, // Same as PATH_START in chrome_paths.h; no chance of collision
    PATH_QT_END = 1999
};

ProfileQt::ProfileQt(ProfileAdapter *profileAdapter)
    : m_profileIOData(new ProfileIODataQt(this))
    , m_profileAdapter(profileAdapter)
    , m_userAgentMetadata(embedder_support::GetUserAgentMetadata())
#if BUILDFLAG(ENABLE_EXTENSIONS)
    , m_extensionSystem(nullptr)
#endif // BUILDFLAG(ENABLE_EXTENSIONS)
{
    profile_metrics::SetBrowserProfileType(this, IsOffTheRecord()
        ? profile_metrics::BrowserProfileType::kIncognito
        : profile_metrics::BrowserProfileType::kRegular);

    setupPrefService();
    setupStoragePath();

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
        setupPermissionsManager();
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
        auto extensionPrefs = extensions::ExtensionPrefs::Create(
            this, client->GetPrefServiceForContext(this),
            this->GetPath().AppendASCII(extensions::kInstallDirectoryName),
            ExtensionPrefValueMapFactory::GetForBrowserContext(this),
            client->AreExtensionsDisabled(*base::CommandLine::ForCurrentProcess(), this),
            prefsObservers);
        extensions::ExtensionPrefsFactory::GetInstance()->SetInstanceForTesting(this, std::move(extensionPrefs));
    }
#endif
}

void ProfileQt::setupStoragePath()
{
#if defined(Q_OS_WIN)
    if (IsOffTheRecord())
        return;

    // Mark the storage path as a "safe" path, allowing the path service on Windows to
    // block file execution and prevent assertions when saving blobs to disk.
    // We keep a static list of all profile paths

    base::FilePath thisStoragePath = GetPath();

    static std::vector<base::FilePath> storagePaths;
    auto it = std::find(storagePaths.begin(), storagePaths.end(), thisStoragePath);
    if (it == storagePaths.end()) {
        if (storagePaths.size() >= (PATH_QT_END - PATH_QT_START)) {
            qWarning() << "Number of profile paths exceeded " << PATH_QT_END - PATH_QT_START << ", storage may break";
            return;
        }

        storagePaths.push_back(thisStoragePath);
        it = storagePaths.end() - 1;
    }

    int pathID = PATH_QT_START + (it - storagePaths.begin());
    base::ThreadPool::PostTaskAndReplyWithResult(FROM_HERE, { base::MayBlock() },
        base::BindOnce(base::PathService::Override, PATH_QT_START + (it - storagePaths.begin()), thisStoragePath),
        base::BindOnce([](int pathID_, bool succeeded) {
            if (succeeded) base::SetExtraNoExecuteAllowedPath(pathID_);
        }, pathID));
#endif // defined(Q_OS_WIN)
}

void ProfileQt::setupPermissionsManager()
{
    m_permissionManager.reset(new PermissionManagerQt(profileAdapter()));
}

PrefServiceAdapter &ProfileQt::prefServiceAdapter()
{
    return m_prefServiceAdapter;
}

const PrefServiceAdapter &ProfileQt::prefServiceAdapter() const
{
    return m_prefServiceAdapter;
}

const blink::UserAgentMetadata &ProfileQt::userAgentMetadata()
{
    return m_userAgentMetadata;
}

content::PlatformNotificationService *ProfileQt::GetPlatformNotificationService()
{
    if (!m_platformNotificationService)
        m_platformNotificationService = std::make_unique<PlatformNotificationServiceQt>(this);
    return m_platformNotificationService.get();
}

} // namespace QtWebEngineCore
