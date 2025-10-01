// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EXTENSION_LOADER_H_
#define EXTENSION_LOADER_H_

#include <string>

#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "extensions/browser/extension_registrar.h"
#include "extensions/common/extension_set.h"

namespace content {
class BrowserContext;
}
namespace extensions {
class Extension;
class ExtensionRegistry;
}

namespace QtWebEngineCore {
class ExtensionManager;
}

namespace QtWebEngineCore {
class ExtensionLoader : public extensions::ExtensionRegistrar::Delegate
{
public:
    struct LoadingInfo
    {
        scoped_refptr<const extensions::Extension> extension{};
        std::string error{};
        base::FilePath path{};
    };
    static LoadingInfo loadExtensionOnFileThread(const base::FilePath &path);

    ExtensionLoader(content::BrowserContext *context, ExtensionManager *manager);
    ~ExtensionLoader();

    void loadExtension(const base::FilePath &path);
    void unloadExtension(const std::string &id);
    void reloadExtension(const std::string &id);

    void addExtension(scoped_refptr<const extensions::Extension> extension);
    extensions::ExtensionSet extensions() const;

    void enableExtension(const std::string &id);
    void disableExtension(const std::string &id);
    bool isExtensionEnabled(const std::string &id);
    bool isExtensionLoaded(const std::string &id);
    scoped_refptr<const extensions::Extension> getExtensionById(const std::string &id);

private:
    void loadExtensionFinished(const LoadingInfo &loadingInfo);

    // ExtensionRegistrar::Delegate:
    void PreAddExtension(const extensions::Extension *extension,
                         const extensions::Extension *old_extension) override { }
    void PostActivateExtension(scoped_refptr<const extensions::Extension> extension) override { }
    void PostDeactivateExtension(scoped_refptr<const extensions::Extension> extension) override { }
    void PreUninstallExtension(scoped_refptr<const extensions::Extension> extension) override { }
    void PostUninstallExtension(scoped_refptr<const extensions::Extension> extension, base::OnceClosure done_callback) override
    { std::move(done_callback).Run(); }
    void LoadExtensionForReload(const extensions::ExtensionId &extension_id,
                                const base::FilePath &path) override { }
    void LoadExtensionForReloadWithQuietFailure(const extensions::ExtensionId &extension_id,
                                                const base::FilePath &path) override { }
    void ShowExtensionDisabledError(const extensions::Extension *extension, bool is_remote_install) override { }
    bool CanEnableExtension(const extensions::Extension *extension) override { return true; }
    bool CanDisableExtension(const extensions::Extension *extension) override { return true; }
    void OnAddNewOrUpdatedExtension(const extensions::Extension *extension) override { }
    void UpdateExternalExtensionAlert() override {}
    void GrantActivePermissions(const extensions::Extension *extension) override { }

    raw_ptr<content::BrowserContext> m_browserContext;
    extensions::ExtensionRegistrar *m_extensionRegistrar;
    extensions::ExtensionRegistry *m_extensionRegistry;
    ExtensionManager *m_manager;
    base::WeakPtrFactory<ExtensionLoader> m_weakFactory{ this };
};
} // namespace QtWebEngineCore

#endif // EXTENSION_LOADER_H_
