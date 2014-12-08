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

#ifndef BROWSER_CONTEXT_ADAPTER_H
#define BROWSER_CONTEXT_ADAPTER_H

#include "qtwebenginecoreglobal.h"

#include <QScopedPointer>
#include <QSharedData>
#include <QString>

class BrowserContextQt;
class WebEngineVisitedLinksManager;

class QWEBENGINE_EXPORT BrowserContextAdapter : public QSharedData
{
public:
    explicit BrowserContextAdapter(bool offTheRecord = false);
    explicit BrowserContextAdapter(const QString &storagePrefix);
    virtual ~BrowserContextAdapter();

    static BrowserContextAdapter* defaultContext();
    static BrowserContextAdapter* offTheRecordContext();

    WebEngineVisitedLinksManager *visitedLinksManager();

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

private:
    QString m_name;
    bool m_offTheRecord;
    QScopedPointer<BrowserContextQt> m_browserContext;
    QScopedPointer<WebEngineVisitedLinksManager> m_visitedLinksManager;
    QString m_dataPath;
    QString m_cachePath;
    QString m_httpUserAgent;
    HttpCacheType m_httpCacheType;
    PersistentCookiesPolicy m_persistentCookiesPolicy;
    VisitedLinksPolicy m_visitedLinksPolicy;
    int m_httpCacheMaxSize;

    Q_DISABLE_COPY(BrowserContextAdapter)
};

#endif // BROWSER_CONTEXT_ADAPTER_H
