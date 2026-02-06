// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickwebengineprofileprototype_p.h"
#include "qquickwebengineprofileprototype_p_p.h"
#include "profile_adapter.h"
#include "qquickwebengineprofile_p.h"
#include "profile_adapter.h"

#include <QQuickWebEngineProfile>
#include <QtQml/qqmlinfo.h>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebEngineProfilePrototype
    //! \nativetype QQuickWebEngineProfilePrototype
    \inqmlmodule QtWebEngine
    \since QtWebEngine 6.9
    \brief Creates an instance of \l{QQuickWebEngineProfile} class.

    WebEngineProfile contains settings, scripts, and the list of visited links shared by all
    views that belong to the profile. Some of the profile's properties have to be initialized
    in one call and should not be modified during profile lifetime. WebEngineProfilePrototype
    provides way to create a profile, when all the required properties are set.

 \code
    // creating OTR profile
    WebEngineProfilePrototype: {
        id: otrProfile
    }
    let otrProfile = otrProfile.instance();

    // creating non-OTR profile
    WebEngineProfilePrototype: {
        id: nonOtrProfile
        storageName: 'Test'
    }
    let profile = nonOtrProfile.instance();
 \endcode
*/

QQuickWebEngineProfilePrototype::QQuickWebEngineProfilePrototype(QObject *parent)
    : QObject(parent), d_ptr(new QQuickWebEngineProfilePrototypePrivate)
{
}

QQuickWebEngineProfilePrototype::~QQuickWebEngineProfilePrototype() { }

/*!
    \qmlproperty string WebEngineProfilePrototype::storageName

    The storage name that is used to create separate subdirectories for each profile that uses
    the disk for storing persistent data and cache. The storage name must be unique.

    \sa WebEngineProfilePrototype::persistentStoragePath, WebEngineProfilePrototype::cachePath
*/
QString QQuickWebEngineProfilePrototype::storageName() const
{
    return d_ptr->m_storageName;
}

void QQuickWebEngineProfilePrototype::setStorageName(const QString &storageName)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral(
                "storageName is a write-once property, and should not be set again.");
        return;
    }
    d_ptr->m_storageName = storageName;
}

/*!
    \qmlproperty string WebEngineProfilePrototype::persistentStoragePath

    The path to the location where the persistent data for the browser and web content are
    stored. Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, the storage is located below
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) in a directory named using
    storageName.
*/
QString QQuickWebEngineProfilePrototype::persistentStoragePath() const
{
    return d_ptr->m_persistentStoragePath;
}

void QQuickWebEngineProfilePrototype::setPersistentStoragePath(const QString &path)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral(
                "persistentStoragePath is a write-once property, and should not be set again.");
        return;
    }
    d_ptr->m_persistentStoragePath = path;
}

/*!
    \qmlproperty string WebEngineProfilePrototype::cachePath

    The path to the location where the profile's caches are stored, in particular the HTTP cache.

    By default, the caches are stored
    below QStandardPaths::writableLocation(QStandardPaths::CacheLocation) in a directory named using
    storageName.
*/
QString QQuickWebEngineProfilePrototype::cachePath() const
{
    return d_ptr->m_cachePath;
}

void QQuickWebEngineProfilePrototype::setCachePath(const QString &cachePath)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral(
                "cachePath is a write-once property, and should not be set again.");
        return;
    }
    d_ptr->m_cachePath = cachePath;
}

/*!
    \qmlproperty enumeration WebEngineProfilePrototype::httpCacheType

    This enumeration describes the type of the HTTP cache:

    \value  WebEngineProfile.MemoryHttpCache
            Uses an in-memory cache. This is the only setting possible if offTheRecord is set or
            no storageName is available, which is the default.
    \value  WebEngineProfile.DiskHttpCache
            Uses a disk cache. This is the default value for non off-the-record profile with
   storageName. \value  WebEngineProfile.NoCache Disables caching.
*/
QQuickWebEngineProfile::HttpCacheType QQuickWebEngineProfilePrototype::httpCacheType() const
{
    return d_ptr->m_httpCacheType;
}

void QQuickWebEngineProfilePrototype::setHttpCacheType(
        QQuickWebEngineProfile::HttpCacheType httpCacheType)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral(
                "httpCacheType is a write-once property, and should not be set again.");
        return;
    }
    d_ptr->m_httpCacheType = httpCacheType;
}

