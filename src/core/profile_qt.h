/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef PROFILE_QT_H
#define PROFILE_QT_H

#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/resource_context.h"
#include "extensions/buildflags/buildflags.h"
#include "pref_service_adapter.h"
#include "profile_io_data_qt.h"
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE
class InMemoryPrefStore;
class PrefService;

namespace extensions {
class ExtensionSystemQt;
}

namespace QtWebEngineCore {

class BrowsingDataRemoverDelegateQt;
class ProfileAdapter;
class PermissionManagerQt;
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

    content::ResourceContext *GetResourceContext() override;
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
    void SetCorsOriginAccessListForOrigin(const url::Origin &source_origin,
                                          std::vector<network::mojom::CorsOriginPatternPtr> allow_patterns,
                                          std::vector<network::mojom::CorsOriginPatternPtr> block_patterns,
                                          base::OnceClosure closure) override;
    content::SharedCorsOriginAccessList *GetSharedCorsOriginAccessList() override;
    std::string GetMediaDeviceIDSalt() override;

    // Profile implementation:
    PrefService *GetPrefs() override;
    const PrefService *GetPrefs() const override;

    void Initialize();
    ProfileAdapter *profileAdapter() { return m_profileAdapter; }

    content::PlatformNotificationService *platformNotificationService();

#if QT_CONFIG(webengine_spellchecker)
    void FailedToLoadDictionary(const std::string &language) override;
#endif
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::ExtensionSystemQt* GetExtensionSystem();
#endif // defined(ENABLE_EXTENSIONS)

    // Build/Re-build the preference service. Call when updating the storage
    // data path.
    void setupPrefService();

    PrefServiceAdapter &prefServiceAdapter();

    const PrefServiceAdapter &prefServiceAdapter() const;

private:
    friend class ContentBrowserClientQt;
    friend class ProfileIODataQt;
    friend class WebContentsAdapter;
    std::unique_ptr<BrowsingDataRemoverDelegateQt> m_removerDelegate;
    std::unique_ptr<PermissionManagerQt> m_permissionManager;
    std::unique_ptr<SSLHostStateDelegateQt> m_sslHostStateDelegate;
    scoped_refptr<content::SharedCorsOriginAccessList> m_sharedCorsOriginAccessList;
    std::unique_ptr<ProfileIODataQt> m_profileIOData;
    std::unique_ptr<content::PlatformNotificationService> m_platformNotificationService;
    ProfileAdapter *m_profileAdapter;
    PrefServiceAdapter m_prefServiceAdapter;

    friend class ProfileAdapter;
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::ExtensionSystemQt *m_extensionSystem;
#endif //ENABLE_EXTENSIONS
    friend class BrowserContextAdapter;

    DISALLOW_COPY_AND_ASSIGN(ProfileQt);
};

} // namespace QtWebEngineCore

#endif // PROFILE_QT_H
