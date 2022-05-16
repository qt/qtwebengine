// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/notreached.h"
#include "content/public/common/content_switches.h"
#include "sandbox/policy/switches.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/data_pack.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_switches.h"

#include "web_engine_library_info.h"

#if BUILDFLAG(IS_LINUX)
#include "base/posix/global_descriptors.h"
#include "global_descriptors_qt.h"
#endif

namespace ui {

void ResourceBundle::LoadCommonResources()
{
    // We repacked the resources we need and installed them. now let chromium mmap that file.
    AddDataPackFromPath(WebEngineLibraryInfo::getPath(QT_RESOURCES_100P_PAK), ui::k100Percent);
    AddDataPackFromPath(WebEngineLibraryInfo::getPath(QT_RESOURCES_200P_PAK), ui::k200Percent);
    AddDataPackFromPath(WebEngineLibraryInfo::getPath(QT_RESOURCES_PAK), ui::kScaleFactorNone);
    AddOptionalDataPackFromPath(WebEngineLibraryInfo::getPath(QT_RESOURCES_DEVTOOLS_PAK), ui::kScaleFactorNone);
}

gfx::Image& ResourceBundle::GetNativeImageNamed(int resource_id)
{
    return GetImageNamed(resource_id);
}

// static
bool ResourceBundle::LocaleDataPakExists(const std::string& locale)
{
#if BUILDFLAG(IS_LINUX)
    base::CommandLine *parsed_command_line = base::CommandLine::ForCurrentProcess();
    std::string process_type = parsed_command_line->GetSwitchValueASCII(switches::kProcessType);
    bool no_sandbox = parsed_command_line->HasSwitch(sandbox::policy::switches::kNoSandbox);
    if (process_type == switches::kRendererProcess && !no_sandbox) {
        // The Renderer Process is sandboxed thus only one locale is available in it.
        // The particular one is passed by the --lang command line option.
        if (!parsed_command_line->HasSwitch(switches::kLang) || parsed_command_line->GetSwitchValueASCII(switches::kLang) != locale)
            return false;

        auto global_descriptors = base::GlobalDescriptors::GetInstance();
        return global_descriptors->MaybeGet(kWebEngineLocale) != -1;
    }
#endif

    const auto path = GetLocaleFilePath(locale);
    return !path.empty() && base::PathExists(path);
}

std::string ResourceBundle::LoadLocaleResources(const std::string &pref_locale, bool /*crash_on_failure*/)
{
    DCHECK(!locale_resources_data_.get()) << "locale.pak already loaded";

    std::string app_locale = l10n_util::GetApplicationLocale(pref_locale, false /* set_icu_locale */);

#if BUILDFLAG(IS_LINUX)
    int locale_fd = base::GlobalDescriptors::GetInstance()->MaybeGet(kWebEngineLocale);
    if (locale_fd > -1) {
        std::unique_ptr<DataPack> data_pack(new DataPack(ui::k100Percent));
        data_pack->LoadFromFile(base::File(locale_fd));
        locale_resources_data_ = std::move(data_pack);
        return app_locale;
    }
#endif

    base::FilePath locale_file_path = GetOverriddenPakPath();
    if (locale_file_path.empty())
        locale_file_path = GetLocaleFilePath(app_locale);

    if (locale_file_path.empty()) {
        // It's possible that there is no locale.pak.
        LOG(WARNING) << "locale_file_path.empty() for locale " << app_locale;
        return std::string();
    }

    std::unique_ptr<DataPack> data_pack(new DataPack(ui::k100Percent));
    if (!data_pack->LoadFromPath(locale_file_path)) {
        UMA_HISTOGRAM_ENUMERATION("ResourceBundle.LoadLocaleResourcesError",
                                  logging::GetLastSystemErrorCode(), 16000);
        LOG(ERROR) << "failed to load locale.pak";
        NOTREACHED();
        return std::string();
    }

    locale_resources_data_ = std::move(data_pack);
    return app_locale;
}

}  // namespace ui
