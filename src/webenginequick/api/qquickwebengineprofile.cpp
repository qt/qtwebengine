// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebengineprofile.h"
#include "qquickwebengineprofile_p.h"
#include "qquickwebenginedownloadrequest.h"
#include "qquickwebenginesettings_p.h"
#include "qquickwebenginescriptcollection_p.h"
#include "qquickwebenginescriptcollection_p_p.h"
#include "qquickwebengineview_p_p.h"

#include "profile_adapter.h"
#include "web_engine_settings.h"

#include <QtWebEngineCore/qwebenginescriptcollection.h>
#include <QtWebEngineCore/private/qwebenginescriptcollection_p.h>
#include <QtWebEngineCore/qwebengineclienthints.h>
#include <QtWebEngineCore/qwebenginecookiestore.h>
#include <QtWebEngineCore/qwebenginenotification.h>
#include <QtWebEngineCore/private/qwebenginedownloadrequest_p.h>
#include <QtWebEngineCore/qwebengineurlscheme.h>
#include <QtWebEngineCore/private/qwebenginepermission_p.h>

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>

using QtWebEngineCore::ProfileAdapter;

QT_BEGIN_NAMESPACE

/*!
    \class QQuickWebEngineProfile
    \brief The QQuickWebEngineProfile class provides a web engine profile shared by multiple pages.
    \since 5.6

    \inmodule QtWebEngineQuick

    A web engine profile contains settings, scripts, persistent cookie policy, and the list of
    visited links shared by all web engine pages that belong to the profile.

    Information about visited links is stored together with persistent cookies
    and other persistent data in a storage determined by the storageName
    property. Persistent data is stored in a subdirectory determined by the
    persistentStoragePath property and the cache in a subdirectory determined by
    the cachePath property. The httpCacheType property describes the type of the
    cache: \e in-memory or \e on-disk. If only the storageName property is set,
    the other values are generated automatically based on it. If you specify
    any of the values manually, you should do it before creating any pages that
    belong to the profile.

    Profiles can be used to isolate pages from each other. A typical use case is a dedicated
    \e {off-the-record profile} for a \e {private browsing} mode. An off-the-record profile forces
    cookies, the HTTP cache, and other normally persistent data to be stored only in memory. The
    offTheRecord property holds whether a profile is off-the-record.

    The default profile can be accessed by defaultProfile(). It is a built-in profile that all
    web pages not specifically created with another profile belong to.

    A WebEngineProfile instance can be created and accessed from C++ through the
    QQuickWebEngineProfile class, which exposes further functionality in C++. This allows Qt Quick
    applications to intercept URL requests (QQuickWebEngineProfile::setRequestInterceptor), or
    register custom URL schemes (QQuickWebEngineProfile::installUrlSchemeHandler).

    Spellchecking HTML form fields can be enabled per profile by setting the \l spellCheckEnabled
    property and the current languages used for spellchecking can be set by using the
    \l spellCheckLanguages property.
*/

/*!
    \enum QQuickWebEngineProfile::HttpCacheType

    This enum describes the HTTP cache type:

    \value MemoryHttpCache Use an in-memory cache. This is the default if
    \c off-the-record is set.
    \value DiskHttpCache Use a disk cache. This is the default if \c off-the-record
    is not set. Falls back to \c MemoryHttpCache if \c off-the-record is set.
    \value NoCache Disable both in-memory and disk caching. (Added in Qt 5.7)
*/

/*!
    \enum QQuickWebEngineProfile::PersistentCookiesPolicy

    This enum describes policy for cookie persistency:

    \value  NoPersistentCookies
            Both session and persistent cookies are stored in memory. This is the only setting
            possible if \c off-the-record is set or no persistent data path is available.
    \value  AllowPersistentCookies
            Cookies marked persistent are saved to and restored from disk, whereas session cookies
            are only stored to disk for crash recovery. This is the default setting.
    \value  ForcePersistentCookies
            Both session and persistent cookies are saved to and restored from disk.
*/

/*!
    \enum QQuickWebEngineProfile::PersistentPermissionsPolicy

    \since 6.8

    This enum describes the policy for permission persistence:

    \value  AskEveryTime
            The application will ask for permissions every time they're needed, regardless of
            whether they've been granted before or not. This is intended for backwards compatibility
            with existing applications, and otherwise not recommended.
    \value  StoreInMemory
            A request will be made only the first time a permission is needed. Any subsequent
            requests will be automatically granted or denied, depending on the initial user choice.
            This carries over to all pages that use the same QQuickWebEngineProfile instance, until the
            application is shut down. This is the setting applied if \c off-the-record is set
            or no persistent data path is available.
    \value  StoreOnDisk
            Works the same way as \c PersistentPermissionsInMemory, but the permissions are saved to
            and restored from disk. This is the default setting.
*/

/*!
  \fn QQuickWebEngineProfile::downloadRequested(QQuickWebEngineDownloadRequest *download)

  This signal is emitted whenever a download has been triggered.
  The \a download argument holds the state of the download.
  The download has to be explicitly accepted with
  \c{QWebEngineDownloadRequest::accept()} or it will be
  cancelled by default.
  The download item is parented by the profile. If it is not accepted, it
  will be deleted immediately after the signal emission.
  This signal cannot be used with a queued connection.

  \note To use from C++ static_cast \a download to QWebEngineDownloadRequest
*/

/*!
  \fn QQuickWebEngineProfile::downloadFinished(QQuickWebEngineDownloadRequest *download)

  This signal is emitted whenever downloading stops, because it finished successfully, was
  cancelled, or was interrupted (for example, because connectivity was lost).
  The \a download argument holds the state of the finished download instance.

  \note To use from C++ static_cast \a download to QWebEngineDownloadRequest
*/