/*!
    \qmlproperty enumeration WebEngineProfilePrototype::persistentCookiesPolicy

    This enumeration describes the policy of cookie persistence:

    \value  WebEngineProfile.NoPersistentCookies
            Both session and persistent cookies are stored in memory. This is the only setting
            possible if offTheRecord is set or no storageName is available, which is the default.
    \value  WebEngineProfile.AllowPersistentCookies
            Cookies marked persistent are saved to and restored from disk, whereas session cookies
            are only stored to disk for crash recovery.
            This is the default value for non off-the-record profile with storageName.
    \value  WebEngineProfile.OnlyPersistentCookies
            Cookies marked persistent are saved to and restored from disk, whereas session cookies
            are never stored to the disk, even for crash recovery.
    \value  WebEngineProfile.ForcePersistentCookies
            Both session and persistent cookies are saved to and restored from disk.
*/
QQuickWebEngineProfile::PersistentCookiesPolicy
QQuickWebEngineProfilePrototype::persistentCookiesPolicy() const
{
    return d_ptr->m_persistentCookiesPolicy;
}

void QQuickWebEngineProfilePrototype::setPersistentCookiesPolicy(
        QQuickWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral(
                "persistentCookiesPolicy is a write-once property, and should not be set again.");
        return;
    }
    d_ptr->m_persistentCookiesPolicy = persistentCookiesPolicy;
}

/*!
    \qmlproperty int WebEngineProfilePrototype::httpCacheMaximumSize

    The maximum size of the HTTP cache. If \c 0, the size will be controlled automatically by
    QtWebEngine. The default value is \c 0.

    \sa WebEngineProfilePrototype::httpCacheType
 */
int QQuickWebEngineProfilePrototype::httpCacheMaximumSize() const
{
    return d_ptr->m_httpCacheMaxSize;
}

void QQuickWebEngineProfilePrototype::setHttpCacheMaximumSize(int maxSizeInBytes)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral(
                "httpCacheMaximumSize is a write-once property, and should not be set again.");
        return;
    }
    d_ptr->m_httpCacheMaxSize = maxSizeInBytes;
}

/*!
    \qmlproperty enumeration WebEngineProfilePrototype::persistentPermissionsPolicy

    This enumeration describes the policy for permission persistence:

    \value  WebEngineProfile.AskEveryTime
            The application will ask for permissions every time they're needed, regardless of
            whether they've been granted before or not. This is intended for backwards compatibility
            with existing applications, and otherwise not recommended.
    \value  WebEngineProfile.StoreInMemory
            A request will be made only the first time a permission is needed. Any subsequent
            requests will be automatically granted or denied, depending on the initial user choice.
            This carries over to all pages using the same QWebEngineProfile instance, until the
            application is shut down. This is the setting applied if \c off-the-record is set
            or no persistent data path is available.
    \value  WebEngineProfile.StoreOnDisk
            Works the same way as \c PersistentPermissionsInMemory, but the permissions are saved to
            and restored from disk. This is the default setting.
*/
QQuickWebEngineProfile::PersistentPermissionsPolicy
QQuickWebEngineProfilePrototype::persistentPermissionsPolicy() const
{
    return d_ptr->m_persistentPermissionsPolicy;
}

void QQuickWebEngineProfilePrototype::setPersistentPermissionsPolicy(
        QQuickWebEngineProfile::PersistentPermissionsPolicy persistentPermissionsPolicy)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral("persistentPermissionsPolicy is a write-once property, "
                                           "and should not be set again.");
        return;
    }
    d_ptr->m_persistentPermissionsPolicy = persistentPermissionsPolicy;
}

/*!
    \qmlproperty list<string> WebEngineProfilePrototype::additionalTrustedCertificateFiles

    A list of paths of additional trusted certificates in this profile's CA certificate database.

    The certificates are read when the profile is created; any invalid paths or certificate files
    are discarded. The property only holds the paths which were successfully loaded by this profile.
    This property expects the certificate files to be PEM-encoded.
*/
QStringList QQuickWebEngineProfilePrototype::additionalTrustedCertificateFiles() const
{
    return d_ptr->m_additionalTrustedCertificateFiles;
}

