// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEPROFILE_H
#define QQUICKWEBENGINEPROFILE_H

#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QtWebEngineCore/qwebenginepermission.h>
#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstring.h>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineDownloadRequest;
class QQuickWebEngineSettings;
class QWebEngineClientCertificateStore;
class QWebEngineClientHints;
class QWebEngineCookieStore;
class QWebEngineNotification;
class QWebEngineUrlRequestInterceptor;
class QWebEngineUrlSchemeHandler;
class QQuickWebEngineScriptCollection;
class QQuickWebEngineProfilePrivate;

class Q_WEBENGINEQUICK_EXPORT QQuickWebEngineProfile : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString storageName READ storageName WRITE setStorageName NOTIFY storageNameChanged FINAL)
    Q_PROPERTY(bool offTheRecord READ isOffTheRecord WRITE setOffTheRecord NOTIFY offTheRecordChanged FINAL)
    Q_PROPERTY(QString persistentStoragePath READ persistentStoragePath WRITE setPersistentStoragePath NOTIFY persistentStoragePathChanged FINAL)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged FINAL)
    Q_PROPERTY(QString httpUserAgent READ httpUserAgent WRITE setHttpUserAgent NOTIFY httpUserAgentChanged FINAL)
    Q_PROPERTY(HttpCacheType httpCacheType READ httpCacheType WRITE setHttpCacheType NOTIFY httpCacheTypeChanged FINAL)
    Q_PROPERTY(QString httpAcceptLanguage READ httpAcceptLanguage WRITE setHttpAcceptLanguage NOTIFY httpAcceptLanguageChanged FINAL REVISION(1,1))
    Q_PROPERTY(PersistentCookiesPolicy persistentCookiesPolicy READ persistentCookiesPolicy WRITE setPersistentCookiesPolicy NOTIFY persistentCookiesPolicyChanged FINAL)
    Q_PROPERTY(PersistentPermissionsPolicy persistentPermissionsPolicy READ persistentPermissionsPolicy WRITE setPersistentPermissionsPolicy NOTIFY persistentPermissionsPolicyChanged FINAL REVISION(6,8))
    Q_PROPERTY(int httpCacheMaximumSize READ httpCacheMaximumSize WRITE setHttpCacheMaximumSize NOTIFY httpCacheMaximumSizeChanged FINAL)
    Q_PROPERTY(QStringList spellCheckLanguages READ spellCheckLanguages WRITE setSpellCheckLanguages NOTIFY spellCheckLanguagesChanged FINAL REVISION(1,3))
    Q_PROPERTY(bool spellCheckEnabled READ isSpellCheckEnabled WRITE setSpellCheckEnabled NOTIFY spellCheckEnabledChanged FINAL REVISION(1,3))
    Q_PROPERTY(QQuickWebEngineScriptCollection *userScripts READ userScripts)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged FINAL REVISION(1,5))
    Q_PROPERTY(bool isPushServiceEnabled READ isPushServiceEnabled WRITE setPushServiceEnabled NOTIFY pushServiceEnabledChanged FINAL REVISION(6,5))
    Q_PROPERTY(QWebEngineClientHints *clientHints READ clientHints FINAL REVISION(6,8))
    QML_NAMED_ELEMENT(WebEngineProfile)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)

public:
    QQuickWebEngineProfile(QObject *parent = nullptr);
    ~QQuickWebEngineProfile();

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

    enum class PersistentPermissionsPolicy : quint8 {
        AskEveryTime = 0,
        StoreInMemory,
        StoreOnDisk,
    };
    Q_ENUM(PersistentPermissionsPolicy)

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

    PersistentPermissionsPolicy persistentPermissionsPolicy() const;
    void setPersistentPermissionsPolicy(QQuickWebEngineProfile::PersistentPermissionsPolicy);

    int httpCacheMaximumSize() const;
    void setHttpCacheMaximumSize(int maxSize);

    QString httpAcceptLanguage() const;
    void setHttpAcceptLanguage(const QString &httpAcceptLanguage);

    QWebEngineCookieStore *cookieStore() const;

    void setUrlRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);

    const QWebEngineUrlSchemeHandler *urlSchemeHandler(const QByteArray &) const;
    void installUrlSchemeHandler(const QByteArray &scheme, QWebEngineUrlSchemeHandler *);
    void removeUrlScheme(const QByteArray &scheme);
    void removeUrlSchemeHandler(QWebEngineUrlSchemeHandler *);
    void removeAllUrlSchemeHandlers();

    Q_REVISION(1,2) Q_INVOKABLE void clearHttpCache();

    void setSpellCheckLanguages(const QStringList &languages);
    QStringList spellCheckLanguages() const;
    void setSpellCheckEnabled(bool enabled);
    bool isSpellCheckEnabled() const;

    QQuickWebEngineScriptCollection *userScripts() const;

    QString downloadPath() const;
    void setDownloadPath(const QString &path);

    bool isPushServiceEnabled() const;
    void setPushServiceEnabled(bool enable);

    QWebEngineClientCertificateStore *clientCertificateStore();
    QWebEngineClientHints *clientHints() const;

    Q_REVISION(6,8) Q_INVOKABLE QWebEnginePermission queryPermission(const QUrl &securityOrigin, QWebEnginePermission::PermissionType permissionType) const;
    Q_REVISION(6,8) Q_INVOKABLE QList<QWebEnginePermission> listAllPermissions() const;
    Q_REVISION(6,8) Q_INVOKABLE QList<QWebEnginePermission> listPermissionsForOrigin(const QUrl &securityOrigin) const;
    Q_REVISION(6,8) Q_INVOKABLE QList<QWebEnginePermission> listPermissionsForPermissionType(QWebEnginePermission::PermissionType permissionType) const;

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
    Q_REVISION(1,1) void httpAcceptLanguageChanged();
    Q_REVISION(1,3) void spellCheckLanguagesChanged();
    Q_REVISION(1,3) void spellCheckEnabledChanged();
    Q_REVISION(1,5) void downloadPathChanged();
    Q_REVISION(6,5) void pushServiceEnabledChanged();
    Q_REVISION(6,7) void clearHttpCacheCompleted();
    Q_REVISION(6,8) void persistentPermissionsPolicyChanged();
    void downloadRequested(QQuickWebEngineDownloadRequest *download);
    void downloadFinished(QQuickWebEngineDownloadRequest *download);

    Q_REVISION(1,5) void presentNotification(QWebEngineNotification *notification);

private:
    Q_DECLARE_PRIVATE(QQuickWebEngineProfile)
    QQuickWebEngineProfile(QQuickWebEngineProfilePrivate *, QObject *parent = nullptr);
    QQuickWebEngineSettings *settings() const;
    void ensureQmlContext(const QObject *object);

    friend class FaviconImageRequester;
    friend class QQuickWebEngineSingleton;
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickWebEngineView;
    QScopedPointer<QQuickWebEngineProfilePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEPROFILE_H
