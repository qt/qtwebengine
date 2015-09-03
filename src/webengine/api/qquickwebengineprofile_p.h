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

#ifndef QQUICKWEBENGINEPROFILE_P_H
#define QQUICKWEBENGINEPROFILE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qtwebengineglobal_p.h>

#include <QObject>
#include <QScopedPointer>
#include <QString>

namespace QtWebEngineCore {
class BrowserContextAdapter;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineDownloadItem;
class QQuickWebEngineProfilePrivate;
class QQuickWebEngineSettings;

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineProfile : public QObject {
    Q_OBJECT
    Q_ENUMS(HttpCacheType);
    Q_ENUMS(PersistentCookiesPolicy);
    Q_PROPERTY(QString storageName READ storageName WRITE setStorageName NOTIFY storageNameChanged FINAL)
    Q_PROPERTY(bool offTheRecord READ isOffTheRecord WRITE setOffTheRecord NOTIFY offTheRecordChanged FINAL)
    Q_PROPERTY(QString persistentStoragePath READ persistentStoragePath WRITE setPersistentStoragePath NOTIFY persistentStoragePathChanged FINAL)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged FINAL)
    Q_PROPERTY(QString httpUserAgent READ httpUserAgent WRITE setHttpUserAgent NOTIFY httpUserAgentChanged FINAL)
    Q_PROPERTY(HttpCacheType httpCacheType READ httpCacheType WRITE setHttpCacheType NOTIFY httpCacheTypeChanged FINAL)
    Q_PROPERTY(PersistentCookiesPolicy persistentCookiesPolicy READ persistentCookiesPolicy WRITE setPersistentCookiesPolicy NOTIFY persistentCookiesPolicyChanged FINAL)
    Q_PROPERTY(int httpCacheMaximumSize READ httpCacheMaximumSize WRITE setHttpCacheMaximumSize NOTIFY httpCacheMaximumSizeChanged FINAL)
public:
    QQuickWebEngineProfile();
    ~QQuickWebEngineProfile();

    enum HttpCacheType {
        MemoryHttpCache,
        DiskHttpCache
    };

    enum PersistentCookiesPolicy {
        NoPersistentCookies,
        AllowPersistentCookies,
        ForcePersistentCookies
    };

    QString storageName() const;
    void setStorageName(const QString &name);

    bool isOffTheRecord() const;
    void setOffTheRecord(bool offTheRecord);

    QString persistentStoragePath() const;
    void setPersistentStoragePath(const QString &path);

    QString cachePath() const;
    void setCachePath(const QString &path);

    QString httpUserAgent() const;
    void setHttpUserAgent(const QString &userAgent);

    HttpCacheType httpCacheType() const;
    void setHttpCacheType(QQuickWebEngineProfile::HttpCacheType);

    PersistentCookiesPolicy persistentCookiesPolicy() const;
    void setPersistentCookiesPolicy(QQuickWebEngineProfile::PersistentCookiesPolicy);

    int httpCacheMaximumSize() const;
    void setHttpCacheMaximumSize(int maxSize);

    static QQuickWebEngineProfile *defaultProfile();

signals:
    void storageNameChanged();
    void offTheRecordChanged();
    void persistentStoragePathChanged();
    void cachePathChanged();
    void httpUserAgentChanged();
    void httpCacheTypeChanged();
    void persistentCookiesPolicyChanged();
    void httpCacheMaximumSizeChanged();

    void downloadRequested(QQuickWebEngineDownloadItem *download);
    void downloadFinished(QQuickWebEngineDownloadItem *download);

private:
    Q_DECLARE_PRIVATE(QQuickWebEngineProfile)
    QQuickWebEngineProfile(QQuickWebEngineProfilePrivate *, QObject *parent = 0);
    QQuickWebEngineSettings *settings() const;

    friend class QQuickWebEngineSettings;
    friend class QQuickWebEngineSingleton;
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickWebEngineDownloadItem;
    friend class QQuickWebEngineDownloadItemPrivate;
    QScopedPointer<QQuickWebEngineProfilePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEPROFILE_P_H