void QQuickWebEngineProfilePrototype::setAdditionalTrustedCertificateFiles(const QStringList &paths)
{
    if (d_ptr->m_isComponentComplete) {
        qmlWarning(this) << QStringLiteral("additionalTrustedCertificateFiles is a write-once "
                                           "property, and should not be set again.");
        return;
    }
    d_ptr->m_additionalTrustedCertificateFiles = paths;
}

/*!
   \internal
*/
void QQuickWebEngineProfilePrototype::componentComplete()
{
    auto buildLocationFromStandardPath = [](const QString &standardPath, const QString &name) {
        QString location = standardPath;
        if (location.isEmpty())
            location = QDir::homePath() % QLatin1String("/.") % QCoreApplication::applicationName();

        location.append(QLatin1String("/QtWebEngine/") % name);
        return location;
    };

    QString dataPath = d_ptr->m_persistentStoragePath;
    if (dataPath.isEmpty() && !d_ptr->m_storageName.isEmpty())
        dataPath = buildLocationFromStandardPath(
                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
                d_ptr->m_storageName);

    if (!dataPath.isEmpty()) {
        if (QtWebEngineCore::ProfileAdapter::profileExistOnPath(dataPath)) {
            qWarning("Unable to create new Profile, "
                     "as another profile is using the same data path");
            return;
        }
    }

    if (d_ptr->m_storageName.isEmpty()) {
        if (d_ptr->m_httpCacheType == QQuickWebEngineProfile::DiskHttpCache)
            d_ptr->m_httpCacheType = QQuickWebEngineProfile::MemoryHttpCache;

        if (d_ptr->m_persistentCookiesPolicy != QQuickWebEngineProfile::NoPersistentCookies)
            d_ptr->m_persistentCookiesPolicy = QQuickWebEngineProfile::NoPersistentCookies;
    }

    QList<QSslCertificate> additionalCertificates;
    for (const auto &certFileName : std::as_const(d_ptr->m_additionalTrustedCertificateFiles)) {
        QFile certFile(certFileName);
        if (certFile.open(QIODevice::ReadOnly)) {
            auto &&certs = QSslCertificate::fromDevice(&certFile, QSsl::Pem);
            if (certs.empty()) {
                qmlWarning(this) << certFileName
                                 << QStringLiteral(" does not contain any valid PEM SSL "
                                                   "certs. It will be skipped.");
            }
            for (auto &&cert : certs) {
                if (!cert.isNull()) {
                    additionalCertificates.emplace_back(std::move(cert));
                }
            }
        } else {
            qmlWarning(this) << certFileName
                             << QStringLiteral(" is not found. It will be skipped.");
        }
    }

    auto profileAdapter = new QtWebEngineCore::ProfileAdapter(
            d_ptr->m_storageName, d_ptr->m_persistentStoragePath, d_ptr->m_cachePath,
            QtWebEngineCore::ProfileAdapter::HttpCacheType(d_ptr->m_httpCacheType),
            QtWebEngineCore::ProfileAdapter::PersistentCookiesPolicy(
                    d_ptr->m_persistentCookiesPolicy),
            d_ptr->m_httpCacheMaxSize,
            QtWebEngineCore::ProfileAdapter::PersistentPermissionsPolicy(
                    d_ptr->m_persistentPermissionsPolicy)
#if QT_CONFIG(ssl)
                    ,
            additionalCertificates
#endif
    );

    d_ptr->profile.reset(new QQuickWebEngineProfile(
            new QQuickWebEngineProfilePrivate(profileAdapter), this->parent()));
    d_ptr->m_isComponentComplete = true;
}

/*!
    \qmlmethod WebEngineProfile WebEngineProfilePrototype::instance()

    Returns an instance of WebEngineProfile.

    \note This function returns a null object if \l persistentStoragePath
    is already in use by another profile.
*/
QQuickWebEngineProfile *QQuickWebEngineProfilePrototype::instance()
{
    return d_ptr->profile.get();
}

/*!
   \internal
*/
void QQuickWebEngineProfilePrototype::classBegin() { }

QT_END_NAMESPACE

#include "moc_qquickwebengineprofileprototype_p.cpp"
