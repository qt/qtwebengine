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
#include "command_line_pref_store_qt.h"
#include "download_manager_delegate_qt.h"
#include "net/ssl_host_state_delegate_qt.h"
#include "net/url_request_context_getter_qt.h"
#include "permission_manager_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "type_conversion.h"
#include "web_engine_library_info.h"
#include "web_engine_context.h"

#include "base/time/time.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/shared_cors_origin_access_list.h"
#include "content/public/browser/storage_partition.h"

#include "base/base_paths.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/in_memory_pref_store.h"
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
#include "extensions/browser/extension_protocols.h"
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
    PrefServiceFactory factory;
    factory.set_user_prefs(new InMemoryPrefStore);
    factory.set_command_line_prefs(base::MakeRefCounted<CommandLinePrefStoreQt>(
            WebEngineContext::commandLine()));
    PrefRegistrySimple *registry = new PrefRegistrySimple();
    PrefProxyConfigTrackerImpl::RegisterPrefs(registry);
#if QT_CONFIG(webengine_spellchecker)
    // Initial spellcheck settings
    registry->RegisterStringPref(prefs::kAcceptLanguages, std::string());
    registry->RegisterListPref(spellcheck::prefs::kSpellCheckDictionaries, std::make_unique<base::ListValue>());
    registry->RegisterListPref(spellcheck::prefs::kSpellCheckForcedDictionaries, std::make_unique<base::ListValue>());
    registry->RegisterStringPref(spellcheck::prefs::kSpellCheckDictionary, std::string());
    registry->RegisterBooleanPref(spellcheck::prefs::kSpellCheckEnable, false);
    registry->RegisterBooleanPref(spellcheck::prefs::kSpellCheckUseSpellingService, false);
#endif // QT_CONFIG(webengine_spellchecker)
    registry->RegisterBooleanPref(prefs::kShowInternalAccessibilityTree, false);
    registry->RegisterIntegerPref(prefs::kNotificationNextPersistentId, 10000);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    registry->RegisterDictionaryPref(extensions::pref_names::kExtensions);
    registry->RegisterListPref(extensions::pref_names::kInstallAllowList);
    registry->RegisterListPref(extensions::pref_names::kInstallDenyList);
    registry->RegisterDictionaryPref(extensions::pref_names::kInstallForceList);
    registry->RegisterDictionaryPref(extensions::pref_names::kInstallLoginScreenAppList);
    registry->RegisterListPref(extensions::pref_names::kAllowedTypes);
    registry->RegisterBooleanPref(extensions::pref_names::kStorageGarbageCollect, false);
    registry->RegisterListPref(extensions::pref_names::kAllowedInstallSites);
    registry->RegisterStringPref(extensions::pref_names::kLastChromeVersion, std::string());
    registry->RegisterListPref(extensions::pref_names::kNativeMessagingBlacklist);
    registry->RegisterListPref(extensions::pref_names::kNativeMessagingWhitelist);
    registry->RegisterBooleanPref(extensions::pref_names::kNativeMessagingUserLevelHosts, true);
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

    m_prefService = factory.Create(registry);
    user_prefs::UserPrefs::Set(this, m_prefService.get());

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
    content::BrowserContext::NotifyWillBeDestroyed(this);
    BrowserContextDependencyManager::GetInstance()->DestroyBrowserContextServices(this);
    ShutdownStoragePartitions();
    m_profileIOData->shutdownOnUIThread();
    //Should be deleted by IO Thread
    m_profileIOData.release();
}

PrefService* ProfileQt::GetPrefs()
{
    return m_prefService.get();
}

const PrefService* ProfileQt::GetPrefs() const
{
    return m_prefService.get();
}

base::FilePath ProfileQt::GetPath() const
{
    return toFilePath(m_profileAdapter->dataPath());
}

base::FilePath ProfileQt::GetCachePath() const
{
    return toFilePath(m_profileAdapter->cachePath());
}

bool ProfileQt::IsOffTheRecord() const
{
    return m_profileAdapter->isOffTheRecord();
}