/*!
    \fn QQuickWebEngineProfile::presentNotification(QWebEngineNotification *notification)

    This signal is emitted whenever there is a newly created user notification.
    The \a notification argument holds the \l {QWebEngineNotification} instance
    to query data and interact with.

    \sa WebEngineProfile::presentNotification
*/

/*!
    \fn QQuickWebEngineProfile::clearHttpCacheCompleted()
    \since 6.7

    This signal is emitted when the clearHttpCache() operation is completed.

    \sa clearHttpCache()
*/

QQuickWebEngineProfilePrivate::QQuickWebEngineProfilePrivate(ProfileAdapter *profileAdapter)
    : m_settings(new QQuickWebEngineSettings())
    , m_clientHints(new QWebEngineClientHints(profileAdapter))
    , m_profileAdapter(profileAdapter)
{
    profileAdapter->addClient(this);
    // Fullscreen API was implemented before the supported setting, so we must
    // make it default true to avoid change in default API behavior.
    m_settings->d_ptr->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
}

QQuickWebEngineProfilePrivate::~QQuickWebEngineProfilePrivate()
{
    if (m_profileAdapter) {
        // In the case the user sets this profile as the parent of the interceptor
        // it can be deleted before the browser-context still referencing it is.
        m_profileAdapter->setRequestInterceptor(nullptr);
        m_profileAdapter->removeClient(this);
    }

    if (m_profileAdapter != QtWebEngineCore::ProfileAdapter::defaultProfileAdapter())
        delete m_profileAdapter;
    else if (m_profileAdapter)
        m_profileAdapter->releaseAllWebContentsAdapterClients();
}

void QQuickWebEngineProfilePrivate::addWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient *adapter)
{
    Q_ASSERT(m_profileAdapter);
    m_profileAdapter->addWebContentsAdapterClient(adapter);
}

void QQuickWebEngineProfilePrivate::removeWebContentsAdapterClient(QtWebEngineCore::WebContentsAdapterClient*adapter)
{
    Q_ASSERT(m_profileAdapter);
    m_profileAdapter->removeWebContentsAdapterClient(adapter);
}

QtWebEngineCore::ProfileAdapter *QQuickWebEngineProfilePrivate::profileAdapter() const
{
    return m_profileAdapter;
}

QQuickWebEngineSettings *QQuickWebEngineProfilePrivate::settings() const
{
    return m_settings.data();
}

QtWebEngineCore::WebEngineSettings *QQuickWebEngineProfilePrivate::coreSettings() const
{
    return QtWebEngineCore::WebEngineSettings::get(m_settings->d_ptr.data());
}

void QQuickWebEngineProfilePrivate::cancelDownload(quint32 downloadId)
{
    if (m_profileAdapter)
        m_profileAdapter->cancelDownload(downloadId);
}

void QQuickWebEngineProfilePrivate::downloadDestroyed(quint32 downloadId)
{
    m_ongoingDownloads.remove(downloadId);
    if (m_profileAdapter)
        m_profileAdapter->removeDownload(downloadId);
}

void QQuickWebEngineProfilePrivate::cleanDownloads()
{
    for (auto download : m_ongoingDownloads.values()) {
        if (!download)
            continue;

        if (!download->isFinished())
            download->cancel();

        if (m_profileAdapter)
            m_profileAdapter->removeDownload(download->id());
    }
    m_ongoingDownloads.clear();
}

void QQuickWebEngineProfilePrivate::downloadRequested(DownloadItemInfo &info)
{
    Q_Q(QQuickWebEngineProfile);

    Q_ASSERT(!m_ongoingDownloads.contains(info.id));
    QWebEngineDownloadRequestPrivate *itemPrivate =
            new QWebEngineDownloadRequestPrivate(m_profileAdapter);
    itemPrivate->downloadId = info.id;
    itemPrivate->downloadState = info.accepted ? QWebEngineDownloadRequest::DownloadInProgress
                                               : QWebEngineDownloadRequest::DownloadRequested;
    itemPrivate->startTime = info.startTime;
    itemPrivate->downloadUrl = info.url;
    itemPrivate->totalBytes = info.totalBytes;
    itemPrivate->mimeType = info.mimeType;
    itemPrivate->downloadDirectory = QFileInfo(info.path).path();
    itemPrivate->downloadFileName = QFileInfo(info.path).fileName();
    itemPrivate->suggestedFileName = info.suggestedFileName;
    itemPrivate->savePageFormat = static_cast<QWebEngineDownloadRequest::SavePageFormat>(
                info.savePageFormat);
    itemPrivate->isSavePageDownload = info.isSavePageDownload;
    if (info.page && info.page->clientType() == QtWebEngineCore::WebContentsAdapterClient::QmlClient)
        itemPrivate->adapterClient = info.page;
    else
        itemPrivate->adapterClient = nullptr;

    QQuickWebEngineDownloadRequest *download = new QQuickWebEngineDownloadRequest(itemPrivate, q);

    m_ongoingDownloads.insert(info.id, download);
    QObject::connect(download, &QObject::destroyed, q, [id = info.id, this] () { downloadDestroyed(id); });

    QQmlEngine::setObjectOwnership(download, QQmlEngine::JavaScriptOwnership);
    Q_EMIT q->downloadRequested(download);

    QWebEngineDownloadRequest::DownloadState state = download->state();
    info.path = QDir(download->downloadDirectory()).filePath(download->downloadFileName());
    info.savePageFormat = itemPrivate->savePageFormat;
    info.accepted = state != QWebEngineDownloadRequest::DownloadCancelled
                      && state != QWebEngineDownloadRequest::DownloadRequested;

    if (state == QWebEngineDownloadRequest::DownloadRequested) {
        // Delete unaccepted downloads.
        info.accepted = false;
        delete download;
    }
}

