// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RUNTIME_API_DELEGATE_QT_
#define RUNTIME_API_DELEGATE_QT_

#include "base/memory/raw_ptr.h"
#include "extensions/browser/api/runtime/runtime_api_delegate.h"
#include "extensions/common/extension_id.h"

namespace content {
class BrowserContext;
}

namespace extensions {

class RuntimeAPIDelegateQt : public RuntimeAPIDelegate
{
public:
    explicit RuntimeAPIDelegateQt(content::BrowserContext *browser_context);

    RuntimeAPIDelegateQt(const RuntimeAPIDelegateQt &) = delete;
    RuntimeAPIDelegateQt &operator=(const RuntimeAPIDelegateQt &) = delete;

    ~RuntimeAPIDelegateQt() override;

    // RuntimeAPIDelegate implementation.
    void AddUpdateObserver(UpdateObserver *observer) override;
    void RemoveUpdateObserver(UpdateObserver *observer) override;
    void ReloadExtension(const ExtensionId &extension_id) override;
    bool CheckForUpdates(const ExtensionId &extension_id, UpdateCheckCallback callback) override;
    void OpenURL(const GURL &uninstall_url) override;
    bool GetPlatformInfo(api::runtime::PlatformInfo *info) override;
    bool RestartDevice(std::string *error_message) override;

private:
    raw_ptr<content::BrowserContext> browser_context_;
};

} // namespace extensions

#endif // RUNTIME_API_DELEGATE_QT_