net::URLRequestContextGetter *ProfileQt::GetRequestContext()
{
    return m_urlRequestContextGetter.get();
}

net::URLRequestContextGetter *ProfileQt::CreateMediaRequestContext()
{
    return m_urlRequestContextGetter.get();
}

net::URLRequestContextGetter *ProfileQt::CreateMediaRequestContextForStoragePartition(const base::FilePath&, bool)
{
    Q_UNIMPLEMENTED();
    return nullptr;
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
    QT_NOT_YET_IMPLEMENTED
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

net::URLRequestContextGetter *ProfileQt::CreateRequestContext(
        content::ProtocolHandlerMap *protocol_handlers,
        content::URLRequestInterceptorScopedVector request_interceptors)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    DCHECK(!m_urlRequestContextGetter.get());
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::InfoMap* extension_info_map = GetExtensionSystem()->info_map();
    (*protocol_handlers)[extensions::kExtensionScheme] =
            extensions::CreateExtensionProtocolHandler(IsOffTheRecord(),extension_info_map);
#endif

    m_profileIOData->setRequestContextData(protocol_handlers, std::move(request_interceptors));
    m_profileIOData->updateStorageSettings();
    m_urlRequestContextGetter = new URLRequestContextGetterQt(m_profileIOData.get());
    return m_urlRequestContextGetter.get();
}

net::URLRequestContextGetter *ProfileQt::CreateRequestContextForStoragePartition(
        const base::FilePath& partition_path, bool in_memory,
        content::ProtocolHandlerMap* protocol_handlers,
        content::URLRequestInterceptorScopedVector request_interceptors)
{
    Q_UNIMPLEMENTED();
    return nullptr;
}

content::ClientHintsControllerDelegate *ProfileQt::GetClientHintsControllerDelegate()
{
    return nullptr;
}

void ProfileQt::SetCorsOriginAccessListForOrigin(const url::Origin &source_origin,
                                                 std::vector<network::mojom::CorsOriginPatternPtr> allow_patterns,
                                                 std::vector<network::mojom::CorsOriginPatternPtr> block_patterns,
                                                 base::OnceClosure closure)
{
    m_sharedCorsOriginAccessList->SetForOrigin(source_origin,
                                               std::move(allow_patterns),
                                               std::move(block_patterns),
                                               std::move(closure));
}

const content::SharedCorsOriginAccessList *ProfileQt::GetSharedCorsOriginAccessList() const
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

void ProfileQt::setSpellCheckLanguages(const QStringList &languages)
{
    StringListPrefMember dictionaries_pref;
    dictionaries_pref.Init(spellcheck::prefs::kSpellCheckDictionaries, m_prefService.get());
    std::vector<std::string> dictionaries;
    dictionaries.reserve(languages.size());
    for (const auto &language : languages)
        dictionaries.push_back(language.toStdString());
    dictionaries_pref.SetValue(dictionaries);
}

QStringList ProfileQt::spellCheckLanguages() const
{
    QStringList spellcheck_dictionaries;
    for (const auto &value : *m_prefService->GetList(spellcheck::prefs::kSpellCheckDictionaries)) {
        std::string dictionary;
        if (value.GetAsString(&dictionary))
            spellcheck_dictionaries.append(QString::fromStdString(dictionary));
    }

    return spellcheck_dictionaries;
}

void ProfileQt::setSpellCheckEnabled(bool enabled)
{
    m_prefService->SetBoolean(spellcheck::prefs::kSpellCheckEnable, enabled);
}

bool ProfileQt::isSpellCheckEnabled() const
{
    return m_prefService->GetBoolean(spellcheck::prefs::kSpellCheckEnable);
}
#endif // QT_CONFIG(webengine_spellchecker)

#if BUILDFLAG(ENABLE_EXTENSIONS)
extensions::ExtensionSystemQt* ProfileQt::GetExtensionSystem()
{
    return m_extensionSystem;
}
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

} // namespace QtWebEngineCore
