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

#include "qwebengineprofile.h"

#include "qwebenginecookiestoreclient.h"
#include "qwebenginedownloaditem.h"
#include "qwebenginedownloaditem_p.h"
#include "qwebenginepage.h"
#include "qwebengineprofile_p.h"
#include "qwebenginesettings.h"
#include "qwebenginescriptcollection_p.h"

#include "browser_context_adapter.h"
#include "web_engine_visited_links_manager.h"
#include "web_engine_settings.h"

QT_BEGIN_NAMESPACE

using QtWebEngineCore::BrowserContextAdapter;

/*!
    \class QWebEngineProfile
    \brief The QWebEngineProfile class provides a web-engine profile shared by multiple pages.
    \since 5.5

    \inmodule QtWebEngineWidgets

    QWebEngineProfile contains settings, scripts, and the list of visited links shared by all
    web engine pages that belong to the profile. As such, profiles can be used to isolate pages
    from each other. A typical use case is a dedicated profile for a 'private browsing' mode.

    The default profile is a built-in profile that all web pages not specifically created with
    another profile belong to.
*/

/*!
    \enum QWebEngineProfile::HttpCacheType

    This enum describes the HTTP cache type:

    \value MemoryHttpCache Use an in-memory cache. This is the only setting possible if
    \c off-the-record is set or no cache path is available.
    \value DiskHttpCache Use a disk cache. This is the default.
*/

/*!
    \enum QWebEngineProfile::PersistentCookiesPolicy

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
  \fn QWebEngineProfile::downloadRequested(QWebEngineDownloadItem *download)

  \since 5.5

  This signal is emitted whenever a download has been triggered.
  The \a download argument holds the state of the download.
  The download has to be explicitly accepted with QWebEngineDownloadItem::accept() or it will be
  cancelled by default.
  The download item is parented by the profile. If it is not accepted, it
  will be deleted immediately after the signal emission.
  This signal cannot be used with a queued connection.

  \sa QWebEngineDownloadItem
*/

QWebEngineProfilePrivate::QWebEngineProfilePrivate(BrowserContextAdapter* browserContext)
        : m_settings(new QWebEngineSettings())
        , m_scriptCollection(new QWebEngineScriptCollection(new QWebEngineScriptCollectionPrivate(browserContext->userScriptController())))
        , m_browserContextRef(browserContext)
{
    m_browserContextRef->addClient(this);
    m_settings->d_ptr->initDefaults(browserContext->isOffTheRecord());
}

QWebEngineProfilePrivate::~QWebEngineProfilePrivate()
{
    delete m_settings;
    m_settings = 0;
    m_browserContextRef->removeClient(this);

    Q_FOREACH (QWebEngineDownloadItem* download, m_ongoingDownloads) {
        if (download)
            download->cancel();
    }

    m_ongoingDownloads.clear();
}

void QWebEngineProfilePrivate::cancelDownload(quint32 downloadId)
{
    browserContext()->cancelDownload(downloadId);
}

void QWebEngineProfilePrivate::downloadDestroyed(quint32 downloadId)
{
    m_ongoingDownloads.remove(downloadId);
}

void QWebEngineProfilePrivate::downloadRequested(DownloadItemInfo &info)
{
    Q_Q(QWebEngineProfile);

    Q_ASSERT(!m_ongoingDownloads.contains(info.id));
    QWebEngineDownloadItemPrivate *itemPrivate = new QWebEngineDownloadItemPrivate(this, info.url);
    itemPrivate->downloadId = info.id;
    itemPrivate->downloadState = QWebEngineDownloadItem::DownloadRequested;
    itemPrivate->downloadPath = info.path;

    QWebEngineDownloadItem *download = new QWebEngineDownloadItem(itemPrivate, q);

    m_ongoingDownloads.insert(info.id, download);

    Q_EMIT q->downloadRequested(download);

    QWebEngineDownloadItem::DownloadState state = download->state();

    info.path = download->path();
    info.accepted = state != QWebEngineDownloadItem::DownloadCancelled;

    if (state == QWebEngineDownloadItem::DownloadRequested) {
        // Delete unaccepted downloads.
        info.accepted = false;
        m_ongoingDownloads.remove(info.id);
        delete download;
    }
}

