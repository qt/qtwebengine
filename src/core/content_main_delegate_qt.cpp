// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content_main_delegate_qt.h"

#include "base/command_line.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/no_destructor.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"
#include "net/grit/net_resources.h"
#include "net/base/net_module.h"
#include "sandbox/policy/switches.h"
#include "url/url_util_qt.h"

#include "content_client_qt.h"
#include "renderer/content_renderer_client_qt.h"
#include "type_conversion.h"
#include "web_engine_context.h"
#include "web_engine_library_info.h"

#if defined(ARCH_CPU_ARM_FAMILY) && (BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_LINUX))
#include "base/cpu.h"
#endif

#if BUILDFLAG(IS_LINUX)
#include "media/audio/audio_manager.h"
#include "ui/base/ui_base_switches.h"
#endif

#if BUILDFLAG(IS_WIN)
#include "media/gpu/windows/dxva_video_decode_accelerator_win.h"
#include "media/gpu/windows/media_foundation_video_encode_accelerator_win.h"
#endif

#if BUILDFLAG(IS_MAC)
#include "base/trace_event/trace_event.h"
#include "content/public/common/content_features.h"
#include "media/gpu/mac/vt_video_decode_accelerator_mac.h"
#endif

#include <QtCore/qcoreapplication.h>

namespace content {
ContentClient *GetContentClient();
}

namespace QtWebEngineCore {

namespace {

// The logic of this function is based on chrome/common/net/net_resource_provider.cc
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

// The net module doesn't have access to this HTML or the strings that need to
// be localized.  The Chrome locale will never change while we're running, so
// it's safe to have a static string that we always return a pointer into.
struct LazyDirectoryListerCacher
{
    LazyDirectoryListerCacher()
    {
        base::Value::Dict dict;
        dict.Set("header", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_HEADER));
        dict.Set("parentDirText", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_PARENT));
        dict.Set("headerName", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_NAME));
        dict.Set("headerSize", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_SIZE));
        dict.Set("headerDateModified", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_DATE_MODIFIED));
        dict.Set("language", l10n_util::GetLanguage(base::i18n::GetConfiguredLocale()));
        dict.Set("textdirection", base::i18n::IsRTL() ? "rtl" : "ltr");
        std::string html =
                webui::GetI18nTemplateHtml(
                    ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(IDR_DIR_HEADER_HTML),
                    std::move(dict));
        html_data = base::MakeRefCounted<base::RefCountedString>(std::move(html));
    }

    scoped_refptr<base::RefCountedMemory> html_data;
};

}  // namespace

static scoped_refptr<base::RefCountedMemory> PlatformResourceProvider(int key)
{
    static base::NoDestructor<LazyDirectoryListerCacher> lazy_dir_lister;

    if (IDR_DIR_HEADER_HTML == key)
        return lazy_dir_lister->html_data;

    return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(key);
}

// Logging logic is based on chrome/common/logging_chrome.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

static logging::LoggingDestination DetermineLogMode(const base::CommandLine& command_line)
{
#ifdef NDEBUG
    bool enable_logging = false;
    const char *kInvertLoggingSwitch = switches::kEnableLogging;
#else
    bool enable_logging = true;
    const char *kInvertLoggingSwitch = switches::kDisableLogging;
#endif

    if (command_line.HasSwitch(kInvertLoggingSwitch))
        enable_logging = !enable_logging;

    if (enable_logging)
        return logging::LOG_TO_SYSTEM_DEBUG_LOG | logging::LOG_TO_STDERR;
    else
        return logging::LOG_NONE;
}

