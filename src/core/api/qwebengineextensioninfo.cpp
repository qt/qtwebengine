// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineextensioninfo.h"
#include "qwebengineextensioninfo_p.h"

#if QT_CONFIG(webengine_extensions)
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>

#include "extensions/extension_manager.h"

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE
QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QWebEngineExtensionInfoPrivate)

/*!
    \class QWebEngineExtensionInfo
    \brief The QWebEngineExtensionInfo provides information about a browser extension.

    \since 6.10
    \inmodule QtWebEngineCore

    QWebEngineExtensionInfo provides information of an extension loaded into \QWE.
    Extensions can be loaded via the \l QWebEngineExtensionManager.
    You can check if the extension was successfully loaded using its \l isLoaded() property.
    The \l error() property contains error messages if the loading process failed.
    Extensions are always loaded in disabled state. To run an extension, it has to be enabled via
    \l QWebEngineExtensionManager::setExtensionEnabled().

    An extension can be removed using \l QWebEngineExtensionManager::unloadExtension().

    You can access extensions with \l QWebEngineExtensionManager::extensions() which provides a list
    of the loaded extensions, or connect to the manager's signals to be notified if the loading
    process is complete.

    \sa QWebEngineExtensionManager, QWebEngineProfile::extensionManager()
*/

QWebEngineExtensionInfoPrivate::QWebEngineExtensionInfoPrivate(
        const ExtensionData &data, QtWebEngineCore::ExtensionManager *manager)
    : QSharedData(), m_data(data), m_manager(manager)
{
}

QWebEngineExtensionInfoPrivate::~QWebEngineExtensionInfoPrivate() = default;

QString QWebEngineExtensionInfoPrivate::name() const
{
    return m_data.name;
}

std::string QWebEngineExtensionInfoPrivate::id() const
{
    return m_data.id;
}

QString QWebEngineExtensionInfoPrivate::description() const
{
    return m_data.description;
}

QString QWebEngineExtensionInfoPrivate::path() const
{
    return m_data.path;
}

QString QWebEngineExtensionInfoPrivate::error() const
{
    return m_data.error;
}

QUrl QWebEngineExtensionInfoPrivate::actionPopupUrl() const
{
    return m_data.actionPopupUrl;
}

bool QWebEngineExtensionInfoPrivate::isEnabled() const
{
    return m_manager->isExtensionEnabled(id());
}

bool QWebEngineExtensionInfoPrivate::isLoaded() const
{
    return m_manager->isExtensionLoaded(id());
}

bool QWebEngineExtensionInfoPrivate::isInstalled() const
{
    return QFileInfo(m_data.path).dir() == QDir(m_manager->installDirectory());
}

QWebEngineExtensionInfo::QWebEngineExtensionInfo() : d_ptr(nullptr) { }

QWebEngineExtensionInfo::QWebEngineExtensionInfo(QWebEngineExtensionInfoPrivate *d) : d_ptr(d) { }

QWebEngineExtensionInfo::QWebEngineExtensionInfo(const QWebEngineExtensionInfo &other) noexcept =
        default;
QWebEngineExtensionInfo::QWebEngineExtensionInfo(QWebEngineExtensionInfo &&other) noexcept =
        default;
QWebEngineExtensionInfo &
QWebEngineExtensionInfo::operator=(const QWebEngineExtensionInfo &other) noexcept = default;

QWebEngineExtensionInfo::~QWebEngineExtensionInfo() = default;

/*!
    \property QWebEngineExtensionInfo::name
    \brief The name of the extension.

    Acquired from the extension's manifest file's name property.

    Empty if the load failed.
*/
QString QWebEngineExtensionInfo::name() const
{
    return d_ptr ? d_ptr->name() : ""_L1;
}

/*!
    \property QWebEngineExtensionInfo::id
    \brief The id of the extension.

    Generated at load time. Multiple QWebEngineExtensionInfo objects with the same id
    represent the same underlying extension.

    The id is generated from the filesystem path where the extension was loaded from
    and the extensions manfiest file. Loading the same extension from the same path
    always have the same id.

    Empty if the load failed.
*/
QString QWebEngineExtensionInfo::id() const
{
    return d_ptr ? QString::fromStdString(d_ptr->id()) : ""_L1;
}

/*!
    \property QWebEngineExtensionInfo::name
    \brief The description of the extension.

    Acquired from the extension's manifest file's description property.

    Empty if the load failed.
*/
QString QWebEngineExtensionInfo::description() const
{
    return d_ptr ? d_ptr->description() : ""_L1;
}

/*!
    \property QWebEngineExtensionInfo::path
    \brief The install path of the extension.

    The filesystem path where the extension was loaded from.
*/
QString QWebEngineExtensionInfo::path() const
{
    return d_ptr ? d_ptr->path() : ""_L1;
}

/*!
    \property QWebEngineExtensionInfo::error
    \brief Errors happened during loading, installing or uninstalling the extension.

    Multiple errors can happen during load time, like missing manifest, invalid file format
    or path. The loading process stops at the first error.

    Empty if the load succeeded.
*/
QString QWebEngineExtensionInfo::error() const
{
    return d_ptr ? d_ptr->error() : ""_L1;
}

/*!
    \property QWebEngineExtensionInfo::actionPopupUrl
    \brief Returns the url of the extension's popup.

    Extension developers usually provide a popup menu where users can control
    their extension. The menu can be accessed via this url.

    Empty if the load failed.
*/
QUrl QWebEngineExtensionInfo::actionPopupUrl() const
{
    return d_ptr ? d_ptr->actionPopupUrl() : QUrl(""_L1);
}

/*!
    \property QWebEngineExtensionInfo::isEnabled
    \brief This property holds whether the extension is enabled.

    \sa QWebEngineExtensionManager::setExtensionEnabled()
*/
bool QWebEngineExtensionInfo::isEnabled() const
{
    return d_ptr && d_ptr->isEnabled();
}

/*!
    \property QWebEngineExtensionInfo::isLoaded
    \brief This property holds whether the extension is loaded.

    If the extension was loaded or installed successfully this property returns \c true.
    Returns false if the extension was unloaded, uninstalled or the loading process failed.

    \sa QWebEngineExtensionManager::loadExtension(), QWebEngineExtensionManager::unloadExtension()
*/

bool QWebEngineExtensionInfo::isLoaded() const
{
    return d_ptr && d_ptr->isLoaded();
}

/*
    \property QWebEngineExtensionInfo::isInstalled
    \brief This property holds whether the extension is installed in the profile's install
   directory.

    \sa QWebEngineExtensionManager::installDirectory(),
    QWebEngineExtensionManager::installExtension(), QWebEngineExtensionManager::uninstallExtension()
*/
bool QWebEngineExtensionInfo::isInstalled() const
{
    return d_ptr && d_ptr->isInstalled();
}

QT_END_NAMESPACE

#endif // QT_CONFIG(webengine_extensions)