void QWebEngineProfilePrivate::downloadUpdated(const DownloadItemInfo &info)
{
    if (!m_ongoingDownloads.contains(info.id))
        return;

    QWebEngineDownloadItem* download = m_ongoingDownloads.value(info.id).data();

    if (!download) {
        downloadDestroyed(info.id);
        return;
    }

    download->d_func()->update(info);

    if (download->isFinished())
        m_ongoingDownloads.remove(info.id);
}

/*!
    Constructs a new off-the-record profile with the parent \a parent.

    An off-the-record profile leaves no record on the local machine, and has no persistent data or cache.
    Thus, the HTTP cache can only be in memory and the cookies can only be non-persistent. Trying to change
    these settings will have no effect.

    \sa isOffTheRecord()
*/
QWebEngineProfile::QWebEngineProfile(QObject *parent)
    : QObject(parent)
    , d_ptr(new QWebEngineProfilePrivate(new BrowserContextAdapter(false)))
{
    d_ptr->q_ptr = this;
}

/*!
    Constructs a new profile with the storage name \a storageName and parent \a parent.

    The storage name must be unique.

    A disk-based QWebEngineProfile should be destroyed on or before application exit, otherwise the cache
    and persistent data may not be fully flushed to disk.

    \sa storageName()
*/
QWebEngineProfile::QWebEngineProfile(const QString &storageName, QObject *parent)
    : QObject(parent)
    , d_ptr(new QWebEngineProfilePrivate(new BrowserContextAdapter(storageName)))
{
    d_ptr->q_ptr = this;
}

/*! \internal
*/
QWebEngineProfile::QWebEngineProfile(QWebEngineProfilePrivate *privatePtr, QObject *parent)
    : QObject(parent)
    , d_ptr(privatePtr)
{
    d_ptr->q_ptr = this;
}

/*! \internal
*/
QWebEngineProfile::~QWebEngineProfile()
{
}

/*!
    Returns the storage name for the profile.

    The storage name is used to give each profile that uses the disk separate subdirectories for persistent data and cache.
*/
QString QWebEngineProfile::storageName() const
{
    const Q_D(QWebEngineProfile);
    return d->browserContext()->storageName();
}

/*!
    Returns \c true if this is an off-the-record profile that leaves no record on the computer.

    This will force cookies and HTTP cache to be in memory, but also force all other normally
    persistent data to be stored in memory.
*/
bool QWebEngineProfile::isOffTheRecord() const
{
    const Q_D(QWebEngineProfile);
    return d->browserContext()->isOffTheRecord();
}

/*!
    Returns the path used to store persistent data for the browser and web content.

    Persistent data includes persistent cookies, HTML5 local storage, and visited links.

    By default, this is below QStandardPaths::writableLocation() in a storage name specific
    directory.

    \sa setPersistentStoragePath(), storageName(), QStandardPaths::writableLocation()
*/
QString QWebEngineProfile::persistentStoragePath() const
{
    const Q_D(QWebEngineProfile);
    return d->browserContext()->dataPath();
}

/*!
    Overrides the default path used to store persistent web engine data.

    If \a path is set to the null string, the default path is restored.

    \sa persistentStoragePath()
*/
void QWebEngineProfile::setPersistentStoragePath(const QString &path)
{
    const Q_D(QWebEngineProfile);
    d->browserContext()->setDataPath(path);
}

/*!
    Returns the path used for caches.

    By default, this is below QStandardPaths::writableLocation() in a storage name specific
    directory.

    \sa setCachePath(), storageName(), QStandardPaths::writableLocation()
*/
QString QWebEngineProfile::cachePath() const
{
    const Q_D(QWebEngineProfile);
    return d->browserContext()->cachePath();
}

/*!
    Overrides the default path used for disk caches, setting it to \a path.

    If set to the null string, the default path is restored.

    \sa cachePath()
*/
void QWebEngineProfile::setCachePath(const QString &path)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setCachePath(path);
}

/*!
    Returns the user-agent string sent with HTTP to identify the browser.

    \sa setHttpUserAgent()
*/
QString QWebEngineProfile::httpUserAgent() const
{
    const Q_D(QWebEngineProfile);
    return d->browserContext()->httpUserAgent();
}

/*!
    Overrides the default user-agent string, setting it to \a userAgent.

    \sa httpUserAgent()
*/
void QWebEngineProfile::setHttpUserAgent(const QString &userAgent)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setHttpUserAgent(userAgent);
}

