// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineprofilebuilder.h"
#include "qwebengineprofile_p.h"
#include "qwebengineprofilebuilder_p.h"
#include "profile_adapter.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

/*!
    \class QWebEngineProfileBuilder
    \brief The QWebEngineProfileBuilder class provides a way to construct \l{QWebEngineProfile}.
    \since 6.9
    \inmodule QtWebEngineCore

    A \l{QWebEngineProfileBuilder} creates an instance of \l{QWebEngineProfile} class. Some profile's
    properties have to be initialized in one call and should not be modified during profile
    lifetime. The web engine profile builder simply guards that.
*/
QWebEngineProfileBuilder::QWebEngineProfileBuilder() : d_ptr(new QWebEngineProfileBuilderPrivate())
{
}

QWebEngineProfileBuilder::~QWebEngineProfileBuilder() = default;

/*!
    Creates an off-the-record profile with the parent object \a parent that leaves no
    record on the local machine and has no persistent data or cache. This will force
    cookies, persistent data and HTTP cache to be stored in memory
*/
QWebEngineProfile *QWebEngineProfileBuilder::createOffTheRecordProfile(QObject *parent)
{
    return new QWebEngineProfile(parent);
}

/*!
    Constructs a profile with the storage name \a storageName and parent \a parent.

    The storage name is used to give each disk-based profile, a separate subdirectory for
    persistent data and cache. The storage location must be unique during application life time.
    It is up to the user to prevent the creation of profiles with same storage's location, which can
    lead to corrupted browser cache.

    A disk-based \l{QWebEngineProfile} should be destroyed before the application exit, otherwise the
    cache and persistent data may not be fully flushed to disk.

    \note When creating a disk-based profile, if the data path is already in use by another
    profile, the function will return a null pointer.

    \sa QWebEngineProfile::storageName()
*/
QWebEngineProfile *QWebEngineProfileBuilder::createProfile(const QString &storageName,
                                                           QObject *parent) const
{
    auto buildLocationFromStandardPath = [](const QString &standardPath, const QString &name) {
        QString location;
        location += standardPath;
        if (location.isEmpty())
            location += QDir::homePath() % "/."_L1 % QCoreApplication::applicationName();

        location += "/QtWebEngine/"_L1 % name;
        return location;
    };

    QString dataPath = d_ptr->m_dataPath;
    if (dataPath.isEmpty() && !storageName.isEmpty())
        dataPath = buildLocationFromStandardPath(
                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), storageName);

    if (!dataPath.isEmpty()) {
        if (QtWebEngineCore::ProfileAdapter::profileExistOnPath(dataPath)) {
            qWarning("Unable to create new profile, "
                     "as another profile is using the same data path");
            return nullptr;
        }
    }

    return new QWebEngineProfile(
            new QWebEngineProfilePrivate(new QtWebEngineCore::ProfileAdapter(
                    storageName, d_ptr->m_dataPath, d_ptr->m_cachePath,
                    QtWebEngineCore::ProfileAdapter::HttpCacheType(d_ptr->m_httpCacheType),
                    QtWebEngineCore::ProfileAdapter::PersistentCookiesPolicy(
                            d_ptr->m_persistentCookiesPolicy),
                    d_ptr->m_httpCacheMaxSize,
                    QtWebEngineCore::ProfileAdapter::PersistentPermissionsPolicy(
                            d_ptr->m_persistentPermissionPolicy))),
            parent);
}

/*!
    Sets the path used to store persistent data for the browser and web content to \a path.
    Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, this is below QStandardPaths::DataLocation in a QtWebengine/StorageName specific
    subdirectory.

    \note Use QStandardPaths::writableLocation(QStandardPaths::DataLocation)
    to obtain the QStandardPaths::DataLocation path.

    \sa QWebEngineProfile::persistentStoragePath(), QStandardPaths::writableLocation()
*/
QWebEngineProfileBuilder &QWebEngineProfileBuilder::setPersistentStoragePath(const QString &path)
{
    d_ptr->m_dataPath = path;
    return *this;
}

/*!
    Sets the path used for the cache to \a path.

    By default, this is below StandardPaths::CacheLocation in a QtWebengine/StorageName specific
    subdirectory.

    \note Use QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    to obtain the QStandardPaths::CacheLocation path.

    \sa QWebEngineProfile::cachePath(), QStandardPaths::writableLocation()
*/
QWebEngineProfileBuilder &QWebEngineProfileBuilder::setCachePath(const QString &path)
{
    d_ptr->m_cachePath = path;
    return *this;
}

/*!
    Sets the HTTP cache type to \a httpCacheType.

    \sa QWebEngineProfile::httpCacheType(), setCachePath()
*/
QWebEngineProfileBuilder &QWebEngineProfileBuilder::setHttpCacheType(
        QWebEngineProfile::HttpCacheType httpCacheType)
{
    d_ptr->m_httpCacheType = httpCacheType;
    return *this;
}

/*!
    Sets the policy for persistent cookies to \a persistentCookiePolicy.

    \sa QWebEngineProfile::persistentCookiesPolicy()
*/
QWebEngineProfileBuilder &QWebEngineProfileBuilder::setPersistentCookiesPolicy(
        QWebEngineProfile::PersistentCookiesPolicy persistentCookiePolicy)
{
    d_ptr->m_persistentCookiesPolicy = persistentCookiePolicy;
    return *this;
}

/*!
    Sets the maximum size of the HTTP cache to \a maxSizeInBytes bytes.

    Setting it to \c 0 means the size will be controlled automatically by QtWebEngine.
    \sa QWebEngineProfile::httpCacheMaximumSize(), setHttpCacheType()
*/
QWebEngineProfileBuilder &QWebEngineProfileBuilder::setHttpCacheMaximumSize(int maxSizeInBytes)
{
    d_ptr->m_httpCacheMaxSize = maxSizeInBytes;
    return *this;
}

/*!
    Sets the policy for persistent permissions to \a persistentPermissionPolicy.

    \sa QWebEngineProfile::persistentPermissionsPolicy()
*/
QWebEngineProfileBuilder &QWebEngineProfileBuilder::setPersistentPermissionsPolicy(
        QWebEngineProfile::PersistentPermissionsPolicy persistentPermissionPolicy)
{
    d_ptr->m_persistentPermissionPolicy = persistentPermissionPolicy;
    return *this;
}