void QQuickWebEngineProfilePrivate::downloadUpdated(const DownloadItemInfo &info)
{
    if (!m_ongoingDownloads.contains(info.id))
        return;

    Q_Q(QQuickWebEngineProfile);

    QQuickWebEngineDownloadRequest* download = m_ongoingDownloads.value(info.id).data();

    if (!download) {
        downloadDestroyed(info.id);
        return;
    }

    download->d_ptr->update(info);

    if (info.state != ProfileAdapterClient::DownloadInProgress) {
        Q_EMIT q->downloadFinished(download);
    }
}

void QQuickWebEngineProfilePrivate::showNotification(QSharedPointer<QtWebEngineCore::UserNotificationController> &controller)
{
    Q_Q(QQuickWebEngineProfile);
    auto notification = new QWebEngineNotification(controller);
    QQmlEngine::setObjectOwnership(notification, QQmlEngine::JavaScriptOwnership);
    Q_EMIT q->presentNotification(notification);
}

void QQuickWebEngineProfilePrivate::clearHttpCacheCompleted()
{
    Q_Q(QQuickWebEngineProfile);
    Q_EMIT q->clearHttpCacheCompleted();
}

QQuickWebEngineScriptCollection *QQuickWebEngineProfilePrivate::getUserScripts()
{
    Q_Q(QQuickWebEngineProfile);
    if (!m_scriptCollection)
        m_scriptCollection.reset(
            new QQuickWebEngineScriptCollection(
                new QQuickWebEngineScriptCollectionPrivate(
                    new QWebEngineScriptCollectionPrivate(
                        m_profileAdapter->userResourceController()))));

    if (!m_scriptCollection->qmlEngine())
        m_scriptCollection->setQmlEngine(qmlEngine(q));

    return m_scriptCollection.data();
}
/*!
    \qmltype WebEngineProfile
    \nativetype QQuickWebEngineProfile
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.1
    \brief Contains settings, scripts, and visited links common to multiple web engine views.

    WebEngineProfile contains settings, scripts, and the list of visited links shared by all
    views that belong to the profile.

    Information about visited links is stored together with persistent cookies
    and other persistent data in a storage determined by the storageName
    property. Persistent data is stored in a subdirectory determined by the
    persistentStoragePath property and the cache in a subdirectory determined by
    the cachePath property. The httpCacheType property describes the type of the
    cache: \e in-memory or \e on-disk. If only the storageName property is set,
    the other values are generated automatically based on it. If you specify
    any of the values manually, you should do it before creating any pages that
    belong to the profile.

    Profiles can be used to isolate pages from each other. A typical use case is
    a dedicated \e {off-the-record profile} for a \e {private browsing} mode. An
    off-the-record profile forces cookies, the HTTP cache, and other normally
    persistent data to be stored only in memory. The offTheRecord property holds
    whether a profile is off-the-record.

    Each web engine view has an associated profile. Views that do not have a specific profile set
    share a common one, which is off-the-record by default.
*/

/*!
    \qmlsignal WebEngineProfile::downloadRequested(WebEngineDownloadRequest download)

    This signal is emitted whenever a download has been triggered.
    The \a download argument holds the state of the download.
    The download has to be explicitly accepted with WebEngineDownloadRequest::accept() or the
    download will be cancelled by default.
*/

/*!
    \qmlsignal WebEngineProfile::downloadFinished(WebEngineDownloadRequest download)

    This signal is emitted whenever downloading stops, because it finished successfully, was
    cancelled, or was interrupted (for example, because connectivity was lost).
    The \a download argument holds the state of the finished download instance.
*/

/*!
    \qmlsignal WebEngineProfile::presentNotification(WebEngineNotification notification)
    \since QtWebEngine 1.9

    This signal is emitted whenever there is a newly created user notification.
    The \a notification argument holds the \l {WebEngineNotification} instance
    to query data and interact with.
*/

/*!
    \qmlsignal WebEngineProfile::clearHttpCacheCompleted()
    \since QtWebEngine 6.7

    This signal is emitted when the clearHttpCache() operation is completed.

    \sa clearHttpCache()
*/

/*!
    Constructs a new off-the-record profile with the parent \a parent.

    An off-the-record profile leaves no record on the local machine, and has no
    persistent data or cache. Thus, the HTTP cache can only be in memory and the
    cookies can only be non-persistent. Trying to change these settings will
    have no effect.
*/
QQuickWebEngineProfile::QQuickWebEngineProfile(QObject *parent)
    : QObject(parent),
      d_ptr(new QQuickWebEngineProfilePrivate(new QtWebEngineCore::ProfileAdapter()))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineProfile::QQuickWebEngineProfile(QQuickWebEngineProfilePrivate *privatePtr, QObject *parent)
    : QObject(parent)
    , d_ptr(privatePtr)
{
    d_ptr->q_ptr = this;
}

/*!
   \internal
*/
QQuickWebEngineProfile::~QQuickWebEngineProfile()
{
    d_ptr->cleanDownloads();
}

/*!
    \qmlproperty string WebEngineProfile::storageName

    The storage name that is used to create separate subdirectories for each profile that uses
    the disk for storing persistent data and cache.

    \sa WebEngineProfile::persistentStoragePath, WebEngineProfile::cachePath
*/

/*!
    \property QQuickWebEngineProfile::storageName

    The storage name that is used to create separate subdirectories for each profile that uses
    the disk for storing persistent data and cache.

    \sa persistentStoragePath, cachePath
*/

