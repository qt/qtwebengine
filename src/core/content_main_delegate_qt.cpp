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

#include "content_main_delegate_qt.h"

#include "base/command_line.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "media/gpu/buildflags.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"
#include "net/grit/net_resources.h"
#include "net/base/net_module.h"
#include "services/service_manager/embedder/switches.h"
#include "services/service_manager/sandbox/switches.h"
#include "url/url_util_qt.h"

#include "content_client_qt.h"
#include "renderer/content_renderer_client_qt.h"
#include "type_conversion.h"
#include "web_engine_context.h"
#include "web_engine_library_info.h"

#if defined(ARCH_CPU_ARM_FAMILY) && (defined(OS_ANDROID) || defined(OS_LINUX))
#include "base/cpu.h"
#endif

#if defined(OS_LINUX)
#include "media/audio/audio_manager.h"
#include "ui/base/ui_base_switches.h"
#endif

// must be included before vaapi_wrapper.h
#include <QtCore/qcoreapplication.h>

#if defined(OS_WIN)
#include "media/gpu/windows/dxva_video_decode_accelerator_win.h"
#include "media/gpu/windows/media_foundation_video_encode_accelerator_win.h"
#endif

#if defined(OS_MACOSX)
#include "content/public/common/content_features.h"
#include "media/gpu/mac/vt_video_decode_accelerator_mac.h"
#endif

#if BUILDFLAG(USE_VAAPI)
#include "media/gpu/vaapi/vaapi_wrapper.h"
#endif

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
        base::DictionaryValue dict;
        dict.SetString("header", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_HEADER));
        dict.SetString("parentDirText", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_PARENT));
        dict.SetString("headerName", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_NAME));
        dict.SetString("headerSize", l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_SIZE));
        dict.SetString("headerDateModified",
                       l10n_util::GetStringUTF16(IDS_DIRECTORY_LISTING_DATE_MODIFIED));
        dict.SetString("language", l10n_util::GetLanguage(base::i18n::GetConfiguredLocale()));
        dict.SetString("listingParsingErrorBoxText",
                       l10n_util::GetStringFUTF16(IDS_DIRECTORY_LISTING_PARSING_ERROR_BOX_TEXT,
                                                  toString16(QCoreApplication::applicationName())));
        dict.SetString("textdirection", base::i18n::IsRTL() ? "rtl" : "ltr");
        std::string html =
                webui::GetI18nTemplateHtml(
                    ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(IDR_DIR_HEADER_HTML),
                    &dict);
        html_data = base::RefCountedString::TakeString(&html);
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
#if defined(ARCH_CPU_ARM_FAMILY) && (defined(OS_ANDROID) || defined(OS_LINUX))
    // Create an instance of the CPU class to parse /proc/cpuinfo and cache
    // cpu_brand info.
    base::CPU cpu_info;
#endif

    net::NetModule::SetResourceProvider(PlatformResourceProvider);
    ui::ResourceBundle::InitSharedInstanceWithLocale(WebEngineLibraryInfo::getApplicationLocale(), nullptr, ui::ResourceBundle::LOAD_COMMON_RESOURCES);

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

    if (logging::GetMinLogLevel() >= logging::LOG_INFO) {
        if (parsedCommandLine->HasSwitch(switches::kLoggingLevel)) {
            std::string logLevelValue = parsedCommandLine->GetSwitchValueASCII(switches::kLoggingLevel);
            int level = 0;
            if (base::StringToInt(logLevelValue, &level) && level >= logging::LOG_INFO && level < logging::LOG_NUM_SEVERITIES)
                logging::SetMinLogLevel(level);
        }
    }

#if defined(OS_POSIX) && !defined(OS_ANDROID)
    if (parsedCommandLine->HasSwitch(switches::kSingleProcess))
        setlocale(LC_NUMERIC, "C");
#endif

    // from gpu_main.cc:
#if BUILDFLAG(USE_VAAPI)
    media::VaapiWrapper::PreSandboxInitialization();
#endif
#if defined(OS_WIN)
    media::DXVAVideoDecodeAccelerator::PreSandboxInitialization();
    media::MediaFoundationVideoEncodeAccelerator::PreSandboxInitialization();
#endif

#if defined(OS_MACOSX)
    if (base::FeatureList::IsEnabled(features::kMacV2GPUSandbox)) {
        TRACE_EVENT0("gpu", "Initialize VideoToolbox");
        media::InitializeVideoToolbox();
    }
#endif

    if (parsedCommandLine->HasSwitch(service_manager::switches::kApplicationName)) {
        std::string appName = parsedCommandLine->GetSwitchValueASCII(service_manager::switches::kApplicationName);
        appName = QByteArray::fromPercentEncoding(QByteArray::fromStdString(appName)).toStdString();
        QCoreApplication::setApplicationName(QString::fromStdString(appName));
#if defined(OS_LINUX)
        media::AudioManager::SetGlobalAppName(appName);
#endif
    }
}

void ContentMainDelegateQt::PostEarlyInitialization(bool)
{
    PostFieldTrialInitialization();
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
#if defined(OS_LINUX)
    base::CommandLine *parsedCommandLine = base::CommandLine::ForCurrentProcess();
    std::string process_type = parsedCommandLine->GetSwitchValueASCII(switches::kProcessType);
    bool no_sandbox = parsedCommandLine->HasSwitch(service_manager::switches::kNoSandbox);

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

bool ContentMainDelegateQt::BasicStartupComplete(int *exit_code)
{
    SafeOverridePath(base::FILE_EXE, WebEngineLibraryInfo::getPath(base::FILE_EXE));
    SafeOverridePath(base::DIR_QT_LIBRARY_DATA, WebEngineLibraryInfo::getPath(base::DIR_QT_LIBRARY_DATA));
    SafeOverridePath(ui::DIR_LOCALES, WebEngineLibraryInfo::getPath(ui::DIR_LOCALES));
#if QT_CONFIG(webengine_spellchecker)
    SafeOverridePath(base::DIR_APP_DICTIONARIES, WebEngineLibraryInfo::getPath(base::DIR_APP_DICTIONARIES));
#endif

    url::CustomScheme::LoadSchemes(base::CommandLine::ForCurrentProcess());

    return false;
}

} // namespace QtWebEngineCore
