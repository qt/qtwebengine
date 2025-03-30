// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PROFILE_QT_H
#define PROFILE_QT_H

#include "chrome/browser/profiles/profile.h"
#include "components/embedder_support/user_agent_utils.h"
#include "extensions/buildflags/buildflags.h"
#include "pref_service_adapter.h"

class PrefService;

namespace extensions {
class ExtensionSystemQt;
}

namespace QtWebEngineCore {

class BrowsingDataRemoverDelegateQt;
class PermissionManagerQt;
class ProfileAdapter;
class ProfileIODataQt;
class SSLHostStateDelegateQt;

class ProfileQt : public Profile
{
public:
    explicit ProfileQt(ProfileAdapter *profileAdapter);

    virtual ~ProfileQt();

    base::FilePath GetCachePath() const;

    // BrowserContext implementation:
    base::FilePath GetPath() override;
    bool IsOffTheRecord() override;

    content::DownloadManagerDelegate *GetDownloadManagerDelegate() override;
    content::BrowserPluginGuestManager *GetGuestManager() override;
    storage::SpecialStoragePolicy *GetSpecialStoragePolicy() override;
    content::PushMessagingService *GetPushMessagingService() override;
    content::SSLHostStateDelegate *GetSSLHostStateDelegate() override;
    std::unique_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
            const base::FilePath &partition_path) override;
    content::PermissionControllerDelegate * GetPermissionControllerDelegate() override;
    content::BackgroundFetchDelegate *GetBackgroundFetchDelegate() override;
    content::BackgroundSyncController *GetBackgroundSyncController() override;
    content::BrowsingDataRemoverDelegate *GetBrowsingDataRemoverDelegate() override;
    content::ClientHintsControllerDelegate *GetClientHintsControllerDelegate() override;
    content::StorageNotificationService *GetStorageNotificationService() override;
    content::PlatformNotificationService *GetPlatformNotificationService() override;
    content::FileSystemAccessPermissionContext *GetFileSystemAccessPermissionContext() override;
    content::ReduceAcceptLanguageControllerDelegate *GetReduceAcceptLanguageControllerDelegate() override;

    // Profile implementation:
    PrefService *GetPrefs() override;
    const PrefService *GetPrefs() const override;
    bool IsNewProfile() const override;

    void DoFinalInit();
    ProfileAdapter *profileAdapter() { return m_profileAdapter; }
    std::string GetMediaDeviceIDSalt();

#if QT_CONFIG(webengine_spellchecker)
    void FailedToLoadDictionary(const std::string &language) override;
#endif
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::ExtensionSystemQt* GetExtensionSystem();
#endif // defined(ENABLE_EXTENSIONS)

    // Build/Re-build the preference service. Call when updating the storage
    // data path.
    void setupPrefService();
    void setupStoragePath();
    void setupPermissionsManager();

    PrefServiceAdapter &prefServiceAdapter();
    const PrefServiceAdapter &prefServiceAdapter() const;

    const blink::UserAgentMetadata &userAgentMetadata();

private:
    std::unique_ptr<BrowsingDataRemoverDelegateQt> m_removerDelegate;
    std::unique_ptr<PermissionManagerQt> m_permissionManager;
    std::unique_ptr<SSLHostStateDelegateQt> m_sslHostStateDelegate;
    std::unique_ptr<ProfileIODataQt> m_profileIOData;
    std::unique_ptr<content::PlatformNotificationService> m_platformNotificationService;
    ProfileAdapter *m_profileAdapter;
    PrefServiceAdapter m_prefServiceAdapter;
    blink::UserAgentMetadata m_userAgentMetadata;

#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::ExtensionSystemQt *m_extensionSystem;
#endif //ENABLE_EXTENSIONS

    friend class ProfileAdapter;
    friend class ProfileIODataQt;
};

} // namespace QtWebEngineCore

#endif // PROFILE_QT_H
