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

#ifndef BROWSER_CONTEXT_ADAPTER_H
#define BROWSER_CONTEXT_ADAPTER_H

#include "qtwebenginecoreglobal.h"

#include <QList>
#include <QScopedPointer>
#include <QSharedData>
#include <QString>
#include <QVector>

QT_FORWARD_DECLARE_CLASS(QObject)

namespace QtWebEngineCore {

class BrowserContextAdapterClient;
class BrowserContextQt;
class CustomUrlSchemeHandler;
class DownloadManagerDelegateQt;
class UserScriptControllerHost;
class WebEngineVisitedLinksManager;

class QWEBENGINE_EXPORT BrowserContextAdapter : public QSharedData
{
public:
    explicit BrowserContextAdapter(bool offTheRecord = false);
    explicit BrowserContextAdapter(const QString &storagePrefix);
    virtual ~BrowserContextAdapter();

    static BrowserContextAdapter* defaultContext();
    static QObject* globalQObjectRoot();

    WebEngineVisitedLinksManager *visitedLinksManager();
    DownloadManagerDelegateQt *downloadManagerDelegate();

    QList<BrowserContextAdapterClient*> clients() { return m_clients; }
    void addClient(BrowserContextAdapterClient *adapterClient);
    void removeClient(BrowserContextAdapterClient *adapterClient);

    void cancelDownload(quint32 downloadId);

    BrowserContextQt *browserContext();

    QString storageName() const { return m_name; }
    void setStorageName(const QString &storageName);

    bool isOffTheRecord() const { return m_offTheRecord; }
    void setOffTheRecord(bool offTheRecord);

    QString dataPath() const;
    void setDataPath(const QString &path);

    QString cachePath() const;
    void setCachePath(const QString &path);

    QString httpCachePath() const;
    QString cookiesPath() const;

    QString httpUserAgent() const;
    void setHttpUserAgent(const QString &userAgent);

    // KEEP IN SYNC with API or add mapping layer
    enum HttpCacheType {
        MemoryHttpCache = 0,
        DiskHttpCache
    };

    enum PersistentCookiesPolicy {
        NoPersistentCookies = 0,
        AllowPersistentCookies,
        ForcePersistentCookies
    };

    enum VisitedLinksPolicy {
        DoNotTrackVisitedLinks = 0,
        TrackVisitedLinksInMemory,
        TrackVisitedLinksOnDisk,
    };

    HttpCacheType httpCacheType() const;
    void setHttpCacheType(BrowserContextAdapter::HttpCacheType);

    PersistentCookiesPolicy persistentCookiesPolicy() const;
    void setPersistentCookiesPolicy(BrowserContextAdapter::PersistentCookiesPolicy);

    VisitedLinksPolicy visitedLinksPolicy() const;
    void setVisitedLinksPolicy(BrowserContextAdapter::VisitedLinksPolicy);

    int httpCacheMaxSize() const;
    void setHttpCacheMaxSize(int maxSize);

    bool trackVisitedLinks() const;
    bool persistVisitedLinks() const;

    QVector<CustomUrlSchemeHandler*> &customUrlSchemeHandlers();
    void updateCustomUrlSchemeHandlers();
    UserScriptControllerHost *userScriptController();

private:
    QString m_name;
    bool m_offTheRecord;
    QScopedPointer<BrowserContextQt> m_browserContext;
    QScopedPointer<WebEngineVisitedLinksManager> m_visitedLinksManager;
    QScopedPointer<DownloadManagerDelegateQt> m_downloadManagerDelegate;
    QScopedPointer<UserScriptControllerHost> m_userScriptController;

    QString m_dataPath;
    QString m_cachePath;
    QString m_httpUserAgent;
    HttpCacheType m_httpCacheType;
    PersistentCookiesPolicy m_persistentCookiesPolicy;
    VisitedLinksPolicy m_visitedLinksPolicy;
    QVector<CustomUrlSchemeHandler*> m_customUrlSchemeHandlers;
    QList<BrowserContextAdapterClient*> m_clients;
    int m_httpCacheMaxSize;

    Q_DISABLE_COPY(BrowserContextAdapter)
};

} // namespace QtWebEngineCore

#endif // BROWSER_CONTEXT_ADAPTER_H
