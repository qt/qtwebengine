// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPROFILE_H
#define QWEBENGINEPROFILE_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstring.h>

#include <functional>
#include <memory>

QT_BEGIN_NAMESPACE

class QUrl;
class QWebEngineClientCertificateStore;
class QWebEngineCookieStore;
class QWebEngineDownloadRequest;
class QWebEngineNotification;
class QWebEngineProfilePrivate;
class QWebEngineSettings;
class QWebEngineScriptCollection;
class QWebEngineUrlRequestInterceptor;
class QWebEngineUrlSchemeHandler;

class Q_WEBENGINECORE_EXPORT QWebEngineProfile : public QObject
{
    Q_OBJECT
public:
    explicit QWebEngineProfile(QObject *parent = nullptr);
    explicit QWebEngineProfile(const QString &name, QObject *parent = nullptr);
    virtual ~QWebEngineProfile();

    enum HttpCacheType {
        MemoryHttpCache,
        DiskHttpCache,
        NoCache
    };
    Q_ENUM(HttpCacheType)

    enum PersistentCookiesPolicy {
        NoPersistentCookies,
        AllowPersistentCookies,
        ForcePersistentCookies
    };
    Q_ENUM(PersistentCookiesPolicy)

    QString storageName() const;
    bool isOffTheRecord() const;

    QString persistentStoragePath() const;
    void setPersistentStoragePath(const QString &path);

    QString cachePath() const;
    void setCachePath(const QString &path);

    QString httpUserAgent() const;
    void setHttpUserAgent(const QString &userAgent);

    HttpCacheType httpCacheType() const;
    void setHttpCacheType(QWebEngineProfile::HttpCacheType);

    void setHttpAcceptLanguage(const QString &httpAcceptLanguage);
    QString httpAcceptLanguage() const;

    PersistentCookiesPolicy persistentCookiesPolicy() const;
    void setPersistentCookiesPolicy(QWebEngineProfile::PersistentCookiesPolicy);

    int httpCacheMaximumSize() const;
    void setHttpCacheMaximumSize(int maxSize);

    QWebEngineCookieStore *cookieStore();
    void setUrlRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);

    void clearAllVisitedLinks();
    void clearVisitedLinks(const QList<QUrl> &urls);
    bool visitedLinksContainsUrl(const QUrl &url) const;

    QWebEngineSettings *settings() const;
    QWebEngineScriptCollection *scripts() const;

    const QWebEngineUrlSchemeHandler *urlSchemeHandler(const QByteArray &) const;
    void installUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *);
    void removeUrlScheme(const QByteArray &scheme);
    void removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *);
    void removeAllUrlSchemeHandlers();

    void clearHttpCache();

    void setSpellCheckLanguages(const QStringList &languages);
    QStringList spellCheckLanguages() const;
    void setSpellCheckEnabled(bool enabled);
    bool isSpellCheckEnabled() const;

    QString downloadPath() const;
    void setDownloadPath(const QString &path);

    bool isPushServiceEnabled() const;
    void setPushServiceEnabled(bool enabled);

    void setNotificationPresenter(std::function<void(std::unique_ptr<QWebEngineNotification>)> notificationPresenter);

    QWebEngineClientCertificateStore *clientCertificateStore();

    void requestIconForPageURL(const QUrl &url, int desiredSizeInPixel, std::function<void(const QIcon &, const QUrl &, const QUrl &)> iconAvailableCallback) const;
    void requestIconForIconURL(const QUrl &url, int desiredSizeInPixel, std::function<void(const QIcon &, const QUrl &)> iconAvailableCallback) const;

    static QWebEngineProfile *defaultProfile();

Q_SIGNALS:
    void downloadRequested(QWebEngineDownloadRequest *download);

private:
    Q_DISABLE_COPY(QWebEngineProfile)
    Q_DECLARE_PRIVATE(QWebEngineProfile)
    QWebEngineProfile(QWebEngineProfilePrivate *, QObject *parent = nullptr);

    friend class QWebEngineView;
    std::function<void(std::unique_ptr<QWebEngineNotification>)> notificationPresenter();

    friend class QWebEnginePagePrivate;
    QScopedPointer<QWebEngineProfilePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEPROFILE_H