QString QQuickWebEngineProfile::storageName() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->storageName();
}

void QQuickWebEngineProfile::setStorageName(const QString &name)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->storageName() == name)
        return;
    ProfileAdapter::HttpCacheType oldCacheType = d->profileAdapter()->httpCacheType();
    ProfileAdapter::PersistentCookiesPolicy oldCookiePolicy = d->profileAdapter()->persistentCookiesPolicy();
    ProfileAdapter::PersistentPermissionsPolicy oldPermissionsPolicy = d->profileAdapter()->persistentPermissionsPolicy();
    d->profileAdapter()->setStorageName(name);
    emit storageNameChanged();
    emit persistentStoragePathChanged();
    emit cachePathChanged();
    if (d->profileAdapter()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
    if (d->profileAdapter()->persistentCookiesPolicy() != oldCookiePolicy)
        emit persistentCookiesPolicyChanged();
    if (d->profileAdapter()->persistentPermissionsPolicy() != oldPermissionsPolicy)
        emit persistentPermissionsPolicyChanged();
}

/*!
    \qmlproperty bool WebEngineProfile::offTheRecord

    Whether the web engine profile is \e off-the-record.
    An off-the-record profile forces cookies, the HTTP cache, and other normally persistent data
    to be stored only in memory. Profile is off-the-record by default.

    Changing a profile from \e off-the-record to disk-based behavior also requires setting a
    proper storageName.

    \sa storageName
*/


/*!
    \property QQuickWebEngineProfile::offTheRecord

    Whether the web engine profile is \e off-the-record.
    An off-the-record profile forces cookies, the HTTP cache, and other normally persistent data
    to be stored only in memory. Profile is off-the-record by default.

    Changing a profile from \e off-the-record to disk-based behavior also requires setting a
    proper storageName.

    \sa setStorageName()
*/

bool QQuickWebEngineProfile::isOffTheRecord() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->isOffTheRecord();
}

void QQuickWebEngineProfile::setOffTheRecord(bool offTheRecord)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->isOffTheRecord() == offTheRecord)
        return;

    if (!offTheRecord && d->profileAdapter()->storageName().isEmpty()) {
        qWarning("Storage name is empty. Cannot change profile from off-the-record "
                 "to disk-based behavior as it requires a proper storage name");
        return;
    }

    ProfileAdapter::HttpCacheType oldCacheType = d->profileAdapter()->httpCacheType();
    ProfileAdapter::PersistentCookiesPolicy oldCookiePolicy = d->profileAdapter()->persistentCookiesPolicy();
    ProfileAdapter::PersistentPermissionsPolicy oldPermissionsPolicy = d->profileAdapter()->persistentPermissionsPolicy();
    d->profileAdapter()->setOffTheRecord(offTheRecord);
    emit offTheRecordChanged();
    if (d->profileAdapter()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
    if (d->profileAdapter()->persistentCookiesPolicy() != oldCookiePolicy)
        emit persistentCookiesPolicyChanged();
    if (d->profileAdapter()->persistentPermissionsPolicy() != oldPermissionsPolicy)
        emit persistentPermissionsPolicyChanged();
}

/*!
    \qmlproperty string WebEngineProfile::persistentStoragePath

    The path to the location where the persistent data for the browser and web content are
    stored. Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, the storage is located below
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) in a directory named using
    storageName.
*/

/*!
    \property QQuickWebEngineProfile::persistentStoragePath

    The path to the location where the persistent data for the browser and web content are
    stored. Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, the storage is located below
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) in a directory named using
    storageName.
*/

QString QQuickWebEngineProfile::persistentStoragePath() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->dataPath();
}

void QQuickWebEngineProfile::setPersistentStoragePath(const QString &path)
{
    Q_D(QQuickWebEngineProfile);
    if (persistentStoragePath() == path)
        return;
    d->profileAdapter()->setDataPath(path);
    emit persistentStoragePathChanged();
}

/*!
    \qmlproperty string WebEngineProfile::cachePath

    The path to the location where the profile's caches are stored, in particular the HTTP cache.

    By default, the caches are stored
    below QStandardPaths::writableLocation(QStandardPaths::CacheLocation) in a directory named using
    storageName.
*/

/*!
    \property QQuickWebEngineProfile::cachePath

    The path to the location where the profile's caches are stored, in particular the HTTP cache.

    By default, the caches are stored
    below QStandardPaths::writableLocation(QStandardPaths::CacheLocation) in a directory named using
    storageName.
*/

QString QQuickWebEngineProfile::cachePath() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->cachePath();
}

void QQuickWebEngineProfile::setCachePath(const QString &path)
{
    Q_D(QQuickWebEngineProfile);
    if (cachePath() == path)
        return;
    d->profileAdapter()->setCachePath(path);
    emit cachePathChanged();
}

/*!
    \qmlproperty string WebEngineProfile::httpUserAgent

    The user-agent string sent with HTTP to identify the browser.

    \note On Windows 8.1 and newer, the default user agent will always report
    "Windows NT 6.2" (Windows 8), unless the application does contain a manifest
    that declares newer Windows versions as supported.
*/

/*!
    \property QQuickWebEngineProfile::httpUserAgent

    The user-agent string sent with HTTP to identify the browser.
*/

QString QQuickWebEngineProfile::httpUserAgent() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->httpUserAgent();
}

void QQuickWebEngineProfile::setHttpUserAgent(const QString &userAgent)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->httpUserAgent() == userAgent)
        return;
    d->profileAdapter()->setHttpUserAgent(userAgent);
    emit httpUserAgentChanged();
}


