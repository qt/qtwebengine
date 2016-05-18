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

#ifndef QQUICKWEBENGINEPROFILE_H
#define QQUICKWEBENGINEPROFILE_H


#include <QtWebEngine/qtwebengineglobal.h>

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>

namespace QtWebEngineCore {
class BrowserContextAdapter;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineDownloadItem;
class QQuickWebEngineProfilePrivate;
class QQuickWebEngineSettings;
class QWebEngineCookieStore;
class QWebEngineUrlRequestInterceptor;
class QWebEngineUrlSchemeHandler;

class Q_WEBENGINE_EXPORT QQuickWebEngineProfile : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString storageName READ storageName WRITE setStorageName NOTIFY storageNameChanged FINAL)
    Q_PROPERTY(bool offTheRecord READ isOffTheRecord WRITE setOffTheRecord NOTIFY offTheRecordChanged FINAL)
    Q_PROPERTY(QString persistentStoragePath READ persistentStoragePath WRITE setPersistentStoragePath NOTIFY persistentStoragePathChanged FINAL)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged FINAL)
    Q_PROPERTY(QString httpUserAgent READ httpUserAgent WRITE setHttpUserAgent NOTIFY httpUserAgentChanged FINAL)
    Q_PROPERTY(HttpCacheType httpCacheType READ httpCacheType WRITE setHttpCacheType NOTIFY httpCacheTypeChanged FINAL)
    Q_PROPERTY(QString httpAcceptLanguage READ httpAcceptLanguage WRITE setHttpAcceptLanguage NOTIFY httpAcceptLanguageChanged FINAL REVISION 1)
    Q_PROPERTY(PersistentCookiesPolicy persistentCookiesPolicy READ persistentCookiesPolicy WRITE setPersistentCookiesPolicy NOTIFY persistentCookiesPolicyChanged FINAL)
    Q_PROPERTY(int httpCacheMaximumSize READ httpCacheMaximumSize WRITE setHttpCacheMaximumSize NOTIFY httpCacheMaximumSizeChanged FINAL)
public:
    QQuickWebEngineProfile(QObject *parent = Q_NULLPTR);
    ~QQuickWebEngineProfile();

    enum HttpCacheType {
        MemoryHttpCache,
        DiskHttpCache
    };
    Q_ENUM(HttpCacheType)

    enum PersistentCookiesPolicy {
        NoPersistentCookies,
        AllowPersistentCookies,
        ForcePersistentCookies
    };
    Q_ENUM(PersistentCookiesPolicy)

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

    QString httpAcceptLanguage() const;
    void setHttpAcceptLanguage(const QString &httpAcceptLanguage);

    QWebEngineCookieStore *cookieStore() const;

    void setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);

    const QWebEngineUrlSchemeHandler *urlSchemeHandler(const QByteArray &) const;
    void installUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *);
    void removeUrlScheme(const QByteArray &scheme);
    void removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *);
    void removeAllUrlSchemeHandlers();

    static QQuickWebEngineProfile *defaultProfile();

Q_SIGNALS:
    void storageNameChanged();
    void offTheRecordChanged();
    void persistentStoragePathChanged();
    void cachePathChanged();
    void httpUserAgentChanged();
    void httpCacheTypeChanged();
    void persistentCookiesPolicyChanged();
    void httpCacheMaximumSizeChanged();
    Q_REVISION(1) void httpAcceptLanguageChanged();

    void downloadRequested(QQuickWebEngineDownloadItem *download);
    void downloadFinished(QQuickWebEngineDownloadItem *download);

private Q_SLOTS:
    void destroyedUrlSchemeHandler(QWebEngineUrlSchemeHandler *obj);

private:
    Q_DECLARE_PRIVATE(QQuickWebEngineProfile)
    QQuickWebEngineProfile(QQuickWebEngineProfilePrivate *, QObject *parent = Q_NULLPTR);
    QQuickWebEngineSettings *settings() const;

    friend class QQuickWebEngineSettings;
    friend class QQuickWebEngineSingleton;
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickWebEngineDownloadItem;
    friend class QQuickWebEngineDownloadItemPrivate;
    QScopedPointer<QQuickWebEngineProfilePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEPROFILE_H
