// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebengineextensionmanager.h"

#if QT_CONFIG(webengine_extensions)
#include "qwebengineextensioninfo_p.h"
#include "extensions/extension_manager.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineExtensionManager
    \brief The QWebEngineExtensionManager class allows applications to access Chrome/Firefox
     extension-compatible interfaces through managed packages.

    \since 6.10
    \inmodule QtWebEngineCore

    QWebEngineExtensionManager exposes interfaces compatible with the Chrome/Firefox extension platform,
    letting applications load or install packages built against that format at runtime.

    Loading via \l loadExtension() is temporary and doesn't persist across sessions, while
    \l installExtension() stores the package in the profile directory so it loads automatically at startup.

    Loaded packages can be inspected through \l extensions() property, which returns a list of
    \l QWebEngineExtensionInfo objects, and the manager emits signals when packages are loaded, installed,
    unloaded, or uninstalled. The \l installPath property indicates where installed packages are persisted.

    Only the ManifestV3 specification is supported — packages using other manifest versions are rejected outright.
    This capability is not available for off-the-record profiles. Every profile ships with two built-in components,
    Google Hangouts and the Chromium PDF Viewer, which can be disabled if not needed.

    \sa QWebEngineProfile::extensionManager, QWebEngineExtensionInfo
*/

QWebEngineExtensionManager::QWebEngineExtensionManager(QtWebEngineCore::ExtensionManager *d)
    : d_ptr(d)
{
    d->q_ptr = this;
}

QWebEngineExtensionManager::~QWebEngineExtensionManager() { }

/*!
    Loads an unpacked extension from \a path.

    The \l loadFinished() signal is emitted when an extension
    is loaded or the load failed. If the load succeeded \l QWebEngineExtensionInfo::isLoaded() will
    return \c true, otherwise \l QWebEngineExtensionInfo::error() will contain information where the
    loading process failed.

    Extensions are always loaded in disabled state, users have to enable them manually.
    Loading an already loaded extension from the same path will reload the extension.

    \sa QWebEngineExtensionInfo::isLoaded(), QWebEngineExtensionInfo::error()
*/
void QWebEngineExtensionManager::loadExtension(const QString &path)
{
    d_ptr->loadExtension(path);
}

/*!
    Installs an extension from \a path to the profile's directory and loads it.

    The \l installFinished() signal is emitted after an
    extension is installed or the install failed. If the install succeeded \l
    QWebEngineExtensionInfo::isInstalled() will return \c true, otherwise \l
    QWebEngineExtensionInfo::error() will contain information how the install process failed.

    Extensions are loaded in disabled state after the install succeeded.
    Installed extensions are automatically loaded at every startup in disabled state.
    The install path can be queried with \l installPath().

    The installer is capable of installing zipped or unpacked extensions.
    The \a path parameter should point to a directory or a zip file containing the extension's
    manifest file. If the manifest is missing from the top level directory, the install process will
    abort.

    Installing an already loaded or installed extension from the same path will install a new
    extension.

    \sa QWebEngineExtensionInfo::isInstalled(), QWebEngineExtensionInfo::error(), installPath()
*/
void QWebEngineExtensionManager::installExtension(const QString &path)
{
    d_ptr->installExtension(path);
}

/*!
    Unloads the \a extension

    Removes all the extension's data from memory.

    The \l unloadFinished() signal is emitted after the unload process finished.

    \note It is also possible to unload internal extensions like Hangouts and PDF,
    but they will be loaded at next startup like other installed extensions.

    \sa QWebEngineExtensionInfo::isLoaded()
*/
void QWebEngineExtensionManager::unloadExtension(const QWebEngineExtensionInfo &extension)
{
    if (extension.d_ptr)
        d_ptr->unloadExtension(extension.d_ptr->id());
}

/*!
    Uninstalls the \a extension.

    Removes the extension's files from the install path and unloads the extension.
    The \l uninstallFinished() signal is emitted after the process finished.

    \sa QWebEngineExtensionManager::installPath(), QWebEngineExtensionInfo::isInstalled(),
    QWebEngineExtensionInfo::error()
*/
void QWebEngineExtensionManager::uninstallExtension(const QWebEngineExtensionInfo &extension)
{
    if (extension.d_ptr)
        d_ptr->uninstallExtension(extension.d_ptr->id());
}

/*!
    Allows to turn on/off the \a extension at runtime.

    The \a enabled argument determines whether the extension should be enabled or disabled.
    \note It is also possible to disable internal extensions like Hangouts and PDF.

    \sa QWebEngineExtensionInfo::isEnabled()
*/
void QWebEngineExtensionManager::setExtensionEnabled(const QWebEngineExtensionInfo &extension,
                                                     bool enabled)
{
    if (extension.d_ptr)
        d_ptr->setExtensionEnabled(extension.d_ptr->id(), enabled);
}

/*!
    \fn void QWebEngineExtensionManager::loadFinished(const QWebEngineExtensionInfo &extension)
    \fn void QWebEngineExtensionManager::installFinished(const QWebEngineExtensionInfo &extension)
    \fn void QWebEngineExtensionManager::unloadFinished(const QWebEngineExtensionInfo &extension)
    \fn void QWebEngineExtensionManager::uninstallFinished(const QWebEngineExtensionInfo &extension)

    Signals that are emitted when \a extension is loaded, unloaded, installed,
    or uninstalled.
*/

/*!
    \property QWebEngineExtensionManager::installPath
    \brief Returns the directory's path where the extensions are installed.

    \sa installExtension(), QWebEngineExtensionInfo::isInstalled()
*/

QString QWebEngineExtensionManager::installPath() const
{
    return d_ptr->installDirectory();
}

/*!
    \property QWebEngineExtensionManager::extensions
    \brief Returns a list of the loaded extensions.

    \sa QWebEngineExtensionInfo
*/
QList<QWebEngineExtensionInfo> QWebEngineExtensionManager::extensions() const
{
    return d_ptr->extensions();
}

/*!
    \reimp
*/
bool QWebEngineExtensionManager::event(QEvent *e)
{
    return QObject::event(e);
}

QT_END_NAMESPACE
#endif // QT_CONFIG(webengine_extensions)
