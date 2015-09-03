/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickwebengineprofile_p.h"

#include "qquickwebenginedownloaditem_p.h"
#include "qquickwebenginedownloaditem_p_p.h"
#include "qquickwebengineprofile_p_p.h"
#include "qquickwebenginesettings_p.h"

#include <QQmlEngine>

#include "browser_context_adapter.h"
#include "web_engine_settings.h"

using QtWebEngineCore::BrowserContextAdapter;

QT_BEGIN_NAMESPACE

QQuickWebEngineProfilePrivate::QQuickWebEngineProfilePrivate(BrowserContextAdapter* browserContext)
        : m_settings(new QQuickWebEngineSettings())
        , m_browserContextRef(browserContext)
{
    m_browserContextRef->addClient(this);
    m_settings->d_ptr->initDefaults(browserContext->isOffTheRecord());
}

QQuickWebEngineProfilePrivate::~QQuickWebEngineProfilePrivate()
{
    m_browserContextRef->removeClient(this);

    Q_FOREACH (QQuickWebEngineDownloadItem* download, m_ongoingDownloads) {
        if (download)
            download->cancel();
    }

    m_ongoingDownloads.clear();
}

void QQuickWebEngineProfilePrivate::cancelDownload(quint32 downloadId)
{
    browserContext()->cancelDownload(downloadId);
}

void QQuickWebEngineProfilePrivate::downloadDestroyed(quint32 downloadId)
{
    m_ongoingDownloads.remove(downloadId);
}

void QQuickWebEngineProfilePrivate::downloadRequested(DownloadItemInfo &info)
{
    Q_Q(QQuickWebEngineProfile);

    Q_ASSERT(!m_ongoingDownloads.contains(info.id));
    QQuickWebEngineDownloadItemPrivate *itemPrivate = new QQuickWebEngineDownloadItemPrivate(q);
    itemPrivate->downloadId = info.id;
    itemPrivate->downloadState = QQuickWebEngineDownloadItem::DownloadRequested;
    itemPrivate->totalBytes = info.totalBytes;
    itemPrivate->downloadPath = info.path;

    QQuickWebEngineDownloadItem *download = new QQuickWebEngineDownloadItem(itemPrivate, q);

    m_ongoingDownloads.insert(info.id, download);

    QQmlEngine::setObjectOwnership(download, QQmlEngine::JavaScriptOwnership);
    Q_EMIT q->downloadRequested(download);

    QQuickWebEngineDownloadItem::DownloadState state = download->state();
    info.path = download->path();
    info.accepted = state != QQuickWebEngineDownloadItem::DownloadCancelled
                      && state != QQuickWebEngineDownloadItem::DownloadRequested;
}

void QQuickWebEngineProfilePrivate::downloadUpdated(const DownloadItemInfo &info)
{
    if (!m_ongoingDownloads.contains(info.id))
        return;

    Q_Q(QQuickWebEngineProfile);

    QQuickWebEngineDownloadItem* download = m_ongoingDownloads.value(info.id).data();

    if (!download) {
        downloadDestroyed(info.id);
        return;
    }

    download->d_func()->update(info);

    if (info.state != BrowserContextAdapterClient::DownloadInProgress) {
        Q_EMIT q->downloadFinished(download);
        m_ongoingDownloads.remove(info.id);
    }
}

/*!
    \qmltype WebEngineProfile
    \instantiates QQuickWebEngineProfile
    \inqmlmodule QtWebEngine 1.1
    \since QtWebEngine 1.1
    \brief Contains common settings for multiple web engine views.

    Contains settings and history shared by all the web engine views that belong
    to the profile.

    Each web engine view has an associated profile. Views that do not have a specific profile set
    share a common default one.
*/

/*!
    \qmlsignal WebEngineProfile::downloadRequested(WebEngineDownloadItem download)

    This signal is emitted whenever a download has been triggered.
    The \a download argument holds the state of the download.
    The download has to be explicitly accepted with WebEngineDownloadItem::accept() or the
    download will be cancelled by default.
*/

/*!
    \qmlsignal WebEngineProfile::downloadFinished(WebEngineDownloadItem download)

    This signal is emitted whenever downloading stops, because it finished successfully, was
    cancelled, or was interrupted (for example, because connectivity was lost).
    The \a download argument holds the state of the finished download instance.
*/