/*!
    \qmlproperty enumeration WebEngineProfile::httpCacheType

    This enumeration describes the type of the HTTP cache:

    \value  WebEngineProfile.MemoryHttpCache
            Uses an in-memory cache. This is the only setting possible if offTheRecord is set or
            no storageName is available, which is the default.
    \value  WebEngineProfile.DiskHttpCache
            Uses a disk cache. This is the default value for non off-the-record profile with storageName.
    \value  WebEngineProfile.NoCache
            Disables caching. (Added in 5.7)
*/

/*!
    \property QQuickWebEngineProfile::httpCacheType

    This enumeration describes the type of the HTTP cache.

    If the profile is off-the-record or has no storageName set, MemoryHttpCache is returned.
*/

QQuickWebEngineProfile::HttpCacheType QQuickWebEngineProfile::httpCacheType() const
{
    const Q_D(QQuickWebEngineProfile);
    return QQuickWebEngineProfile::HttpCacheType(d->profileAdapter()->httpCacheType());
}

void QQuickWebEngineProfile::setHttpCacheType(QQuickWebEngineProfile::HttpCacheType httpCacheType)
{
    Q_D(QQuickWebEngineProfile);
    ProfileAdapter::HttpCacheType oldCacheType = d->profileAdapter()->httpCacheType();
    d->profileAdapter()->setHttpCacheType(ProfileAdapter::HttpCacheType(httpCacheType));
    if (d->profileAdapter()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
}

/*!
    \qmlproperty enumeration WebEngineProfile::persistentCookiesPolicy

    This enumeration describes the policy of cookie persistence:

    \value  WebEngineProfile.NoPersistentCookies
            Both session and persistent cookies are stored in memory. This is the only setting
            possible if offTheRecord is set or no storageName is available, which is the default.
    \value  WebEngineProfile.AllowPersistentCookies
            Cookies marked persistent are saved to and restored from disk, whereas session cookies
            are only stored to disk for crash recovery.
            This is the default value for non off-the-record profile with storageName.
    \value WebEngineProfile.ForcePersistentCookies
            Both session and persistent cookies are saved to and restored from disk.
*/

/*!
    \property QQuickWebEngineProfile::persistentCookiesPolicy

    This enumeration describes the policy of cookie persistency.
    If the profile is off-the-record, NoPersistentCookies is returned.
*/

QQuickWebEngineProfile::PersistentCookiesPolicy QQuickWebEngineProfile::persistentCookiesPolicy() const
{
    const Q_D(QQuickWebEngineProfile);
    return QQuickWebEngineProfile::PersistentCookiesPolicy(d->profileAdapter()->persistentCookiesPolicy());
}

void QQuickWebEngineProfile::setPersistentCookiesPolicy(QQuickWebEngineProfile::PersistentCookiesPolicy newPersistentCookiesPolicy)
{
    Q_D(QQuickWebEngineProfile);
    ProfileAdapter::PersistentCookiesPolicy oldPolicy = d->profileAdapter()->persistentCookiesPolicy();
    d->profileAdapter()->setPersistentCookiesPolicy(ProfileAdapter::PersistentCookiesPolicy(newPersistentCookiesPolicy));
    if (d->profileAdapter()->persistentCookiesPolicy() != oldPolicy)
        emit persistentCookiesPolicyChanged();
}

/*!
    \qmlproperty enumeration WebEngineProfile::persistentPermissionsPolicy

    \since 6.8

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

/*!
    \property QQuickWebEngineProfile::persistentPermissionsPolicy
    \since 6.8

    Describes the policy of permission persistence.
    If the profile is off-the-record, NoPersistentCookies is returned.
*/

QQuickWebEngineProfile::PersistentPermissionsPolicy QQuickWebEngineProfile::persistentPermissionsPolicy() const
{
    Q_D(const QQuickWebEngineProfile);
    return QQuickWebEngineProfile::PersistentPermissionsPolicy(d->profileAdapter()->persistentPermissionsPolicy());
}

void QQuickWebEngineProfile::setPersistentPermissionsPolicy(QQuickWebEngineProfile::PersistentPermissionsPolicy newPersistentPermissionsPolicy)
{
    Q_D(QQuickWebEngineProfile);
    ProfileAdapter::PersistentPermissionsPolicy oldPolicy = d->profileAdapter()->persistentPermissionsPolicy();
    d->profileAdapter()->setPersistentPermissionsPolicy(ProfileAdapter::PersistentPermissionsPolicy(newPersistentPermissionsPolicy));
    if (d->profileAdapter()->persistentPermissionsPolicy() != oldPolicy)
        emit persistentPermissionsPolicyChanged();
}

/*!
    \qmlproperty int WebEngineProfile::httpCacheMaximumSize

    The maximum size of the HTTP cache. If \c 0, the size will be controlled automatically by
    QtWebEngine. The default value is \c 0.

    \sa httpCacheType
*/

/*!
    \property QQuickWebEngineProfile::httpCacheMaximumSize

    The maximum size of the HTTP cache. If \c 0, the size will be controlled automatically by
    QtWebEngine. The default value is \c 0.

    \sa httpCacheType
*/

int QQuickWebEngineProfile::httpCacheMaximumSize() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->httpCacheMaxSize();
}

void QQuickWebEngineProfile::setHttpCacheMaximumSize(int maximumSize)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->httpCacheMaxSize() == maximumSize)
        return;
    d->profileAdapter()->setHttpCacheMaxSize(maximumSize);
    emit httpCacheMaximumSizeChanged();
}

/*!
    \qmlproperty string WebEngineProfile::httpAcceptLanguage

    The value of the Accept-Language HTTP request-header field.

    \since QtWebEngine 1.2
*/

/*!
    \property QQuickWebEngineProfile::httpAcceptLanguage

    The value of the Accept-Language HTTP request-header field.
*/

