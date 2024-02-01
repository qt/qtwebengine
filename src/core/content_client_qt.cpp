// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content_client_qt.h"

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/json/json_string_value_serializer.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "base/version.h"
#include "content/public/common/cdm_info.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "extensions/buildflags/buildflags.h"
#include "extensions/common/constants.h"
#include "media/base/media_switches.h"
#include "media/base/video_codecs.h"
#include "media/cdm/supported_audio_codecs.h"
#include "media/media_buildflags.h"
#include "ui/base/layout.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "type_conversion.h"

#include <QCoreApplication>
#include <QFile>
#include <QLibraryInfo>
#include <QString>


#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
#include "media/cdm/cdm_paths.h"  // nogncheck
#include "media/cdm/clear_key_cdm_common.h"
#include "third_party/widevine/cdm/buildflags.h"
#include "third_party/widevine/cdm/widevine_cdm_common.h"
#if BUILDFLAG(ENABLE_WIDEVINE) && !BUILDFLAG(ENABLE_WIDEVINE_CDM_COMPONENT)
#define WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT

// File name of the CDM on different platforms.
const char kWidevineCdmFileName[] =
#if BUILDFLAG(IS_MAC)
    "widevinecdm.plugin";
#elif BUILDFLAG(IS_WIN)
    "widevinecdm.dll";
#else  // OS_LINUX, etc.
    "libwidevinecdm.so";
#endif
#endif
#endif  // BUILDFLAG(ENABLE_LIBRARY_CDMS)

#if QT_CONFIG(webengine_printing_and_pdf)
#include "components/pdf/common/internal_plugin_helpers.h"
#include "pdf/pdf.h"
const char kPdfPluginPath[] = "internal-pdf-viewer";
#endif // QT_CONFIG(webengine_printing_and_pdf)

using Robustness = content::CdmInfo::Robustness;

static QString webenginePluginsPath()
{
    // Look for plugins in /plugins/webengine or application dir.
    static bool initialized = false;
    static QString potentialPluginsPath = QLibraryInfo::path(QLibraryInfo::PluginsPath) % QLatin1String("/webengine");
    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialPluginsPath))
            potentialPluginsPath = QCoreApplication::applicationDirPath();
    }
    return potentialPluginsPath;
}

#if defined(Q_OS_WIN)
#include <shlobj.h>
static QString getLocalAppDataDir()
{
    QString result;
    wchar_t path[MAX_PATH];
    if (SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, FALSE))
        result = QDir::fromNativeSeparators(QString::fromWCharArray(path));
    return result;
}

static QString getProgramFilesDir(bool x86Dir = false)
{
    QString result;
    wchar_t path[MAX_PATH];
    if (SHGetSpecialFolderPath(0, path, x86Dir ? CSIDL_PROGRAM_FILESX86 : CSIDL_PROGRAM_FILES, FALSE))
        result = QDir::fromNativeSeparators(QString::fromWCharArray(path));
    return result;
}
#endif

#if QT_CONFIG(webengine_pepper_plugins)

// The plugin logic is based on chrome/common/chrome_content_client.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "content/public/common/content_plugin_info.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

static QString ppapiPluginsPath()
{
    // Look for plugins in /plugins/ppapi or application dir.
    static bool initialized = false;
    static QString potentialPluginsPath = QLibraryInfo::path(QLibraryInfo::PluginsPath) % QLatin1String("/ppapi");
    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialPluginsPath))
            potentialPluginsPath = QCoreApplication::applicationDirPath();
    }
    return potentialPluginsPath;
}

void ComputeBuiltInPlugins(std::vector<content::ContentPluginInfo> *plugins)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    static constexpr char kPDFPluginExtension[] = "pdf";
    static constexpr char kPDFPluginDescription[] = "Portable Document Format";
    content::ContentPluginInfo pdf_info;
    pdf_info.is_internal = true;
    pdf_info.is_out_of_process = true;
    pdf_info.name = "Chromium PDF Viewer";
    pdf_info.description = kPDFPluginDescription;
    pdf_info.path = base::FilePath::FromUTF8Unsafe(kPdfPluginPath);
    content::WebPluginMimeType pdf_mime_type(
        pdf::kInternalPluginMimeType, kPDFPluginExtension, kPDFPluginDescription);
    pdf_info.mime_types.push_back(pdf_mime_type);
    plugins->push_back(pdf_info);
#endif // QT_CONFIG(webengine_printing_and_pdf)
}

namespace QtWebEngineCore {

void ContentClientQt::AddPlugins(std::vector<content::ContentPluginInfo> *plugins)
{
    ComputeBuiltInPlugins(plugins);
}

} // namespace QtWebEngineCore
#endif // QT_CONFIG(webengine_pepper_plugins)

