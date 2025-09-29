// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef EXTENSION_INSTALLER_H_
#define EXTENSION_INSTALLER_H_

#include "extension_loader.h"

#include "api/qwebengineextensioninfo.h"

#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"

namespace content {
class BrowserContext;
}

namespace QtWebEngineCore {
class ExtensionManager;
}

namespace QtWebEngineCore {
class ExtensionInstaller
{
public:
    ExtensionInstaller(content::BrowserContext *context, ExtensionManager *manager);
    ~ExtensionInstaller() { }

    enum class ExtensionFormat {
        Invalid,
        Zip,
        Unpacked,
    };

    void installExtension(const base::FilePath &path);
    void uninstallExtension(scoped_refptr<const extensions::Extension> extension);

    base::FilePath installDirectory() const;

private:
    void installExtensionInternal(const base::FilePath &path, ExtensionFormat format);
    void installDone(const base::FilePath &source, const base::FilePath &installDir,
                     const std::string &error);
    void loadFinished(const base::FilePath &source,
                      const ExtensionLoader::LoadingInfo &loadingInfo);
    bool uninstallInternal(const base::FilePath &dirToDelete);
    void uninstallFinished(const std::string &id, bool success);
    void cleanupBrokenInstall(const base::FilePath &dirToDelete, const std::string &error);
    void onInstallFailure(const base::FilePath &brokenInstallDir, const std::string &error,
                          bool cleanupSucceeded);

    raw_ptr<content::BrowserContext> m_browserContext;
    ExtensionManager *m_manager;
    base::WeakPtrFactory<ExtensionInstaller> m_weakFactory{ this };
};
} // namespace QtWebEngineCore

#endif // EXTENSION_INSTALLER_H_