QString QQuickWebEngineProfile::httpAcceptLanguage() const
{
    Q_D(const QQuickWebEngineProfile);
    return d->profileAdapter()->httpAcceptLanguage();
}

void QQuickWebEngineProfile::setHttpAcceptLanguage(const QString &httpAcceptLanguage)
{
    Q_D(QQuickWebEngineProfile);
    if (d->profileAdapter()->httpAcceptLanguage() == httpAcceptLanguage)
        return;
    d->profileAdapter()->setHttpAcceptLanguage(httpAcceptLanguage);
    emit httpAcceptLanguageChanged();
}

/*!
    Returns the default profile.

    The default profile is off-the-record.

    \sa storageName()
*/
QQuickWebEngineProfile *QQuickWebEngineProfile::defaultProfile()
{
    static QQuickWebEngineProfile *profile = new QQuickWebEngineProfile(
                new QQuickWebEngineProfilePrivate(ProfileAdapter::createDefaultProfileAdapter()),
                ProfileAdapter::globalQObjectRoot());
    return profile;
}

/*!
    \property QQuickWebEngineProfile::spellCheckLanguages
    \brief The languages used by the spell checker.

    \since QtWebEngine 1.4
*/

/*!
    \qmlproperty list<string> WebEngineProfile::spellCheckLanguages

    This property holds the list of languages used by the spell checker.
    Each language should match the name of the \c .bdic dictionary.
    For example, the language \c en-US will load the \c en-US.bdic
    dictionary file.

    See the \l {Spellchecker}{Spellchecker feature documentation} for how
    dictionary files are searched.

    For more information about how to compile \c .bdic dictionaries, see the
    \l{WebEngine Widgets Spellchecker Example}{Spellchecker Example}.

    \since QtWebEngine 1.4
*/
void QQuickWebEngineProfile::setSpellCheckLanguages(const QStringList &languages)
{
    Q_D(QQuickWebEngineProfile);
    if (languages != d->profileAdapter()->spellCheckLanguages()) {
        d->profileAdapter()->setSpellCheckLanguages(languages);
        emit spellCheckLanguagesChanged();
    }
}

/*!
    \since 5.8

    Returns the list of languages used by the spell checker.
*/
QStringList QQuickWebEngineProfile::spellCheckLanguages() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->spellCheckLanguages();
}

/*!
    \property QQuickWebEngineProfile::spellCheckEnabled
    \brief whether the web engine spell checker is enabled.

    \since QtWebEngine 1.4
*/

/*!
    \qmlproperty bool WebEngineProfile::spellCheckEnabled

    This property holds whether the web engine spell checker is enabled.

    \since QtWebEngine 1.4
*/
void QQuickWebEngineProfile::setSpellCheckEnabled(bool enable)
{
     Q_D(QQuickWebEngineProfile);
    if (enable != isSpellCheckEnabled()) {
        d->profileAdapter()->setSpellCheckEnabled(enable);
        emit spellCheckEnabledChanged();
    }
}

bool QQuickWebEngineProfile::isSpellCheckEnabled() const
{
     const Q_D(QQuickWebEngineProfile);
     return d->profileAdapter()->isSpellCheckEnabled();
}

/*!
    \qmlproperty string WebEngineProfile::downloadPath
    \since  QtWebEngine 1.9

    The path to the location where the downloaded files are stored.

    Overrides the default path used for download location.

    If set to an empty string, the default path is restored.

    \note By default, the download path is QStandardPaths::DownloadLocation.
*/

/*!
    \property QQuickWebEngineProfile::downloadPath
    \since QtWebEngine 1.9

    The path to the location where the downloaded files are stored.

    Overrides the default path used for download location, setting it to \a path.

    If set to an empty string, the default path is restored.

    \note By default, the download path is QStandardPaths::DownloadLocation.
*/

void QQuickWebEngineProfile::setDownloadPath(const QString &path)
{
    Q_D(QQuickWebEngineProfile);
    if (downloadPath() == path)
        return;
    d->profileAdapter()->setDownloadPath(path);
    emit downloadPathChanged();
}

QString QQuickWebEngineProfile::downloadPath() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->downloadPath();
}

/*!
    \qmlproperty bool WebEngineProfile::isPushServiceEnabled
    \since  QtWebEngine 6.5

    Whether the push messaging service is enabled.
    \note By default the push messaging service is disabled.
    \note \QWE uses the \l {https://firebase.google.com}{Firebase Cloud Messaging (FCM)} as a browser push service.
    Therefore, all push messages will go through the Google push service and its respective servers.
*/

/*!
    \property QQuickWebEngineProfile::isPushServiceEnabled
    \since QtWebEngine 6.5

    Whether the push messaging service is enabled.
    \note By default the push messaging service is disabled.
    \note \QWE uses the \l {https://firebase.google.com}{Firebase Cloud Messaging (FCM)} as a browser push service.
    Therefore, all push messages will go through the Google push service and its respective servers.
*/

bool QQuickWebEngineProfile::isPushServiceEnabled() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->pushServiceEnabled();
}

void QQuickWebEngineProfile::setPushServiceEnabled(bool enabled)
{
    Q_D(QQuickWebEngineProfile);
    if (isPushServiceEnabled() == enabled)
        return;
    d->profileAdapter()->setPushServiceEnabled(enabled);
    emit pushServiceEnabledChanged();
}

/*!

    Returns the cookie store for this profile.
*/
QWebEngineCookieStore *QQuickWebEngineProfile::cookieStore() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->cookieStore();
}