void ContentMainDelegateQt::PreSandboxStartup()
{
#if defined(ARCH_CPU_ARM_FAMILY) && (BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_LINUX))
    // Create an instance of the CPU class to parse /proc/cpuinfo and cache
    // cpu_brand info.
    base::CPU cpu_info;
#endif

    net::NetModule::SetResourceProvider(PlatformResourceProvider);

    base::i18n::SetICUDefaultLocale(WebEngineLibraryInfo::getApplicationLocale());
    ui::ResourceBundle::InitSharedInstanceWithLocale(WebEngineLibraryInfo::getResolvedLocale(), nullptr, ui::ResourceBundle::LOAD_COMMON_RESOURCES);

    base::CommandLine* parsedCommandLine = base::CommandLine::ForCurrentProcess();
    logging::LoggingSettings settings;
    settings.logging_dest = DetermineLogMode(*parsedCommandLine);
    bool success = logging::InitLogging(settings);
    if (!success)
        qWarning("Failed to initialize Chromium logging");
    // view the logs with process/thread IDs and timestamps
    logging::SetLogItems(true, //enable_process_id
                         true, //enable_thread_id
                         true, //enable_timestamp
                         false //enable_tickcount
                        );

    if (logging::GetMinLogLevel() >= logging::LOGGING_INFO) {
        if (parsedCommandLine->HasSwitch(switches::kLoggingLevel)) {
            std::string logLevelValue = parsedCommandLine->GetSwitchValueASCII(switches::kLoggingLevel);
            int level = 0;
            if (base::StringToInt(logLevelValue, &level) && level >= logging::LOGGING_INFO && level < logging::LOGGING_NUM_SEVERITIES)
                logging::SetMinLogLevel(level);
        }
    }

#if BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_ANDROID)
    if (parsedCommandLine->HasSwitch(switches::kSingleProcess))
        setlocale(LC_NUMERIC, "C");
#endif

    bool isBrowserProcess = !parsedCommandLine->HasSwitch(switches::kProcessType);
    if (isBrowserProcess) {
        // from gpu_main.cc:
#if BUILDFLAG(IS_WIN)
        media::DXVAVideoDecodeAccelerator::PreSandboxInitialization();
        media::MediaFoundationVideoEncodeAccelerator::PreSandboxInitialization();
#endif

#if BUILDFLAG(IS_MAC)
        {
            TRACE_EVENT0("gpu", "Initialize VideoToolbox");
            media::InitializeVideoToolbox();
        }
#endif
    }

    if (parsedCommandLine->HasSwitch(switches::kApplicationName)) {
        std::string appName = parsedCommandLine->GetSwitchValueASCII(switches::kApplicationName);
        appName = QByteArray::fromPercentEncoding(QByteArray::fromStdString(appName)).toStdString();
        QCoreApplication::setApplicationName(QString::fromStdString(appName));
#if BUILDFLAG(IS_LINUX)
        media::AudioManager::SetGlobalAppName(appName);
#endif
    }
}

content::ContentClient *ContentMainDelegateQt::CreateContentClient()
{
    return &m_contentClient;
}

content::ContentBrowserClient *ContentMainDelegateQt::CreateContentBrowserClient()
{
    m_browserClient.reset(new ContentBrowserClientQt);
    return m_browserClient.get();
}

content::ContentGpuClient *ContentMainDelegateQt::CreateContentGpuClient()
{
    m_gpuClient.reset(new ContentGpuClientQt);
    return m_gpuClient.get();
}

content::ContentRendererClient *ContentMainDelegateQt::CreateContentRendererClient()
{
#if BUILDFLAG(IS_LINUX)
    base::CommandLine *parsedCommandLine = base::CommandLine::ForCurrentProcess();
    std::string process_type = parsedCommandLine->GetSwitchValueASCII(switches::kProcessType);
    bool no_sandbox = parsedCommandLine->HasSwitch(sandbox::policy::switches::kNoSandbox);

    // Reload locale if the renderer process is sandboxed
    if (process_type == switches::kRendererProcess && !no_sandbox) {
        if (parsedCommandLine->HasSwitch(switches::kLang)) {
            const std::string &locale = parsedCommandLine->GetSwitchValueASCII(switches::kLang);
            ui::ResourceBundle::GetSharedInstance().ReloadLocaleResources(locale);
        }
    }
#endif

    return new ContentRendererClientQt;
}

content::ContentUtilityClient *ContentMainDelegateQt::CreateContentUtilityClient()
{
    m_utilityClient.reset(new ContentUtilityClientQt);
    return m_utilityClient.get();
}

// see icu_util.cc
#define ICU_UTIL_DATA_FILE   0
#define ICU_UTIL_DATA_SHARED 1
#define ICU_UTIL_DATA_STATIC 2

static void SafeOverridePathImpl(const char *keyName, int key, const base::FilePath &path)
{
    if (path.empty())
        return;

    // Do not create directories for overridden paths.
    if (base::PathService::OverrideAndCreateIfNeeded(key, path, false, false))
        return;

    qWarning("Path override failed for key %s and path '%s'", keyName, path.value().c_str());
}

#define SafeOverridePath(KEY, PATH) SafeOverridePathImpl(#KEY, KEY, PATH)

absl::optional<int> ContentMainDelegateQt::BasicStartupComplete()
{
    SafeOverridePath(base::FILE_EXE, WebEngineLibraryInfo::getPath(base::FILE_EXE));
    SafeOverridePath(base::DIR_QT_LIBRARY_DATA, WebEngineLibraryInfo::getPath(base::DIR_QT_LIBRARY_DATA));
    SafeOverridePath(base::DIR_ASSETS, WebEngineLibraryInfo::getPath(base::DIR_ASSETS));
    SafeOverridePath(base::DIR_EXE, WebEngineLibraryInfo::getPath(base::DIR_ASSETS));
    SafeOverridePath(ui::DIR_LOCALES, WebEngineLibraryInfo::getPath(ui::DIR_LOCALES));
#if QT_CONFIG(webengine_spellchecker)
    SafeOverridePath(base::DIR_APP_DICTIONARIES, WebEngineLibraryInfo::getPath(base::DIR_APP_DICTIONARIES));
#endif

    url::CustomScheme::LoadSchemes(base::CommandLine::ForCurrentProcess());

    return absl::nullopt;
}

} // namespace QtWebEngineCore