namespace QtWebEngineCore {

#if defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)
#if defined(Q_OS_LINUX)
static const QDir widevineCdmDirHint(const QDir &widevineDir)
{
    const QString hintFilePath = widevineDir.absolutePath() % QDir::separator()
            % QLatin1String("latest-component-updated-widevine-cdm");
    if (!QFileInfo::exists(hintFilePath)) {
        // CDM hint file does not exist.
        return widevineDir;
    }

    std::string jsonString;
    if (!base::ReadFileToString(toFilePath(hintFilePath), &jsonString)) {
        // Could not read the CDM hint file.
        return widevineDir;
    }

    JSONStringValueDeserializer deserializer(jsonString);
    std::unique_ptr<base::Value> dict = deserializer.Deserialize(nullptr, nullptr);
    if (!dict || !dict->is_dict()) {
        // Could not deserialize the CDM hint file.
        return widevineDir;
    }

    std::string *widevineCdmDirPath = dict->FindStringKey("Path");
    if (!widevineCdmDirPath)
        return widevineDir;

    return QDir(QString::fromStdString(*widevineCdmDirPath));
}
#endif // defined(Q_OS_LINUX)

static bool IsWidevineAvailable(base::FilePath *cdm_path,
                                media::CdmCapability *capability)
{
    QStringList pluginPaths;
    const base::CommandLine::StringType widevine_argument = base::CommandLine::ForCurrentProcess()->GetSwitchValueNative(switches::kCdmWidevinePath);
    if (!widevine_argument.empty())
        pluginPaths << QtWebEngineCore::toQt(widevine_argument);
    else {
        pluginPaths << webenginePluginsPath() + QStringLiteral("/") + QString::fromLatin1(kWidevineCdmFileName);
#if QT_CONFIG(webengine_pepper_plugins)
        pluginPaths << ppapiPluginsPath() + QStringLiteral("/") + QString::fromLatin1(kWidevineCdmFileName);
#endif
#if defined(Q_OS_OSX)
    QDir potentialWidevineDir("/Applications/Google Chrome.app/Contents/Frameworks");
    if (potentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = potentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                                                               QDir::Name | QDir::Reversed);
        const QString library = QLatin1String("/Versions/Current/Libraries/"
                                              "WidevineCdm/_platform_specific/mac_x64/libwidevinecdm.dylib");
        for (const QFileInfo &info : widevineVersionDirs)
            pluginPaths << info.absoluteFilePath() + library;
    }

    QDir oldPotentialWidevineDir(QDir::homePath() + "/Library/Application Support/Google/Chrome/WidevineCDM");
    if (oldPotentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = oldPotentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (int i = 0; i < widevineVersionDirs.size(); ++i) {
            QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/mac_x64/" + QString::fromLatin1(kWidevineCdmFileName);
            pluginPaths << potentialWidevinePluginPath;
        }
    }
#elif defined(Q_OS_WIN)
    const QString googleChromeDir = QLatin1String("/Google/Chrome/Application");
    const QStringList programFileDirs{getProgramFilesDir() + googleChromeDir,
                                      getProgramFilesDir(true) + googleChromeDir};
    for (const QString &dir : programFileDirs) {
        QDir d(dir);
        if (d.exists()) {
            QFileInfoList widevineVersionDirs = d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
            for (int i = 0; i < widevineVersionDirs.size(); ++i) {
                QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
#ifdef WIN64
                QString potentialWidevinePluginPath = versionDirPath +
                                                        "/WidevineCdm/_platform_specific/win_x64/" +
                                                        QString::fromLatin1(kWidevineCdmFileName);
#else
                QString potentialWidevinePluginPath = versionDirPath +
                                                        "/WidevineCdm/_platform_specific/win_x86/" +
                                                        QString::fromLatin1(kWidevineCdmFileName);
#endif
                pluginPaths << potentialWidevinePluginPath;
            }
        }
    }
    QDir potentialWidevineDir(getLocalAppDataDir() + "/Google/Chrome/User Data/WidevineCDM");
    if (potentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = potentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (int i = 0; i < widevineVersionDirs.size(); ++i) {
            QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
#ifdef WIN64
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/win_x64/" + QString::fromLatin1(kWidevineCdmFileName);
#else
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/win_x86/" + QString::fromLatin1(kWidevineCdmFileName);
#endif
            pluginPaths << potentialWidevinePluginPath;
        }
    }
#elif defined(Q_OS_LINUX)
        QList<QDir> potentialWidevineVersionDirs;

        // Google Chrome widevine modules
        QDir chromeWidevineDir(QDir::homePath() + "/.config/google-chrome/WidevineCdm");
        if (chromeWidevineDir.exists())
            potentialWidevineVersionDirs << widevineCdmDirHint(chromeWidevineDir);

        // Firefox widevine modules
        QDir firefoxPotentialProfilesDir(QDir::homePath() + "/.mozilla/firefox");
        if (firefoxPotentialProfilesDir.exists()) {
            QFileInfoList firefoxProfileDirs = firefoxPotentialProfilesDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
            for (const QFileInfo &info : firefoxProfileDirs) {
                QDir widevinePluginsDir(info.absoluteFilePath() + "/gmp-widevinecdm");
                if (widevinePluginsDir.exists())
                    potentialWidevineVersionDirs << widevinePluginsDir;
            }
        }

        // Chromium widevine modules (might not work with proprietary codecs)
        QDir chromiumWidevineDir(QDir::homePath() + "/.config/chromium/WidevineCdm");
        if (chromiumWidevineDir.exists())
            potentialWidevineVersionDirs << widevineCdmDirHint(chromiumWidevineDir);

        // Search for widewine versions
        for (const QDir &dir : potentialWidevineVersionDirs) {
            QFileInfoList widevineVersionDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
            widevineVersionDirs.prepend(QFileInfo(dir.absolutePath()));
            // ### alternatively look up in the manifest.json and take the path from there.
#if Q_PROCESSOR_WORDSIZE == 8
            const QString library = QLatin1String("/_platform_specific/linux_x64/libwidevinecdm.so");
#else
            const QString library = QLatin1String("/_platform_specific/linux_x86/libwidevinecdm.so");
#endif
            for (const QFileInfo &info : widevineVersionDirs) {
                pluginPaths << info.absoluteFilePath() + "/libwidevinecdm.so";
                pluginPaths << info.absoluteFilePath() + library;
            }
        }

        // Fixed paths:
        pluginPaths << QStringLiteral("/usr/lib/chromium/libwidevinecdm.so") // Arch
                    << QStringLiteral("/usr/lib/chromium-browser/libwidevinecdm.so") // Ubuntu/neon
                    << QStringLiteral("/usr/lib64/chromium/libwidevinecdm.so") // OpenSUSE style
#if Q_PROCESSOR_WORDSIZE == 8
                    << QStringLiteral("/usr/lib64/chromium-browser/WidevineCdm/_platform_specific/linux_x64/libwidevinecdm.so") // Gentoo
                    << QStringLiteral("/opt/google/chrome/WidevineCdm/_platform_specific/linux_x64/libwidevinecdm.so") // Old Google Chrome
#else
                    << QStringLiteral("/usr/lib/chromium-browser/WidevineCdm/_platform_specific/linux_x86/libwidevinecdm.so") // Gentoo
                    << QStringLiteral("/opt/google/chrome/WidevineCdm/_platform_specific/linux_x86/libwidevinecdm.so") // Old Google Chrome
#endif
                    << QStringLiteral("/opt/google/chrome/libwidevinecdm.so"); // Older Google Chrome
#endif
    }

