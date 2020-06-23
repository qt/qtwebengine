/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "pref_service_adapter.h"

#include "profile_adapter.h"
#include "type_conversion.h"
#include "web_engine_context.h"

#include "chrome/browser/prefs/chrome_command_line_pref_store.h"
#include "content/public/browser/browser_thread.h"
#include "components/language/core/browser/pref_names.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/user_prefs/user_prefs.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "chrome/common/pref_names.h"
#include "extensions/buildflags/buildflags.h"
#include "content/public/browser/browser_context.h"

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
#endif

namespace {
static const char kPrefMediaDeviceIDSalt[] = "qtwebengine.media_device_salt_id";
}

namespace QtWebEngineCore {

void PrefServiceAdapter::setup(const ProfileAdapter &profileAdapter)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    PrefServiceFactory factory;
    factory.set_command_line_prefs(base::MakeRefCounted<ChromeCommandLinePrefStore>(
            WebEngineContext::commandLine()));

    QString userPrefStorePath = profileAdapter.dataPath();
    if (profileAdapter.isOffTheRecord() || profileAdapter.storageName().isEmpty()) {
        factory.set_user_prefs(new InMemoryPrefStore);
    } else {
        userPrefStorePath += QDir::separator();
        userPrefStorePath += QStringLiteral("user_prefs.json");
        factory.set_user_prefs(base::MakeRefCounted<JsonPrefStore>(toFilePath(userPrefStorePath)));
    }

    auto registry = base::MakeRefCounted<PrefRegistrySimple>();
    PrefProxyConfigTrackerImpl::RegisterPrefs(registry.get());

#if QT_CONFIG(webengine_spellchecker)
    // Initial spellcheck settings
    registry->RegisterStringPref(language::prefs::kAcceptLanguages, std::string());
    registry->RegisterListPref(spellcheck::prefs::kSpellCheckDictionaries);
    registry->RegisterListPref(spellcheck::prefs::kSpellCheckForcedDictionaries);
    registry->RegisterListPref(spellcheck::prefs::kSpellCheckBlacklistedDictionaries);
    registry->RegisterStringPref(spellcheck::prefs::kSpellCheckDictionary, std::string());
    registry->RegisterBooleanPref(spellcheck::prefs::kSpellCheckEnable, false);
    registry->RegisterBooleanPref(spellcheck::prefs::kSpellCheckUseSpellingService, false);
#endif // QT_CONFIG(webengine_spellchecker)
    registry->RegisterBooleanPref(prefs::kShowInternalAccessibilityTree, false);
    registry->RegisterBooleanPref(prefs::kAccessibilityImageLabelsEnabled, false);
    registry->RegisterIntegerPref(prefs::kNotificationNextPersistentId, 10000);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    registry->RegisterDictionaryPref(extensions::pref_names::kExtensions);
    registry->RegisterListPref(extensions::pref_names::kInstallAllowList);
    registry->RegisterListPref(extensions::pref_names::kInstallDenyList);
    registry->RegisterDictionaryPref(extensions::pref_names::kInstallForceList);
    registry->RegisterDictionaryPref(extensions::pref_names::kLoginScreenExtensions);
    registry->RegisterListPref(extensions::pref_names::kAllowedTypes);
    registry->RegisterBooleanPref(extensions::pref_names::kStorageGarbageCollect, false);
    registry->RegisterListPref(extensions::pref_names::kAllowedInstallSites);
    registry->RegisterStringPref(extensions::pref_names::kLastChromeVersion, std::string());
    registry->RegisterListPref(extensions::pref_names::kNativeMessagingBlacklist);
    registry->RegisterListPref(extensions::pref_names::kNativeMessagingWhitelist);
    registry->RegisterBooleanPref(extensions::pref_names::kNativeMessagingUserLevelHosts, true);
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

    // Media device salt id key
    // Can't be a random value since every time we run the setup code the
    // default value will be different. We'll need to initialize it later.
    registry->RegisterStringPref(kPrefMediaDeviceIDSalt, std::string());

    m_prefService = factory.Create(registry);

    // Initialize salt value if none was stored before
    if (m_prefService->GetString(kPrefMediaDeviceIDSalt).empty()) {
        m_prefService->SetString(kPrefMediaDeviceIDSalt,
                content::BrowserContext::CreateRandomMediaDeviceIDSalt());
    }

#if QT_CONFIG(webengine_spellchecker)
    // Ignore stored values for these options to preserve backwards compatibility.
    m_prefService->ClearPref(spellcheck::prefs::kSpellCheckEnable);
    m_prefService->ClearPref(spellcheck::prefs::kSpellCheckDictionaries);
#endif // QT_CONFIG(webengine_spellchecker)
}

void PrefServiceAdapter::commit()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    // Make sure modified preferences are written to disk
    m_prefService->CommitPendingWrite();
}

PrefService* PrefServiceAdapter::prefService()
{
    return m_prefService.get();
}

const PrefService* PrefServiceAdapter::prefService() const
{
    return m_prefService.get();
}

std::string PrefServiceAdapter::mediaDeviceIdSalt() const
{
    return m_prefService->GetString(kPrefMediaDeviceIDSalt);
}

#if QT_CONFIG(webengine_spellchecker)

void PrefServiceAdapter::setSpellCheckLanguages(const QStringList &languages)
{
    StringListPrefMember dictionaries_pref;
    dictionaries_pref.Init(spellcheck::prefs::kSpellCheckDictionaries, m_prefService.get());
    std::vector<std::string> dictionaries;
    dictionaries.reserve(languages.size());
    for (const auto &language : languages)
        dictionaries.push_back(language.toStdString());
    dictionaries_pref.SetValue(dictionaries);
}

QStringList PrefServiceAdapter::spellCheckLanguages() const
{
    QStringList spellcheck_dictionaries;
    for (const auto &value : *m_prefService->GetList(spellcheck::prefs::kSpellCheckDictionaries)) {
        std::string dictionary;
        if (value.GetAsString(&dictionary))
            spellcheck_dictionaries.append(QString::fromStdString(dictionary));
    }

    return spellcheck_dictionaries;
}

void PrefServiceAdapter::setSpellCheckEnabled(bool enabled)
{
    m_prefService->SetBoolean(spellcheck::prefs::kSpellCheckEnable, enabled);
}

bool PrefServiceAdapter::isSpellCheckEnabled() const
{
    return m_prefService->GetBoolean(spellcheck::prefs::kSpellCheckEnable);
}

#endif // QT_CONFIG(webengine_spellchecker)
}