QQuickWebEngineProfile::QQuickWebEngineProfile()
    : d_ptr(new QQuickWebEngineProfilePrivate(new BrowserContextAdapter(false)))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineProfile::QQuickWebEngineProfile(QQuickWebEngineProfilePrivate *privatePtr, QObject *parent)
    : QObject(parent)
    , d_ptr(privatePtr)
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineProfile::~QQuickWebEngineProfile()
{
}

/*!
    \qmlproperty QString WebEngineProfile::storageName

    The storage name that is used to create separate subdirectories for each profile that uses
    the disk for storing persistent data and cache.

    \sa WebEngineProfile::persistentStoragePath, WebEngineProfile::cachePath
*/

QString QQuickWebEngineProfile::storageName() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->browserContext()->storageName();
}

void QQuickWebEngineProfile::setStorageName(const QString &name)
{
    Q_D(QQuickWebEngineProfile);
    if (d->browserContext()->storageName() == name)
        return;
    BrowserContextAdapter::HttpCacheType oldCacheType = d->browserContext()->httpCacheType();
    BrowserContextAdapter::PersistentCookiesPolicy oldPolicy = d->browserContext()->persistentCookiesPolicy();
    d->browserContext()->setStorageName(name);
    emit storageNameChanged();
    emit persistentStoragePathChanged();
    emit cachePathChanged();
    if (d->browserContext()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
    if (d->browserContext()->persistentCookiesPolicy() != oldPolicy)
        emit persistentCookiesPolicyChanged();
}

/*!
    \qmlproperty bool WebEngineProfile::offTheRecord

    Whether the web engine profile is \e off-the-record.
    An off-the-record profile forces cookies, the HTTP cache, and other normally persistent data
    to be stored only in memory.
*/
bool QQuickWebEngineProfile::isOffTheRecord() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->browserContext()->isOffTheRecord();
}

void QQuickWebEngineProfile::setOffTheRecord(bool offTheRecord)
{
    Q_D(QQuickWebEngineProfile);
    if (d->browserContext()->isOffTheRecord() == offTheRecord)
        return;
    BrowserContextAdapter::HttpCacheType oldCacheType = d->browserContext()->httpCacheType();
    BrowserContextAdapter::PersistentCookiesPolicy oldPolicy = d->browserContext()->persistentCookiesPolicy();
    d->browserContext()->setOffTheRecord(offTheRecord);
    emit offTheRecordChanged();
    if (d->browserContext()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
    if (d->browserContext()->persistentCookiesPolicy() != oldPolicy)
        emit persistentCookiesPolicyChanged();
}

/*!
    \qmlproperty QString WebEngineProfile::persistentStoragePath

    The path to the location where the persistent data for the browser and web content are
    stored. Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, the storage is located below
    QStandardPaths::writableLocation(QStandardPaths::DataLocation) in a directory named using
    storageName.
*/
QString QQuickWebEngineProfile::persistentStoragePath() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->browserContext()->dataPath();
}

void QQuickWebEngineProfile::setPersistentStoragePath(const QString &path)
{
    Q_D(QQuickWebEngineProfile);
    if (persistentStoragePath() == path)
        return;
    d->browserContext()->setDataPath(path);
    emit persistentStoragePathChanged();
}

/*!
    \qmlproperty QString WebEngineProfile::cachePath

    The path to the location where the profile's caches are stored, in particular the HTTP cache.

    By default, the caches are stored
    below QStandardPaths::writableLocation(QStandardPaths::CacheLocation) in a directory named using
    storageName.
*/
QString QQuickWebEngineProfile::cachePath() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->browserContext()->cachePath();
}

void QQuickWebEngineProfile::setCachePath(const QString &path)
{
    Q_D(QQuickWebEngineProfile);
    if (cachePath() == path)
        return;
    d->browserContext()->setCachePath(path);
    emit cachePathChanged();
}

/*!
    \qmlproperty QString WebEngineProfile::httpUserAgent

    The user-agent string sent with HTTP to identify the browser.
*/
QString QQuickWebEngineProfile::httpUserAgent() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->browserContext()->httpUserAgent();
}