/*!
    \qmlmethod void WebEngineProfile::clearHttpCache()
    \since QtWebEngine 1.3

    Removes the profile's cache entries.

    \note Make sure that you do not start new navigation or any operation on the profile while
    the clear operation is in progress. The clearHttpCacheCompleted() signal notifies about the
    completion.

    \sa WebEngineProfile::cachePath clearHttpCacheCompleted()
*/

/*!
    \since 5.7

    Removes the profile's cache entries.

    \note Make sure that you do not start new navigation or any operation on the profile while
    the clear operation is in progress. The clearHttpCacheCompleted() signal notifies about the
    completion.

    \sa WebEngineProfile::clearHttpCache() clearHttpCacheCompleted()
*/
void QQuickWebEngineProfile::clearHttpCache()
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->clearHttpCache();
}

/*!
    Registers a request interceptor singleton \a interceptor to intercept URL requests.

    The profile does not take ownership of the pointer.

    \sa QWebEngineUrlRequestInfo QWebEngineUrlRequestInterceptor
*/
void QQuickWebEngineProfile::setUrlRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->setRequestInterceptor(interceptor);
}


/*!
    Returns the custom URL scheme handler register for the URL scheme \a scheme.
*/
const QWebEngineUrlSchemeHandler *QQuickWebEngineProfile::urlSchemeHandler(const QByteArray &scheme) const
{
    const Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->urlSchemeHandler(scheme);
}

/*!
    Registers a handler \a handler for custom URL scheme \a scheme in the profile.

    It is necessary to first register the scheme with \l
    QWebEngineUrlScheme::registerScheme at application startup.
*/
void QQuickWebEngineProfile::installUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *handler)
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->installUrlSchemeHandler(scheme, handler);
}

/*!
    Removes the custom URL scheme handler \a handler from the profile.

    \sa removeUrlScheme()
*/
void QQuickWebEngineProfile::removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *handler)
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->removeUrlSchemeHandler(handler);
}

/*!
    Removes the custom URL scheme \a scheme from the profile.

    \sa removeUrlSchemeHandler()
*/
void QQuickWebEngineProfile::removeUrlScheme(const QByteArray &scheme)
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->removeUrlScheme(scheme);
}

/*!
    Removes all custom URL scheme handlers installed in the profile.
*/
void QQuickWebEngineProfile::removeAllUrlSchemeHandlers()
{
    Q_D(QQuickWebEngineProfile);
    d->profileAdapter()->removeAllUrlSchemeHandlers();
}

QQuickWebEngineSettings *QQuickWebEngineProfile::settings() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->settings();
}

/*!
    \qmlproperty WebEngineScriptCollection WebEngineProfile::userScripts
    \since 1.5

    Returns the collection of WebEngineScript objects that are injected into
    all pages that share this profile.
*/

QQuickWebEngineScriptCollection *QQuickWebEngineProfile::userScripts() const
{
    return d_ptr->getUserScripts();
}

/*!
    \since 5.13

    Returns the profile's client certificate store.
*/
QWebEngineClientCertificateStore *QQuickWebEngineProfile::clientCertificateStore()
{
#if QT_CONFIG(ssl)
    Q_D(QQuickWebEngineProfile);
    return d->profileAdapter()->clientCertificateStore();
#else
    return nullptr;
#endif
}

/*!
    Return the Client Hints settings associated with this browsing context.

    \since 6.8
    \sa QWebEngineClientHints
*/
QWebEngineClientHints *QQuickWebEngineProfile::clientHints() const
{
    Q_D(const QQuickWebEngineProfile);
    return d->m_clientHints.data();
}

/*!
    \fn QQuickWebEngineProfile::queryPermission(const QUrl &securityOrigin, QWebEnginePermission::PermissionType permissionType) const

    Returns a QWebEnginePermission object corresponding to a single permission for the provided \a securityOrigin and
    \a permissionType. The object may be used to query for the current state of the permission, or to change it. It is not required
    for a permission to already exist; the returned object may also be used to pre-grant a permission if a website is
    known to use it.

    \note This may only be used for persistent permission types. Calling it with a non-persistent \a permissionType will return an invalid object.
    \since 6.8
    \sa listAllPermissions(), listPermissionsForOrigin(), listPermissionsForPermissionType(), QWebEnginePermission::PermissionType
 */

/*!
    \qmlmethod void WebEngineProfile::queryPermission(url securityOrigin, WebEnginePermission.PermissionType permissionType) const

    Returns a webEnginePermission object corresponding to a single permission for the provided \a securityOrigin and
    \a permissionType. The object may be used to query for the current state of the permission, or to change it. It is not required
    for a permission to already exist; the returned object may also be used to pre-grant a permission if a website is
    known to use it.

    \note This may only be used for persistent permission types. Calling it with a non-persistent \a permissionType will return an invalid object.
    \since 6.8
    \sa listAllPermissions(), listPermissionsForOrigin(), listPermissionsForPermissionType()
 */
QWebEnginePermission QQuickWebEngineProfile::queryPermission(const QUrl &securityOrigin, QWebEnginePermission::PermissionType permissionType) const
{
    Q_D(const QQuickWebEngineProfile);

    if (permissionType == QWebEnginePermission::PermissionType::Unsupported) {
        qWarning("Attempting to get unsupported permission. Returned object will be in an invalid state.");
        return QWebEnginePermission(new QWebEnginePermissionPrivate());
    }

    if (!QWebEnginePermission::isPersistent(permissionType)) {
        qWarning() << "Attempting to get permission for permission type" << permissionType << ". Returned object will be in an invalid state.";
        return QWebEnginePermission(new QWebEnginePermissionPrivate());
    }

    auto *pvt = new QWebEnginePermissionPrivate(securityOrigin, permissionType, nullptr, d->profileAdapter());
    return QWebEnginePermission(pvt);
}