/*!
    Returns the type of HTTP cache used.

    If the profile is off-the-record, MemoryHttpCache is returned.

    \sa setHttpCacheType(), cachePath()
*/
QWebEngineProfile::HttpCacheType QWebEngineProfile::httpCacheType() const
{
    const Q_D(QWebEngineProfile);
    return QWebEngineProfile::HttpCacheType(d->browserContext()->httpCacheType());
}

/*!
    Sets the HTTP cache type to \a httpCacheType.

    \sa httpCacheType(), setCachePath()
*/
void QWebEngineProfile::setHttpCacheType(QWebEngineProfile::HttpCacheType httpCacheType)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setHttpCacheType(BrowserContextAdapter::HttpCacheType(httpCacheType));
}

/*!
    Sets the value of the Accept-Language HTTP request-header field to \a httpAcceptLanguage.

    \since 5.6
 */
void QWebEngineProfile::setHttpAcceptLanguage(const QString &httpAcceptLanguage)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setHttpAcceptLanguage(httpAcceptLanguage);
}

/*!
    Returns the value of the Accept-Language HTTP request-header field.

    \since 5.6
 */
QString QWebEngineProfile::httpAcceptLanguage() const
{
    Q_D(const QWebEngineProfile);
    return d->browserContext()->httpAcceptLanguage();
}

/*!
    Returns the current policy for persistent cookies.

    If the profile is off-the-record, NoPersistentCookies is returned.

    \sa setPersistentCookiesPolicy()
*/
QWebEngineProfile::PersistentCookiesPolicy QWebEngineProfile::persistentCookiesPolicy() const
{
    const Q_D(QWebEngineProfile);
    return QWebEngineProfile::PersistentCookiesPolicy(d->browserContext()->persistentCookiesPolicy());
}

/*!
    Sets the policy for persistent cookies to \a newPersistentCookiesPolicy.

    \sa persistentCookiesPolicy()
*/
void QWebEngineProfile::setPersistentCookiesPolicy(QWebEngineProfile::PersistentCookiesPolicy newPersistentCookiesPolicy)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setPersistentCookiesPolicy(BrowserContextAdapter::PersistentCookiesPolicy(newPersistentCookiesPolicy));
}

/*!
    Returns the maximum size of the HTTP cache in bytes.

    Will return \c 0 if the size is automatically controlled by QtWebEngine.

    \sa setHttpCacheMaximumSize(), httpCacheType()
*/
int QWebEngineProfile::httpCacheMaximumSize() const
{
    const Q_D(QWebEngineProfile);
    return d->browserContext()->httpCacheMaxSize();
}

/*!
    Sets the maximum size of the HTTP cache to \a maxSize bytes.

    Setting it to \c 0 means the size will be controlled automatically by QtWebEngine.

    \sa httpCacheMaximumSize(), setHttpCacheType()
*/
void QWebEngineProfile::setHttpCacheMaximumSize(int maxSize)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setHttpCacheMaxSize(maxSize);
}

/*!
    Returns the cookie store client singleton, if one has been set.
*/

QWebEngineCookieStoreClient* QWebEngineProfile::cookieStoreClient()
{
    Q_D(QWebEngineProfile);
    return d->browserContext()->cookieStoreClient();
}

/*!
    Registers a cookie store client singleton \a client to access Chromium's cookies.

    The profile does not take ownership of the pointer.

    \sa QWebEngineCookieStoreClient
*/

void QWebEngineProfile::setCookieStoreClient(QWebEngineCookieStoreClient *client)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setCookieStoreClient(client);
}

/*!
    Registers a request interceptor singleton \a interceptor to intercept URL requests.

    The profile does not take ownership of the pointer.

    \sa QWebEngineUrlRequestInfo
*/

void QWebEngineProfile::setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->setRequestInterceptor(interceptor);
}

/*!
    Clears all links from the visited links database.

    \sa clearVisitedLinks()
*/
void QWebEngineProfile::clearAllVisitedLinks()
{
    Q_D(QWebEngineProfile);
    d->browserContext()->visitedLinksManager()->deleteAllVisitedLinkData();
}

/*!
    Clears the links in \a urls from the visited links database.

    \sa clearAllVisitedLinks()
*/
void QWebEngineProfile::clearVisitedLinks(const QList<QUrl> &urls)
{
    Q_D(QWebEngineProfile);
    d->browserContext()->visitedLinksManager()->deleteVisitedLinkDataForUrls(urls);
}

