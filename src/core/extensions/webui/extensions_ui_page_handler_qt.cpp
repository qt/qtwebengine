// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "extensions_ui_page_handler_qt.h"

#include "extensions/extension_system_qt.h"
#include "extensions/webui/select_file_dialog.h"

#include "extensions/browser/extension_registry.h"
#include "extensions/extension_manager.h"
#include "extensions/extension_loader.h"
#include "api/qwebengineextensionmanager.h"
#include "profile_adapter.h"
#include "profile_qt.h"
#include "type_conversion.h"

ExtensionsUIPageHandlerQt::ExtensionsUIPageHandlerQt(
        content::WebUI *webui, Profile *profile,
        mojo::PendingReceiver<qtwebengine::mojom::PageHandler> receiver,
        mojo::PendingRemote<qtwebengine::mojom::Page> page)
    : receiver_(this, std::move(receiver)), page_(std::move(page)), webui_(webui), profile_(profile)
{
}

ExtensionsUIPageHandlerQt::~ExtensionsUIPageHandlerQt() { }

void ExtensionsUIPageHandlerQt::GetAllExtensionInfo(GetAllExtensionInfoCallback callback)
{
    std::vector<qtwebengine::mojom::ExtensionInfoPtr> extensionsInfo;
    auto add_to_list = [&extensionsInfo](QList<QWebEngineExtensionInfo> extensions) {
        for (auto extension : extensions) {
            auto info = qtwebengine::mojom::ExtensionInfo::New();
            info->name = extension.name().toStdString();
            info->description = extension.description().toStdString();
            info->id = extension.id().toStdString();
            info->isEnabled = extension.isEnabled();
            info->isInstalled = extension.isInstalled();
            info->isLoaded = extension.isLoaded();
            extensionsInfo.push_back(std::move(info));
        }
    };

    auto profileAdapter = static_cast<QtWebEngineCore::ProfileQt *>(profile_)->profileAdapter();
    QWebEngineExtensionManager *manager = profileAdapter->extensionManager();
    add_to_list(manager->extensions());
    std::move(callback).Run(std::move(extensionsInfo));
}

void ExtensionsUIPageHandlerQt::LoadExtension()
{
    SelectFileDialog::Show(
            base::BindOnce(&ExtensionsUIPageHandlerQt::InnerLoadExtension, base::Unretained(this)),
            base::FilePath(), webui_->GetWebContents());
}

void ExtensionsUIPageHandlerQt::InstallExtension()
{
    SelectFileDialog::Show(base::BindOnce(&ExtensionsUIPageHandlerQt::InnerInstallExtension,
                                          base::Unretained(this)),
                           base::FilePath(), webui_->GetWebContents());
}

void ExtensionsUIPageHandlerQt::InnerLoadExtension(const base::FilePath &path)
{
    auto profileAdapter = static_cast<QtWebEngineCore::ProfileQt *>(profile_)->profileAdapter();
    QWebEngineExtensionManager *manager = profileAdapter->extensionManager();

    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(manager, &QWebEngineExtensionManager::loadFinished,
                                   [this, connection]() {
                                       page_->ReloadPage();
                                       QObject::disconnect(*connection);
                                       delete connection;
                                   });
    manager->loadExtension(QtWebEngineCore::toQt(path));
}

void ExtensionsUIPageHandlerQt::InnerInstallExtension(const base::FilePath &path)
{
    auto profileAdapter = static_cast<QtWebEngineCore::ProfileQt *>(profile_)->profileAdapter();
    QWebEngineExtensionManager *manager = profileAdapter->extensionManager();

    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(manager, &QWebEngineExtensionManager::installFinished,
                                   [this, connection]() {
                                       page_->ReloadPage();
                                       QObject::disconnect(*connection);
                                       delete connection;
                                   });
    manager->installExtension(QtWebEngineCore::toQt(path));
}

void ExtensionsUIPageHandlerQt::UninstallExtension(const std::string &id,
                                                   UninstallExtensionCallback callback)
{
    auto profileAdapter = static_cast<QtWebEngineCore::ProfileQt *>(profile_)->profileAdapter();
    QWebEngineExtensionManager *manager = profileAdapter->extensionManager();

    QWebEngineExtensionInfo extensionInfo;
    if (!FindExtensionById(id, extensionInfo)) {
        std::move(callback).Run("Unable to find extension with Id " + id);
        return;
    }

    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(manager, &QWebEngineExtensionManager::uninstallFinished,
                                   [connection, cb = std::move(callback)](
                                           const QWebEngineExtensionInfo &extension) mutable {
                                       std::move(cb).Run(extension.error().toStdString());
                                       QObject::disconnect(*connection);
                                       delete connection;
                                   });

    manager->uninstallExtension(extensionInfo);
}

void ExtensionsUIPageHandlerQt::UnloadExtension(const std::string &id,
                                                UnloadExtensionCallback callback)
{
    auto profileAdapter = static_cast<QtWebEngineCore::ProfileQt *>(profile_)->profileAdapter();
    QWebEngineExtensionManager *manager = profileAdapter->extensionManager();

    QWebEngineExtensionInfo extensionInfo;
    if (!FindExtensionById(id, extensionInfo)) {
        std::move(callback).Run("Unable to find extension with Id " + id);
        return;
    }

    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(manager, &QWebEngineExtensionManager::unloadFinished,
                                   [connection, cb = std::move(callback)](
                                           const QWebEngineExtensionInfo &extension) mutable {
                                       std::move(cb).Run(extension.error().toStdString());
                                       QObject::disconnect(*connection);
                                       delete connection;
                                   });

    manager->unloadExtension(extensionInfo);
}

void ExtensionsUIPageHandlerQt::SetExtensionEnabled(const std::string &id, bool isEnabled,
                                                    SetExtensionEnabledCallback callback)
{
    static_cast<extensions::ExtensionSystemQt *>(extensions::ExtensionSystem::Get(profile_))
            ->extensionManager()
            ->setExtensionEnabled(id, isEnabled);
    std::move(callback).Run();
}

bool ExtensionsUIPageHandlerQt::FindExtensionById(const std::string &id,
                                                  QWebEngineExtensionInfo &extensionInfo)
{
    auto profileAdapter = static_cast<QtWebEngineCore::ProfileQt *>(profile_)->profileAdapter();
    QWebEngineExtensionManager *manager = profileAdapter->extensionManager();

    QList<QWebEngineExtensionInfo> extensions = manager->extensions();
    for (auto extension : extensions) {
        if (extension.id() == id) {
            extensionInfo = extension;
            return true;
        }
    }
    return false;
}