/*!
    \qmlmethod list<webEnginePermission> WebEngineProfile::listAllPermissions() const

    Returns a \l list of webEnginePermission objects, each one representing a single permission currently
    present in the permissions store. The returned list contains all previously granted/denied permissions for this profile,
    provided they are of a \e persistent type.

    \note When the persistentPermissionPolicy property is set to \c AskEveryTime, this will return an empty list.
    \since 6.8
    \sa queryPermission(), listPermissionsForOrigin(), listPermissionsForPermissionType(), webEnginePermission::isPersistent()
 */

/*!
    Returns a QList of QWebEnginePermission objects, each one representing a single permission currently
    present in the permissions store. The returned list contains all previously granted/denied permissions for this profile,
    provided they are of a \e persistent type.

    \note When persistentPermissionPolicy() is set to \c AskEveryTime, this will return an empty list.
    \since 6.8
    \sa queryPermission(), listPermissionsForOrigin(), listPermissionsForPermissionType(), QWebEnginePermission::isPersistent()
 */
QList<QWebEnginePermission> QQuickWebEngineProfile::listAllPermissions() const
{
    Q_D(const QQuickWebEngineProfile);
    if (persistentPermissionsPolicy() == PersistentPermissionsPolicy::AskEveryTime)
        return QList<QWebEnginePermission>();
    return d->profileAdapter()->listPermissions();
}

/*!
    \qmlmethod list<webEnginePermission> WebEngineProfile::listPermissionsForOrigin(url securityOrigin) const

    Returns a \l list of webEnginePermission objects, each one representing a single permission currently
    present in the permissions store. The returned list contains all previously granted/denied permissions associated with a
    specific \a securityOrigin for this profile, provided they are of a \e persistent type.

    \note Since permissions are granted on a per-origin basis, the provided \a securityOrigin will be stripped to its
    origin form, and the returned list will contain all permissions for the origin. Thus, passing https://www.example.com/some/page.html
    is the same as passing just https://www.example.com/.
    \note When persistentPermissionPolicy() is set to \c AskEveryTime, this will return an empty list.
    \since 6.8
    \sa queryPermission(), listAllPermissions(), listPermissionsForPermissionType(), webEnginePermission::isPersistent()
 */

/*!
    Returns a QList of QWebEnginePermission objects, each one representing a single permission currently
    present in the permissions store. The returned list contains all previously granted/denied permissions associated with a
    specific \a securityOrigin for this profile, provided they are of a \e persistent type.

    \note Since permissions are granted on a per-origin basis, the provided \a securityOrigin will be stripped to its
    origin form, and the returned list will contain all permissions for the origin. Thus, passing https://www.example.com/some/page.html
    is the same as passing just https://www.example.com/.
    \since 6.8
    \sa queryPermission(), listAllPermissions(), listPermissionsForPermissionType(), QWebEnginePermission::isPersistent()
 */
QList<QWebEnginePermission> QQuickWebEngineProfile::listPermissionsForOrigin(const QUrl &securityOrigin) const
{
    Q_D(const QQuickWebEngineProfile);
    if (persistentPermissionsPolicy() == PersistentPermissionsPolicy::AskEveryTime)
        return QList<QWebEnginePermission>();
    return d->profileAdapter()->listPermissions(securityOrigin);
}

/*!
    \qmlmethod list<webEnginePermission> WebEngineProfile::listPermissionsForPermissionType(WebEnginePermission.PermissionType permissionType) const

    Returns a \l list of webEnginePermission objects, each one representing a single permission currently
    present in the permissions store. The returned list contains all previously granted/denied permissions of the provided
    \a permissionType. If the \permissionType is non-persistent, the list will be empty.

    \note When persistentPermissionPolicy() is set to \c AskEveryTime, this will return an empty list.
    \since 6.8
    \sa queryPermission(), listAllPermissions(), listPermissionsForOrigin(), webEnginePermission::isPersistent()
 */

/*!
    Returns a QList of QWebEnginePermission objects, each one representing a single permission currently
    present in the permissions store. The returned list contains all previously granted/denied permissions of the provided
    \a permissionType. If the \permissionType is non-persistent, the list will be empty.

    \note When persistentPermissionPolicy() is set to \c AskEveryTime, this will return an empty list.
    \since 6.8
    \sa queryPermission(), listAllPermissions(), listPermissionsForOrigin(), QWebEnginePermission::PermissionType, QWebEnginePermission::isPersistent()
 */
QList<QWebEnginePermission> QQuickWebEngineProfile::listPermissionsForPermissionType(QWebEnginePermission::PermissionType permissionType) const
{
    Q_D(const QQuickWebEngineProfile);
    if (persistentPermissionsPolicy() == PersistentPermissionsPolicy::AskEveryTime)
        return QList<QWebEnginePermission>();

    if (permissionType == QWebEnginePermission::PermissionType::Unsupported) {
        qWarning("Attempting to get permission list for an unsupported type. Returned list will be empty.");
        return QList<QWebEnginePermission>();
    }

    if (!QWebEnginePermission::isPersistent(permissionType)) {
        qWarning() << "Attempting to get permission list for permission type" << permissionType << ". Returned list will be empty.";
        return QList<QWebEnginePermission>();
    }

    return d->profileAdapter()->listPermissions(QUrl(), permissionType);
}

void QQuickWebEngineProfile::ensureQmlContext(const QObject *object)
{
    if (!qmlContext(this)) {
        auto engine = qmlEngine(object);
        QQmlEngine::setContextForObject(this, new QQmlContext(engine, engine));
    }
}

QT_END_NAMESPACE

#include "moc_qquickwebengineprofile.cpp"
