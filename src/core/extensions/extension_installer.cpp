// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "extension_installer.h"

#include "extension_manager.h"
#include "type_conversion.h"

#include "base/files/file_util.h"
#include "content/public/browser/browser_context.h"
#include "extensions/browser/extension_file_task_runner.h"
#include "extensions/browser/zipfile_installer.h"
#include "extensions/common/constants.h"
#include "unpacked_extension_installer.h"

using namespace extensions;

namespace QtWebEngineCore {
namespace {
bool uninstallExtensionOnFileThread(const base::FilePath &dirToDelete,
                                    const base::FilePath &profileDir,
                                    const base::FilePath &extensionInstallDir)
{
    // The below conditions are asserting that we should only be deleting
    // directories that are inside the `extensionInstallDir` which should be
    // inside the profile directory. Anything outside of that would be considered
    // invalid and dangerous since this is effectively an `rm -rf
    // <extension_delete_path>`.

    if (!base::DirectoryExists(dirToDelete)) {
        return false;
    }

    // Confirm that all the directories involved are not empty and are absolute so
    // that the subsequent comparisons have some value.
    if (profileDir.empty() || extensionInstallDir.empty() || dirToDelete.empty()
        || !profileDir.IsAbsolute() || !extensionInstallDir.IsAbsolute()
        || !dirToDelete.IsAbsolute()) {
        return false;
    }

    // Confirm the directory where we install extensions is a direct subdir of the
    // profile dir.
    if (extensionInstallDir.DirName() != profileDir)
        return false;

    // Confirm the directory we are obliterating is a direct subdir of the
    // extensions install directory.
    if (dirToDelete.DirName() != extensionInstallDir)
        return false;

    // In POSIX environment and if `dirToDelete` is a symbolic link, this deletes only
    // the symlink. (even if the symlink points to a non-existent file)
    return base::DeletePathRecursively(dirToDelete);
}

bool cleanupBrokenInstallOnFileThread(const base::FilePath &dirToDelete,
                                      const base::FilePath &profileDir,
                                      const base::FilePath &extensionInstallDir)
{
    if (base::DirectoryExists(dirToDelete))
        return uninstallExtensionOnFileThread(dirToDelete, profileDir, extensionInstallDir);
    return true;
}

ExtensionInstaller::ExtensionFormat getExtensionFormatOnFileThread(const base::FilePath &path)
{
    if (!base::PathExists(path))
        return ExtensionInstaller::ExtensionFormat::Invalid;
    if (path.MatchesExtension(FILE_PATH_LITERAL(".zip")))
        return ExtensionInstaller::ExtensionFormat::Zip;
    if (base::DirectoryExists(path))
        return ExtensionInstaller::ExtensionFormat::Unpacked;
    return ExtensionInstaller::ExtensionFormat::Invalid;
}
} // namespace

ExtensionInstaller::ExtensionInstaller(content::BrowserContext *context, ExtensionManager *manager)
    : m_browserContext(context), m_manager(manager)
{
}

void ExtensionInstaller::installExtension(const base::FilePath &path)
{
    if (m_browserContext->IsOffTheRecord()) {
        m_manager->onExtensionInstallError(toQt(path), "Cannot install in off-the-record mode");
        return;
    }

    GetExtensionFileTaskRunner()->PostTaskAndReplyWithResult(
            FROM_HERE, base::BindOnce(&getExtensionFormatOnFileThread, path),
            base::BindOnce(&ExtensionInstaller::installExtensionInternal,
                           m_weakFactory.GetWeakPtr(), path));
}

void ExtensionInstaller::installExtensionInternal(const base::FilePath &path,
                                                  ExtensionFormat format)
{
    switch (format) {
    case ExtensionFormat::Zip:
        ZipFileInstaller::Create(
                GetExtensionFileTaskRunner(),
                base::BindOnce(&ExtensionInstaller::installDone, m_weakFactory.GetWeakPtr()))
                ->InstallZipFileToUnpackedExtensionsDir(path, installDirectory());
        break;
    case ExtensionFormat::Unpacked:
        UnpackedExtensionInstaller::Create(
                GetExtensionFileTaskRunner(),
                base::BindOnce(&ExtensionInstaller::installDone, m_weakFactory.GetWeakPtr()))
                ->install(path, installDirectory());
        break;
    case ExtensionFormat::Invalid:
    default:
        m_manager->onExtensionInstallError(toQt(path), "Invalid file format");
    }
}

void ExtensionInstaller::installDone(const base::FilePath &source, const base::FilePath &installDir,
                                     const std::string &error)
{
    if (!error.empty()) {
        cleanupBrokenInstall(installDir, error);
        m_manager->onExtensionInstallError(toQt(source), error);
        return;
    }

    GetExtensionFileTaskRunner()->PostTaskAndReplyWithResult(
            FROM_HERE, base::BindOnce(&ExtensionLoader::loadExtensionOnFileThread, installDir),
            base::BindOnce(&ExtensionInstaller::loadFinished, m_weakFactory.GetWeakPtr(), source));
}

void ExtensionInstaller::loadFinished(const base::FilePath &source,
                                      const ExtensionLoader::LoadingInfo &loadingInfo)
{
    auto error = loadingInfo.error;
    if (!error.empty()) {
        auto install_path = loadingInfo.path;
        cleanupBrokenInstall(install_path, error);
        m_manager->onExtensionInstallError(toQt(source), error);
        return;
    }

    auto extension = loadingInfo.extension;
    m_manager->onExtensionInstalled(extension.get());
}

void ExtensionInstaller::uninstallExtension(scoped_refptr<const Extension> extension)
{
    GetExtensionFileTaskRunner()->PostTaskAndReplyWithResult(
            FROM_HERE,
            base::BindOnce(&uninstallExtensionOnFileThread, extension->path(),
                           m_browserContext->GetPath(), installDirectory()),
            base::BindOnce(&ExtensionInstaller::uninstallFinished, m_weakFactory.GetWeakPtr(),
                           extension->id()));
}

void ExtensionInstaller::uninstallFinished(const std::string &id, bool success)
{
    if (!success) {
        m_manager->onExtensionUninstallError(id, "Invalid install directory");
        return;
    }

    m_manager->onExtensionUninstalled(id);
}

base::FilePath ExtensionInstaller::installDirectory() const
{
    return m_browserContext->GetPath().AppendASCII(extensions::kInstallDirectoryName);
}

void ExtensionInstaller::cleanupBrokenInstall(const base::FilePath &dirToDelete,
                                              const std::string &error)
{
    GetExtensionFileTaskRunner()->PostTaskAndReplyWithResult(
            FROM_HERE,
            base::BindOnce(&cleanupBrokenInstallOnFileThread, dirToDelete,
                           m_browserContext->GetPath(), installDirectory()),
            base::BindOnce(&ExtensionInstaller::onInstallFailure, m_weakFactory.GetWeakPtr(),
                           dirToDelete, error));
}

void ExtensionInstaller::onInstallFailure(const base::FilePath &brokenInstallDir,
                                          const std::string &error, bool cleanupSucceeded)
{

    if (!cleanupSucceeded)
        qWarning("Failed to clean up broken extension install in %ls",
                 qUtf16Printable(toQt(brokenInstallDir)));
}

} // namespace QtWebEngineCore
