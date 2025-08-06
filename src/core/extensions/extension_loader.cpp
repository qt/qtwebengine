// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extension_loader.h"

#include "extension_manager.h"
#include "type_conversion.h"

#include "base/files/file_util.h"
#include "base/task/sequenced_task_runner.h"
#include "content/public/browser/browser_context.h"
#include "extensions/browser/extension_file_task_runner.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/common/file_util.h"

using namespace extensions;

static constexpr int kSupportedManifestVersion = 3;

namespace QtWebEngineCore {
ExtensionLoader::ExtensionLoader(content::BrowserContext *context, ExtensionManager *manager)
    : m_browserContext(context)
    , m_extensionRegistrar(context, this)
    , m_extensionRegistry(ExtensionRegistry::Get(context))
    , m_manager(manager)
{
}

ExtensionLoader::~ExtensionLoader() { }

// static
ExtensionLoader::LoadingInfo ExtensionLoader::loadExtensionOnFileThread(const base::FilePath &path)
{
    ExtensionLoader::LoadingInfo result;
    result.path = path;

    if (!base::DirectoryExists(path)) {
        result.error = "Directory not exists: " + path.AsUTF8Unsafe();
        return result;
    }

    int loadFlags = Extension::NO_FLAGS;
    std::string error;
    scoped_refptr<Extension> extension =
            file_util::LoadExtension(path, mojom::ManifestLocation::kUnpacked, loadFlags, &error);
    if (!extension.get()) {
        result.error = error;
        return result;
    }

    if (extension->manifest_version() != kSupportedManifestVersion) {
        result.error = "Unsupported manifest version";
        return result;
    }

    result.extension = extension;
    return result;
}

void ExtensionLoader::loadExtension(const base::FilePath &path)
{
    if (m_browserContext->IsOffTheRecord()) {
        m_manager->onExtensionLoadError(toQt(path), "Can't load in off-the-record mode");
        return;
    }
    GetExtensionFileTaskRunner()->PostTaskAndReplyWithResult(
            FROM_HERE, base::BindOnce(&loadExtensionOnFileThread, path),
            base::BindOnce(&ExtensionLoader::loadExtensionFinished, m_weakFactory.GetWeakPtr()));
}

void ExtensionLoader::addExtension(scoped_refptr<const Extension> extension)
{
    if (extensions().Contains(extension->id()))
        m_extensionRegistrar.ReloadExtension(extension->id(),
                                             ExtensionRegistrar::LoadErrorBehavior::kQuiet);
    else
        m_extensionRegistry->AddDisabled(extension);
}

void ExtensionLoader::reloadExtension(const std::string &id)
{
    m_extensionRegistrar.ReloadExtension(id, ExtensionRegistrar::LoadErrorBehavior::kQuiet);
}

void ExtensionLoader::loadExtensionFinished(const LoadingInfo &loadingInfo)
{
    if (!loadingInfo.error.empty()) {
        m_manager->onExtensionLoadError(toQt(loadingInfo.path), loadingInfo.error);
        return;
    }

    scoped_refptr<const Extension> extension = loadingInfo.extension;
    Q_ASSERT(extension);

    addExtension(extension);
    m_manager->onExtensionLoaded(extension.get());
}

void ExtensionLoader::unloadExtension(const std::string &id)
{
    m_extensionRegistrar.RemoveExtension(id, UnloadedExtensionReason::UNINSTALL);
}

ExtensionSet ExtensionLoader::extensions() const
{
    return m_extensionRegistry->GenerateInstalledExtensionsSet();
}

void ExtensionLoader::disableExtension(const std::string &id)
{
    if (isExtensionLoaded(id) && isExtensionEnabled(id))
        m_extensionRegistrar.DisableExtension(id, extensions::disable_reason::DISABLE_USER_ACTION);
}

void ExtensionLoader::enableExtension(const std::string &id)
{
    if (isExtensionLoaded(id) && !isExtensionEnabled(id))
        m_extensionRegistrar.EnableExtension(id);
}

bool ExtensionLoader::isExtensionEnabled(const std::string &id)
{
    return m_extensionRegistry->enabled_extensions().Contains(id);
}

bool ExtensionLoader::isExtensionLoaded(const std::string &id)
{
    return extensions().Contains(id);
}

scoped_refptr<const Extension> ExtensionLoader::getExtensionById(const std::string &id)
{
    return isExtensionLoaded(id) ? extensions().GetByID(id) : nullptr;
}

} // namespace QtWebEngineCore
