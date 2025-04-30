// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef MANAGEMENT_API_DELEGATE_QT_
#define MANAGEMENT_API_DELEGATE_QT_

#include "extensions/browser/api/management/management_api_delegate.h"

namespace extensions {

class AppForLinkDelegateQt : public AppForLinkDelegate
{
public:
    ~AppForLinkDelegateQt() override { }

    api::management::ExtensionInfo
    CreateExtensionInfoFromWebApp(const std::string &app_id,
                                  content::BrowserContext *context) override
    {
        return {};
    }
};

class ManagementAPIDelegateQt : public ManagementAPIDelegate
{
public:
    ManagementAPIDelegateQt();
    ~ManagementAPIDelegateQt() override;

    // ManagementAPIDelegate.
    bool LaunchAppFunctionDelegate(const Extension *extension,
                                   content::BrowserContext *context) const override;
    GURL GetFullLaunchURL(const Extension *extension) const override;
    LaunchType GetLaunchType(const ExtensionPrefs *prefs,
                             const Extension *extension) const override;
    std::unique_ptr<InstallPromptDelegate>
    SetEnabledFunctionDelegate(content::WebContents *web_contents,
                               content::BrowserContext *browser_context, const Extension *extension,
                               base::OnceCallback<void(bool)> callback) const override;
    std::unique_ptr<UninstallDialogDelegate>
    UninstallFunctionDelegate(ManagementUninstallFunctionBase *function,
                              const Extension *target_extension,
                              bool show_programmatic_uninstall_ui) const override;
    bool CreateAppShortcutFunctionDelegate(ManagementCreateAppShortcutFunction *function,
                                           const Extension *extension,
                                           std::string *error) const override;
    std::unique_ptr<AppForLinkDelegate>
    GenerateAppForLinkFunctionDelegate(ManagementGenerateAppForLinkFunction *function,
                                       content::BrowserContext *context, const std::string &title,
                                       const GURL &launch_url) const override;
    bool CanContextInstallWebApps(content::BrowserContext *context) const override;
    void InstallOrLaunchReplacementWebApp(
            content::BrowserContext *context, const GURL &web_app_url,
            ManagementAPIDelegate::InstallOrLaunchWebAppCallback callback) const override;
    void EnableExtension(content::BrowserContext *context,
                         const ExtensionId &extension_id) const override;
    void DisableExtension(content::BrowserContext *context, const Extension *source_extension,
                          const ExtensionId &extension_id,
                          disable_reason::DisableReason disable_reason) const override;
    bool UninstallExtension(content::BrowserContext *context,
                            const ExtensionId &transient_extension_id, UninstallReason reason,
                            std::u16string *error) const override;
    void SetLaunchType(content::BrowserContext *context, const ExtensionId &extension_id,
                       LaunchType launch_type) const override;
    GURL GetIconURL(const Extension *extension, int icon_size, ExtensionIconSet::Match match,
                    bool grayscale) const override;
    GURL GetEffectiveUpdateURL(const Extension &extension,
                               content::BrowserContext *context) const override;
    void
    ShowMv2DeprecationReEnableDialog(content::BrowserContext *context,
                                     content::WebContents *web_contents, const Extension &extension,
                                     base::OnceCallback<void(bool)> done_callback) const override;
};

} // namespace extensions
#endif // MANAGEMENT_API_DELEGATE_QT_
