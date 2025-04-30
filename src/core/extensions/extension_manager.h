// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef EXTENSION_MANAGER_H_
#define EXTENSION_MANAGER_H_

#include <memory>

#include "api/qwebengineextensioninfo.h"
#include "api/qwebengineextensioninfo_p.h"

#include <QList>
#include <QString>
#include <QObject>
#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

namespace content {
class BrowserContext;
}

namespace extensions {
class Extension;
}

QT_BEGIN_NAMESPACE
class QWebEngineExtensionManager;
QT_END_NAMESPACE

namespace QtWebEngineCore {
class ExtensionActionManager;
class ExtensionLoader;
class ExtensionInstaller;

class Q_WEBENGINECORE_EXPORT ExtensionManager
{
public:
    Q_DECLARE_PUBLIC(QWebEngineExtensionManager)
    QWebEngineExtensionManager *q_ptr;

    ExtensionManager(content::BrowserContext *context);
    ~ExtensionManager();

    void loadExtension(const QString &path);
    void installExtension(const QString &path);
    void setExtensionEnabled(const std::string &id, bool enabled);
    void unloadExtension(const std::string &id);
    void uninstallExtension(const std::string &id);
    void reloadExtension(const std::string &id);

    bool isExtensionEnabled(const std::string &id) const;
    bool isExtensionLoaded(const std::string &id) const;
    bool isExtensionInstalled(const std::string &id) const;
    QUrl actionPopupUrl(const std::string &id) const;
    QString installDirectory() const;
    QList<QWebEngineExtensionInfo> extensions();

    void onExtensionLoaded(const extensions::Extension *);
    void onExtensionInstalled(const extensions::Extension *);
    void onExtensionUninstalled(const std::string &id);
    void onExtensionLoadError(const QString &path, const std::string &error);
    void onExtensionInstallError(const QString &path, const std::string &error);
    void onExtensionUninstallError(const std::string &id, const std::string &error);

private:
    std::unique_ptr<ExtensionLoader> m_loader;
    std::unique_ptr<ExtensionInstaller> m_installer;
    std::unique_ptr<ExtensionActionManager> m_actionManager;
};
} // namespace QtWebEngineCore

#endif // EXTENSION_MANAGER_H_