    for (const QString &pluginPath : std::as_const(pluginPaths)) {
        *cdm_path = QtWebEngineCore::toFilePath(pluginPath);
        if (base::PathExists(*cdm_path)) {
            // Add the supported codecs as if they came from the component manifest.
            // This list must match the CDM that is being bundled with Chrome.
            const std::vector<media::VideoCodecProfile> kAllProfiles = {};
            capability->video_codecs.emplace(media::VideoCodec::kVP8, kAllProfiles);
            capability->video_codecs.emplace(media::VideoCodec::kVP9, kAllProfiles);
            capability->video_codecs.emplace(media::VideoCodec::kAV1, kAllProfiles);
#if BUILDFLAG(USE_PROPRIETARY_CODECS)
            capability->video_codecs.emplace(media::VideoCodec::kH264, kAllProfiles);
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)
#if BUILDFLAG(ENABLE_PLATFORM_HEVC)
            capability->video_codecs.emplace(media::VideoCodec::kHEVC, kAllProfiles);
#endif
            capability->audio_codecs = media::GetCdmSupportedAudioCodecs();

            // Add the supported encryption schemes as if they came from the
            // component manifest. This list must match the CDM that is being
            // bundled with Chrome.
            capability->encryption_schemes.insert(media::EncryptionScheme::kCenc);
            capability->encryption_schemes.insert(media::EncryptionScheme::kCbcs);

            // Temporary session is always supported.
            capability->session_types.insert(media::CdmSessionType::kTemporary);

            return true;
        }
    }
    return false;
}
#endif  // defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)

