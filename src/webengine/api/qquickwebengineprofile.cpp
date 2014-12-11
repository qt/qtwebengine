/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
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
#include <QQmlEngine>

#include "browser_context_adapter.h"

QT_BEGIN_NAMESPACE

static inline QQuickWebEngineDownloadItem::DownloadState toDownloadState(int state) {
    switch (state) {
    case BrowserContextAdapterClient::DownloadInProgress:
        return QQuickWebEngineDownloadItem::DownloadInProgress;
    case BrowserContextAdapterClient::DownloadCompleted:
        return QQuickWebEngineDownloadItem::DownloadCompleted;
    case BrowserContextAdapterClient::DownloadCancelled:
        return QQuickWebEngineDownloadItem::DownloadCancelled;
    case BrowserContextAdapterClient::DownloadInterrupted:
        return QQuickWebEngineDownloadItem::DownloadInterrupted;
    default:
        Q_UNREACHABLE();
        return QQuickWebEngineDownloadItem::DownloadCancelled;
    }
}

QQuickWebEngineProfilePrivate::QQuickWebEngineProfilePrivate(BrowserContextAdapter* browserContext, bool ownsContext)
        : m_browserContext(browserContext)
{
    if (ownsContext)
        m_browserContextRef = browserContext;

    m_browserContext->setClient(this);
}

QQuickWebEngineProfilePrivate::~QQuickWebEngineProfilePrivate()
{
    m_browserContext->setClient(0);

    Q_FOREACH (QQuickWebEngineDownloadItem* download, m_ongoingDownloads) {
        if (download)
            download->cancel();
    }

    m_ongoingDownloads.clear();
}

void QQuickWebEngineProfilePrivate::cancelDownload(quint32 downloadId)
{
    m_browserContext->cancelDownload(downloadId);
}

void QQuickWebEngineProfilePrivate::downloadDestroyed(quint32 downloadId)
{
    m_ongoingDownloads.remove(downloadId);
}

void QQuickWebEngineProfilePrivate::downloadRequested(quint32 downloadId, QString &downloadPath, bool &cancelled)
{
    Q_Q(QQuickWebEngineProfile);

    Q_ASSERT(!m_ongoingDownloads.contains(downloadId));
    QQuickWebEngineDownloadItemPrivate *itemPrivate = new QQuickWebEngineDownloadItemPrivate(this);
    itemPrivate->downloadId = downloadId;
    itemPrivate->downloadState = QQuickWebEngineDownloadItem::DownloadInProgress;
    itemPrivate->downloadPath = downloadPath;

    QQuickWebEngineDownloadItem *download = new QQuickWebEngineDownloadItem(itemPrivate, q);

    m_ongoingDownloads.insert(downloadId, download);

    QQmlEngine::setObjectOwnership(download, QQmlEngine::JavaScriptOwnership);
    Q_EMIT q->downloadStarted(download);
    download->d_func()->downloadStarted = true;

    downloadPath = download->path();
    cancelled = download->state() == QQuickWebEngineDownloadItem::DownloadCancelled;
}

void QQuickWebEngineProfilePrivate::downloadUpdated(quint32 downloadId, int downloadState, int percentComplete)
{
    Q_Q(QQuickWebEngineProfile);

    Q_ASSERT(m_ongoingDownloads.contains(downloadId));
    QQuickWebEngineDownloadItem* download = m_ongoingDownloads.value(downloadId).data();

    if (!download) {
        cancelDownload(downloadId);
        return;
    }

    download->d_func()->update(toDownloadState(downloadState), percentComplete);

    if (downloadState != BrowserContextAdapterClient::DownloadInProgress) {
        Q_EMIT q->downloadFinished(download);
        m_ongoingDownloads.remove(downloadId);
    }
}

QQuickWebEngineProfile::QQuickWebEngineProfile()
    : d_ptr(new QQuickWebEngineProfilePrivate(new BrowserContextAdapter(false), true))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineProfile::QQuickWebEngineProfile(QQuickWebEngineProfilePrivate *privatePtr)
    : d_ptr(privatePtr)
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineProfile::~QQuickWebEngineProfile()
{
}

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

int QQuickWebEngineProfile::httpCacheMaxSize() const
{
    const Q_D(QQuickWebEngineProfile);
    return d->browserContext()->httpCacheMaxSize();
}

void QQuickWebEngineProfile::setHttpCacheMaxSize(int maxSize)
{
    Q_D(QQuickWebEngineProfile);
    if (d->browserContext()->httpCacheMaxSize() == maxSize)
        return;
    d->browserContext()->setHttpCacheMaxSize(maxSize);
    emit httpCacheMaxSizeChanged();
}

QQuickWebEngineProfile *QQuickWebEngineProfile::defaultProfile()
{
    static QQuickWebEngineProfile profile(new QQuickWebEngineProfilePrivate(BrowserContextAdapter::defaultContext(), false));
    return &profile;
}

QT_END_NAMESPACE