void QQuickWebEngineProfile::setHttpUserAgent(const QString &userAgent)
{
    Q_D(QQuickWebEngineProfile);
    if (d->browserContext()->httpUserAgent() == userAgent)
        return;
    d->browserContext()->setHttpUserAgent(userAgent);
    emit httpUserAgentChanged();
}


/*!
    \qmlproperty enumeration WebEngineProfile::httpCacheType

    This enumeration describes the type of the HTTP cache:

    \value  MemoryHttpCache
            Uses an in-memory cache. This is the only setting possible if offTheRecord is set or
            no persistentStoragePath is available.
    \value  DiskHttpCache
            Uses a disk cache. This is the default value.
*/

QQuickWebEngineProfile::HttpCacheType QQuickWebEngineProfile::httpCacheType() const
{
    const Q_D(QQuickWebEngineProfile);
    return QQuickWebEngineProfile::HttpCacheType(d->browserContext()->httpCacheType());
}

void QQuickWebEngineProfile::setHttpCacheType(QQuickWebEngineProfile::HttpCacheType httpCacheType)
{
    Q_D(QQuickWebEngineProfile);
    BrowserContextAdapter::HttpCacheType oldCacheType = d->browserContext()->httpCacheType();
    d->browserContext()->setHttpCacheType(BrowserContextAdapter::HttpCacheType(httpCacheType));
    if (d->browserContext()->httpCacheType() != oldCacheType)
        emit httpCacheTypeChanged();
}

/*!
    \qmlproperty enumeration WebEngineProfile::persistentCookiesPolicy

    This enumeration describes the policy of cookie persistency:

    \value  NoPersistentCookies
            Both session and persistent cookies are stored in memory. This is the only setting
            possible if offTheRecord is set or no persistentStoragePath is available.
    \value  AllowPersistentCookies
            Cookies marked persistent are saved to and restored from disk, whereas session cookies
            are only stored to disk for crash recovery. This is the default setting.
    \value  ForcePersistentCookies
            Both session and persistent cookies are saved to and restored from disk.
*/

QQuickWebEngineProfile::PersistentCookiesPolicy QQuickWebEngineProfile::persistentCookiesPolicy() const
{
    const Q_D(QQuickWebEngineProfile);
    return QQuickWebEngineProfile::PersistentCookiesPolicy(d->browserContext()->persistentCookiesPolicy());
}

void QQuickWebEngineProfile::setPersistentCookiesPolicy(QQuickWebEngineProfile::PersistentCookiesPolicy newPersistentCookiesPolicy)
{
    Q_D(QQuickWebEngineProfile);
    BrowserContextAdapter::PersistentCookiesPolicy oldPolicy = d->browserContext()->persistentCookiesPolicy();
    d->browserContext()->setPersistentCookiesPolicy(BrowserContextAdapter::PersistentCookiesPolicy(newPersistentCookiesPolicy));
    if (d->browserContext()->persistentCookiesPolicy() != oldPolicy)
        emit persistentCookiesPolicyChanged();
}

/*!
    \qmlproperty int WebEngineProfile::httpCacheMaximumSize

    The maximum size of the HTTP cache. If \c 0, the size will be controlled automatically by
    QtWebEngine. The default value is \c 0.

    \sa httpCacheType
*/
int QQuickWebEngineProfile::httpCacheMaximumSize() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->browserContext()->httpCacheMaxSize();
}

void QQuickWebEngineProfile::setHttpCacheMaximumSize(int maximumSize)
{
    Q_D(QQuickWebEngineProfile);
    if (d->browserContext()->httpCacheMaxSize() == maximumSize)
        return;
    d->browserContext()->setHttpCacheMaxSize(maximumSize);
    emit httpCacheMaximumSizeChanged();
}

QQuickWebEngineProfile *QQuickWebEngineProfile::defaultProfile()
{
    static QQuickWebEngineProfile *profile = new QQuickWebEngineProfile(
                new QQuickWebEngineProfilePrivate(BrowserContextAdapter::defaultContext()),
                BrowserContextAdapter::globalQObjectRoot());
    return profile;
}

QQuickWebEngineSettings *QQuickWebEngineProfile::settings() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->settings();
}

QT_END_NAMESPACE
