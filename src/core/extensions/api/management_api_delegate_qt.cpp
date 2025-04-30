// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "management_api_delegate_qt.h"

#include "extensions/extension_manager.h"
#include "extensions/extension_system_qt.h"

namespace extensions {

ManagementAPIDelegateQt::ManagementAPIDelegateQt() { }

ManagementAPIDelegateQt::~ManagementAPIDelegateQt() { }

bool ManagementAPIDelegateQt::LaunchAppFunctionDelegate(const Extension *extension,
                                                        content::BrowserContext *context) const
{
    return false;
}

GURL ManagementAPIDelegateQt::GetFullLaunchURL(const Extension *extension) const
{
    return GURL();
}

LaunchType ManagementAPIDelegateQt::GetLaunchType(const ExtensionPrefs *prefs,
                                                  const Extension *extension) const
{
    return LaunchType::LAUNCH_TYPE_DEFAULT;
}

std::unique_ptr<InstallPromptDelegate> ManagementAPIDelegateQt::SetEnabledFunctionDelegate(
        content::WebContents *web_contents, content::BrowserContext *browser_context,
        const Extension *extension, base::OnceCallback<void(bool)> callback) const
{
    return std::make_unique<InstallPromptDelegate>();
}

std::unique_ptr<UninstallDialogDelegate>
ManagementAPIDelegateQt::UninstallFunctionDelegate(ManagementUninstallFunctionBase *function,
                                                   const Extension *target_extension,
                                                   bool show_programmatic_uninstall_ui) const
{
    return std::make_unique<UninstallDialogDelegate>();
}

bool ManagementAPIDelegateQt::CreateAppShortcutFunctionDelegate(
        ManagementCreateAppShortcutFunction *function, const Extension *extension,
        std::string *error) const
{
    return false;
}

std::unique_ptr<AppForLinkDelegate> ManagementAPIDelegateQt::GenerateAppForLinkFunctionDelegate(
        ManagementGenerateAppForLinkFunction *function, content::BrowserContext *context,
        const std::string &title, const GURL &launch_url) const
{
    return std::make_unique<AppForLinkDelegateQt>();
}

bool ManagementAPIDelegateQt::CanContextInstallWebApps(content::BrowserContext *context) const
{
    return false;
}

void ManagementAPIDelegateQt::InstallOrLaunchReplacementWebApp(
        content::BrowserContext *context, const GURL &web_app_url,
        ManagementAPIDelegate::InstallOrLaunchWebAppCallback callback) const
{
}

void ManagementAPIDelegateQt::EnableExtension(content::BrowserContext *context,
                                              const ExtensionId &extension_id) const
{
    auto *manager =
            static_cast<ExtensionSystemQt *>(ExtensionSystem::Get(context))->extensionManager();
    manager->setExtensionEnabled(extension_id, true);
}

void ManagementAPIDelegateQt::DisableExtension(content::BrowserContext *context,
                                               const Extension *source_extension,
                                               const ExtensionId &extension_id,
                                               disable_reason::DisableReason disable_reason) const
{
    auto *manager =
            static_cast<ExtensionSystemQt *>(ExtensionSystem::Get(context))->extensionManager();
    manager->setExtensionEnabled(extension_id, false);
}

bool ManagementAPIDelegateQt::UninstallExtension(content::BrowserContext *context,
                                                 const ExtensionId &transient_extension_id,
                                                 UninstallReason reason,
                                                 std::u16string *error) const
{
    auto *manager =
            static_cast<ExtensionSystemQt *>(ExtensionSystem::Get(context))->extensionManager();
    if (!manager->isExtensionInstalled(transient_extension_id)) {
        *error = u"This extension was not installed";
        return false;
    }

    manager->uninstallExtension(transient_extension_id);
    return true;
}

void ManagementAPIDelegateQt::SetLaunchType(content::BrowserContext *context,
                                            const ExtensionId &extension_id,
                                            LaunchType launch_type) const
{
}

GURL ManagementAPIDelegateQt::GetIconURL(const Extension *extension, int icon_size,
                                         ExtensionIconSet::Match match, bool grayscale) const
{
    return GURL();
}

GURL ManagementAPIDelegateQt::GetEffectiveUpdateURL(const Extension &extension,
                                                    content::BrowserContext *context) const
{
    return GURL();
}

void ManagementAPIDelegateQt::ShowMv2DeprecationReEnableDialog(
        content::BrowserContext *context, content::WebContents *web_contents,
        const Extension &extension, base::OnceCallback<void(bool)> done_callback) const
{
}

} // namespace extensions