/*!
    Returns \c true if \a url is considered a visited link by this profile.
*/
bool QWebEngineProfile::visitedLinksContainsUrl(const QUrl &url) const
{
    Q_D(const QWebEngineProfile);
    return d->browserContext()->visitedLinksManager()->containsUrl(url);
}

/*!
    Returns the script collection used by this profile.
    \sa QWebEngineScriptCollection
*/
QWebEngineScriptCollection *QWebEngineProfile::scripts() const
{
    Q_D(const QWebEngineProfile);
    return d->m_scriptCollection.data();
}

/*!
    Returns the default profile.

    The default profile uses the storage name "Default".

    \sa storageName()
*/
QWebEngineProfile *QWebEngineProfile::defaultProfile()
{
    static QWebEngineProfile* profile = new QWebEngineProfile(
                new QWebEngineProfilePrivate(BrowserContextAdapter::defaultContext()),
                BrowserContextAdapter::globalQObjectRoot());
    return profile;
}

/*!
    Returns the default settings for all pages in this profile.
*/
QWebEngineSettings *QWebEngineProfile::settings() const
{
    const Q_D(QWebEngineProfile);
    return d->settings();
}

/*!
    \since 5.6

    Returns the custom URL scheme handler register for the URL scheme \a scheme.
*/
const QWebEngineUrlSchemeHandler *QWebEngineProfile::urlSchemeHandler(const QByteArray &scheme) const
{
    const Q_D(QWebEngineProfile);
    if (d->browserContext()->customUrlSchemeHandlers().contains(scheme))
        return d->browserContext()->customUrlSchemeHandlers().value(scheme);
    return 0;
}

static bool checkInternalScheme(const QByteArray &scheme)
{
    static QSet<QByteArray> internalSchemes;
    if (internalSchemes.isEmpty()) {
        internalSchemes << QByteArrayLiteral("qrc") << QByteArrayLiteral("data") << QByteArrayLiteral("blob")
                        << QByteArrayLiteral("http") << QByteArrayLiteral("ftp") << QByteArrayLiteral("javascript");
    }
    return internalSchemes.contains(scheme);
}

/*!
    \since 5.6

    Installs the custom URL scheme handler \a handler in the profile.
*/
void QWebEngineProfile::installUrlSchemeHandler(QWebEngineUrlSchemeHandler *handler)
{
    Q_D(QWebEngineProfile);
    Q_ASSERT(handler);
    QByteArray scheme = handler->scheme();
    if (checkInternalScheme(scheme)) {
        qWarning() << "Can not install a URL scheme handler overriding internal scheme: " << scheme;
        return;
    }

    if (d->browserContext()->customUrlSchemeHandlers().contains(scheme)) {
        qWarning() << "URL scheme handler already installed for the scheme: " << scheme;
        return;
    }
    d->browserContext()->customUrlSchemeHandlers().insert(scheme, handler);
    d->browserContext()->updateCustomUrlSchemeHandlers();
    connect(handler, SIGNAL(destroyed(QWebEngineUrlSchemeHandler*)), this, SLOT(destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)));
}

/*!
    \since 5.6

    Removes the custom URL scheme handler \a handler from the profile
*/
void QWebEngineProfile::removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *handler)
{
    Q_D(QWebEngineProfile);
    Q_ASSERT(handler);
    if (!handler)
        return;
    int count = d->browserContext()->customUrlSchemeHandlers().remove(handler->scheme());
    if (!count)
        return;
    disconnect(handler, SIGNAL(destroyed(QWebEngineUrlSchemeHandler*)), this, SLOT(destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler*)));
    d->browserContext()->updateCustomUrlSchemeHandlers();
}

/*!
    \since 5.6

    Removes all custom URL scheme handlers installed in the profile.
*/
void QWebEngineProfile::clearUrlSchemeHandlers()
{
    Q_D(QWebEngineProfile);
    d->browserContext()->customUrlSchemeHandlers().clear();
    d->browserContext()->updateCustomUrlSchemeHandlers();
}

void QWebEngineProfile::destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler *obj)
{
    removeUrlSchemeHandler(obj);
}

QT_END_NAMESPACE
