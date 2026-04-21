// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "runtime_api_delegate_qt.h"

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include "extensions/extension_manager.h"
#include "extensions/extension_system_qt.h"

#include "components/update_client/update_query_params.h"

using extensions::api::runtime::PlatformInfo;

namespace extensions {

RuntimeAPIDelegateQt::RuntimeAPIDelegateQt(content::BrowserContext *browser_context)
    : browser_context_(browser_context)
{
    DCHECK(browser_context_);
}

RuntimeAPIDelegateQt::~RuntimeAPIDelegateQt() = default;

void RuntimeAPIDelegateQt::AddUpdateObserver(UpdateObserver *observer) { }

void RuntimeAPIDelegateQt::RemoveUpdateObserver(UpdateObserver *observer) { }

void RuntimeAPIDelegateQt::ReloadExtension(const ExtensionId &extension_id)
{
#if QT_CONFIG(webengine_extensions)
    auto *manager = static_cast<ExtensionSystemQt *>(ExtensionSystem::Get(browser_context_))
                            ->extensionManager();
    manager->reloadExtension(extension_id);
#endif
}

bool RuntimeAPIDelegateQt::CheckForUpdates(const ExtensionId &extension_id,
                                           UpdateCheckCallback callback)
{
    return false;
}

void RuntimeAPIDelegateQt::OpenURL(const GURL &uninstall_url) { }

bool RuntimeAPIDelegateQt::GetPlatformInfo(PlatformInfo *info)
{
    std::string_view os = update_client::UpdateQueryParams::GetOS();
    if (os == "mac") {
        info->os = extensions::api::runtime::PlatformOs::kMac;
    } else if (os == "win") {
        info->os = extensions::api::runtime::PlatformOs::kWin;
    } else if (os == "cros") {
        info->os = extensions::api::runtime::PlatformOs::kCros;
    } else if (os == "linux") {
        info->os = extensions::api::runtime::PlatformOs::kLinux;
    } else if (os == "openbsd") {
        info->os = extensions::api::runtime::PlatformOs::kOpenbsd;
    } else if (os == "android") {
        info->os = extensions::api::runtime::PlatformOs::kAndroid;
    } else {
        NOTREACHED() << "Platform not supported: " << os;
    }

    std::string_view arch = update_client::UpdateQueryParams::GetArch();
    if (arch == "arm") {
        info->arch = extensions::api::runtime::PlatformArch::kArm;
    } else if (arch == "arm64") {
        info->arch = extensions::api::runtime::PlatformArch::kArm64;
    } else if (arch == "x86") {
        info->arch = extensions::api::runtime::PlatformArch::kX86_32;
    } else if (arch == "x64") {
        info->arch = extensions::api::runtime::PlatformArch::kX86_64;
    } else if (arch == "mipsel") {
        info->arch = extensions::api::runtime::PlatformArch::kMips;
    } else if (arch == "mips64el") {
        info->arch = extensions::api::runtime::PlatformArch::kMips64;
    } else if (arch == "riscv64") {
        info->arch = extensions::api::runtime::PlatformArch::kRiscv64;
    } else {
        NOTREACHED();
    }

    return true;
}

bool RuntimeAPIDelegateQt::RestartDevice(std::string *error_message)
{
    *error_message = "Restart is only supported on ChromeOS.";
    return false;
}

} // namespace extensions