void ContentClientQt::AddContentDecryptionModules(std::vector<content::CdmInfo> *cdms,
                                                  std::vector<media::CdmHostFilePath> *cdm_host_file_paths)
{
    Q_UNUSED(cdm_host_file_paths);
    if (cdms) {
#if defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)
        base::FilePath cdm_path;
        media::CdmCapability capability;
        if (IsWidevineAvailable(&cdm_path, &capability)) {
            const base::Version version;
            cdms->push_back(content::CdmInfo(kWidevineKeySystem, Robustness::kSoftwareSecure, std::move(capability),
                                             /*supports_sub_key_systems=*/false, kWidevineCdmDisplayName,
                                             kWidevineCdmType, version, cdm_path));
        }
#endif  // defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
        // Register Clear Key CDM if specified in command line.
        base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
        base::FilePath clear_key_cdm_path = command_line->GetSwitchValuePath(switches::kClearKeyCdmPathForTesting);
        if (!clear_key_cdm_path.empty() && base::PathExists(clear_key_cdm_path)) {
            // TODO(crbug.com/764480): Remove these after we have a central place for
            // External Clear Key (ECK) related information.
            // Normal External Clear Key key system.
            const char kExternalClearKeyKeySystem[] = "org.chromium.externalclearkey";
            // A variant of ECK key system that has a different GUID.
            const char kExternalClearKeyDifferentGuidTestKeySystem[] =
                    "org.chromium.externalclearkey.differentguid";

            // Supported codecs are hard-coded in ExternalClearKeyProperties.
            media::CdmCapability capability(
                {}, {}, {media::EncryptionScheme::kCenc, media::EncryptionScheme::kCbcs},
                {media::CdmSessionType::kTemporary,
                 media::CdmSessionType::kPersistentLicense});

            // Register kExternalClearKeyDifferentGuidTestKeySystem first separately.
            // Otherwise, it'll be treated as a sub-key-system of normal
            // kExternalClearKeyKeySystem. See MultipleCdmTypes test in
            // ECKEncryptedMediaTest.
            cdms->push_back(content::CdmInfo(kExternalClearKeyDifferentGuidTestKeySystem,
                                             Robustness::kSoftwareSecure, capability,
                                             /*supports_sub_key_systems=*/false, media::kClearKeyCdmDisplayName,
                                             media::kClearKeyCdmDifferentCdmType, base::Version("0.1.0.0"),
                                             clear_key_cdm_path));

            cdms->push_back(content::CdmInfo(kExternalClearKeyKeySystem,
                                             Robustness::kSoftwareSecure, capability,
                                             /*supports_sub_key_systems=*/true, media::kClearKeyCdmDisplayName,
                                             media::kClearKeyCdmType, base::Version("0.1.0.0"),
                                             clear_key_cdm_path));
        }
#endif  // BUILDFLAG(ENABLE_LIBRARY_CDMS)
    }
}

void ContentClientQt::AddAdditionalSchemes(Schemes* schemes)
{
    // Matching ChromeContentClient::AddAdditionalSchemes
    schemes->standard_schemes.push_back(extensions::kExtensionScheme);
    schemes->secure_schemes.push_back(extensions::kExtensionScheme);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    schemes->service_worker_schemes.push_back(extensions::kExtensionScheme);
    schemes->cors_enabled_schemes.push_back(extensions::kExtensionScheme);
    schemes->csp_bypassing_schemes.push_back(extensions::kExtensionScheme);
#endif
}

base::StringPiece ContentClientQt::GetDataResource(int resource_id, ui::ResourceScaleFactor scale_factor)
{
    return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(resource_id, scale_factor);
}

base::RefCountedMemory *ContentClientQt::GetDataResourceBytes(int resource_id)
{
    return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

gfx::Image &ContentClientQt::GetNativeImageNamed(int resource_id)
{
    return ui::ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
}

std::u16string ContentClientQt::GetLocalizedString(int message_id)
{
    return l10n_util::GetStringUTF16(message_id);
}

// This method is a copy from chrome/common/chrome_content_client.cc:
// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.
blink::OriginTrialPolicy *ContentClientQt::GetOriginTrialPolicy()
{
    // Prevent initialization race (see crbug.com/721144). There may be a
    // race when the policy is needed for worker startup (which happens on a
    // separate worker thread).
    base::AutoLock auto_lock(origin_trial_policy_lock_);
    if (!origin_trial_policy_)
        origin_trial_policy_ = std::make_unique<embedder_support::OriginTrialPolicyImpl>();
    return origin_trial_policy_.get();
}

} // namespace QtWebEngineCore
