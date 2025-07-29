// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extension_manager.h"

#include <QDirListing>
#include <QUrl>

#include "api/qwebengineextensioninfo.h"
#include "api/qwebengineextensioninfo_p.h"
#include "api/qwebengineextensionmanager.h"
#include "extension_action_manager.h"
#include "extension_loader.h"
#include "extension_installer.h"
#include "type_conversion.h"

#include "base/functional/callback.h"
#include "content/public/browser/browser_context.h"
#include "extensions/browser/extension_file_task_runner.h"

using namespace extensions;

namespace QtWebEngineCore {
namespace {
QWebEngineExtensionInfoPrivate *
createWebEngineExtensionData(ExtensionManager *manager, scoped_refptr<const Extension> extension,
                             const std::string &error = {})
{
    QWebEngineExtensionInfoPrivate::ExtensionData data{
        .id = extension->id(),
        .name = toQt(extension->name()),
        .description = toQt(extension->description()),
        .path = toQt(extension->path()),
        .error = toQt(error),
        .actionPopupUrl = manager->actionPopupUrl(extension->id())
    };
    return new QWebEngineExtensionInfoPrivate(data, manager);
}

QWebEngineExtensionInfoPrivate *createWebEngineExtensionData(ExtensionManager *manager,
                                                             const QString &path,
                                                             const std::string &error)
{
    QWebEngineExtensionInfoPrivate::ExtensionData data{
        .path = path,
        .error = toQt(error)
    };
    return new QWebEngineExtensionInfoPrivate(data, manager);
}
} // namespace

ExtensionManager::ExtensionManager(content::BrowserContext *context)
    : m_loader(new ExtensionLoader(context, this))
    , m_installer(new ExtensionInstaller(context, this))
    , m_actionManager(new ExtensionActionManager())
{
    for (auto dir : QDirListing(installDirectory(), QDirListing::IteratorFlag::DirsOnly)) {
        loadExtension(dir.filePath());
    }
}

ExtensionManager::~ExtensionManager() { }

void ExtensionManager::loadExtension(const QString &path)
{
    m_loader->loadExtension(toFilePath(path));
}

void ExtensionManager::installExtension(const QString &path)
{
    m_installer->installExtension(toFilePath(path));
}

void ExtensionManager::setExtensionEnabled(const std::string &id, bool enabled)
{
    if (enabled)
        m_loader->enableExtension(id);
    else
        m_loader->disableExtension(id);
}

void ExtensionManager::unloadExtension(const std::string &id)
{
    if (!isExtensionLoaded(id))
        return;

    scoped_refptr<const Extension> extension = m_loader->getExtensionById(id);
    m_actionManager->removeExtensionAction(extension->id());
    m_loader->unloadExtension(extension->id());
    Q_Q(QWebEngineExtensionManager);
    Q_EMIT q->unloadFinished(
            QWebEngineExtensionInfo(createWebEngineExtensionData(this, extension)));
}

void ExtensionManager::uninstallExtension(const std::string &id)
{
    scoped_refptr<const Extension> extension = m_loader->getExtensionById(id);
    if (extension->path().DirName() == m_installer->installDirectory()) {
        m_installer->uninstallExtension(extension);
    } else {
        Q_Q(QWebEngineExtensionManager);
        Q_EMIT q->uninstallFinished(QWebEngineExtensionInfo(
                createWebEngineExtensionData(this, extension, "This extension was not installed")));
    }
}

void ExtensionManager::reloadExtension(const std::string &id)
{
    m_loader->reloadExtension(id);
}

bool ExtensionManager::isExtensionEnabled(const std::string &id) const
{
    return m_loader->isExtensionEnabled(id);
}

bool ExtensionManager::isExtensionLoaded(const std::string &id) const
{
    return m_loader->isExtensionLoaded(id);
}

bool ExtensionManager::isExtensionInstalled(const std::string &id) const
{
    scoped_refptr<const Extension> extension = m_loader->getExtensionById(id);
    return QFileInfo(toQt(extension->path())).dir() == QDir(installDirectory());
}

QUrl ExtensionManager::actionPopupUrl(const std::string &id) const
{
    scoped_refptr<const Extension> extension = m_loader->getExtensionById(id);
    if (auto *extensionAction = m_actionManager->getExtensionAction(extension.get()))
        return toQt(extensionAction->GetPopupUrl(-1));
    return QUrl();
}

QString ExtensionManager::installDirectory() const
{
    return toQt(m_installer->installDirectory());
}

QList<QWebEngineExtensionInfo> ExtensionManager::extensions()
{
    QList<QWebEngineExtensionInfo> extension_list;
    for (auto extension : m_loader->extensions())
        extension_list.append(
                QWebEngineExtensionInfo(createWebEngineExtensionData(this, extension)));
    return extension_list;
}

void ExtensionManager::onExtensionLoaded(const Extension *extension)
{
    Q_Q(QWebEngineExtensionManager);
    Q_EMIT q->loadFinished(
            QWebEngineExtensionInfo(createWebEngineExtensionData(this, extension)));
}

void ExtensionManager::onExtensionLoadError(const QString &path, const std::string &error)
{
    Q_Q(QWebEngineExtensionManager);
    Q_EMIT q->loadFinished(
            QWebEngineExtensionInfo(createWebEngineExtensionData(this, path, error)));
}

void ExtensionManager::onExtensionInstalled(const Extension *extension)
{
    m_loader->addExtension(extension);
    Q_Q(QWebEngineExtensionManager);
    Q_EMIT q->installFinished(
            QWebEngineExtensionInfo(createWebEngineExtensionData(this, extension)));
}

void ExtensionManager::onExtensionUninstalled(const std::string &id)
{
    scoped_refptr<const Extension> extension = m_loader->getExtensionById(id);
    m_actionManager->removeExtensionAction(extension->id());
    m_loader->unloadExtension(extension->id());

    Q_Q(QWebEngineExtensionManager);
    Q_EMIT q->uninstallFinished(
            QWebEngineExtensionInfo(createWebEngineExtensionData(this, extension)));
}

void ExtensionManager::onExtensionInstallError(const QString &path, const std::string &error)
{
    Q_Q(QWebEngineExtensionManager);
    Q_EMIT q->installFinished(
            QWebEngineExtensionInfo(createWebEngineExtensionData(this, path, error)));
}

void ExtensionManager::onExtensionUninstallError(const std::string &id, const std::string &error)
{
    scoped_refptr<const Extension> extension = m_loader->getExtensionById(id);
    Q_Q(QWebEngineExtensionManager);
    Q_EMIT q->installFinished(
            QWebEngineExtensionInfo(createWebEngineExtensionData(this, extension, error)));
}

} // namespace QtWebEngineCore
