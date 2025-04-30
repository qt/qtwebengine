// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "runtime_api_delegate_qt.h"

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
    auto *manager = static_cast<ExtensionSystemQt *>(ExtensionSystem::Get(browser_context_))
                            ->extensionManager();
    manager->reloadExtension(extension_id);
}

bool RuntimeAPIDelegateQt::CheckForUpdates(const ExtensionId &extension_id,
                                           UpdateCheckCallback callback)
{
    return false;
}

void RuntimeAPIDelegateQt::OpenURL(const GURL &uninstall_url) { }

bool RuntimeAPIDelegateQt::GetPlatformInfo(PlatformInfo *info)
{
    const char *os = update_client::UpdateQueryParams::GetOS();
    if (strcmp(os, "mac") == 0) {
        info->os = extensions::api::runtime::PlatformOs::kMac;
    } else if (strcmp(os, "win") == 0) {
        info->os = extensions::api::runtime::PlatformOs::kWin;
    } else if (strcmp(os, "cros") == 0) {
        info->os = extensions::api::runtime::PlatformOs::kCros;
    } else if (strcmp(os, "linux") == 0) {
        info->os = extensions::api::runtime::PlatformOs::kLinux;
    } else if (strcmp(os, "openbsd") == 0) {
        info->os = extensions::api::runtime::PlatformOs::kOpenbsd;
    } else {
        NOTREACHED() << "Platform not supported: " << os;
    }

    const char *arch = update_client::UpdateQueryParams::GetArch();
    if (strcmp(arch, "arm") == 0) {
        info->arch = extensions::api::runtime::PlatformArch::kArm;
    } else if (strcmp(arch, "arm64") == 0) {
        info->arch = extensions::api::runtime::PlatformArch::kArm64;
    } else if (strcmp(arch, "x86") == 0) {
        info->arch = extensions::api::runtime::PlatformArch::kX86_32;
    } else if (strcmp(arch, "x64") == 0) {
        info->arch = extensions::api::runtime::PlatformArch::kX86_64;
    } else if (strcmp(arch, "mipsel") == 0) {
        info->arch = extensions::api::runtime::PlatformArch::kMips;
    } else if (strcmp(arch, "mips64el") == 0) {
        info->arch = extensions::api::runtime::PlatformArch::kMips64;
    } else {
        NOTREACHED();
    }

    const char *nacl_arch = update_client::UpdateQueryParams::GetNaclArch();
    if (strcmp(nacl_arch, "arm") == 0) {
        info->nacl_arch = extensions::api::runtime::PlatformNaclArch::kArm;
    } else if (strcmp(nacl_arch, "x86-32") == 0) {
        info->nacl_arch = extensions::api::runtime::PlatformNaclArch::kX86_32;
    } else if (strcmp(nacl_arch, "x86-64") == 0) {
        info->nacl_arch = extensions::api::runtime::PlatformNaclArch::kX86_64;
    } else if (strcmp(nacl_arch, "mips32") == 0) {
        info->nacl_arch = extensions::api::runtime::PlatformNaclArch::kMips;
    } else if (strcmp(nacl_arch, "mips64") == 0) {
        info->nacl_arch = extensions::api::runtime::PlatformNaclArch::kMips64;
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
