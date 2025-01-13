// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QQUICKWEBENGINEPROFILEPROTOTYPE_H
#define QQUICKWEBENGINEPROFILEPROTOTYPE_H

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

#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QtQml/qqmlparserstatus.h>
#include <QtWebEngineQuick/QQuickWebEngineProfile>

QT_BEGIN_NAMESPACE

struct QQuickWebEngineProfilePrototypePrivate;

class Q_WEBENGINEQUICK_EXPORT QQuickWebEngineProfilePrototype : public QObject,
                                                                public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString storageName READ storageName WRITE setStorageName FINAL)
    Q_PROPERTY(QString persistentStoragePath READ persistentStoragePath WRITE
                       setPersistentStoragePath FINAL)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath FINAL)
    Q_PROPERTY(QQuickWebEngineProfile::HttpCacheType httpCacheType READ httpCacheType WRITE
                       setHttpCacheType FINAL)
    Q_PROPERTY(QQuickWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy READ
                       persistentCookiesPolicy WRITE setPersistentCookiesPolicy FINAL)
    Q_PROPERTY(int httpCacheMaximumSize READ httpCacheMaximumSize WRITE setHttpCacheMaximumSize FINAL)
    Q_PROPERTY(QQuickWebEngineProfile::PersistentPermissionsPolicy persistentPermissionsPolicy READ
                       persistentPermissionsPolicy WRITE setPersistentPermissionsPolicy FINAL)
    QML_NAMED_ELEMENT(WebEngineProfilePrototype)
    QML_ADDED_IN_VERSION(6, 9)

public:
    explicit QQuickWebEngineProfilePrototype(QObject *parent = nullptr);
    ~QQuickWebEngineProfilePrototype();

    QString storageName() const;
    void setStorageName(const QString &storageName);

    QString persistentStoragePath() const;
    void setPersistentStoragePath(const QString &path);

    QString cachePath() const;
    void setCachePath(const QString &path);

    QQuickWebEngineProfile::HttpCacheType httpCacheType() const;
    void setHttpCacheType(QQuickWebEngineProfile::HttpCacheType httpCacheType);

    QQuickWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy() const;
    void setPersistentCookiesPolicy(
            QQuickWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy);

    void setHttpCacheMaximumSize(int maxSizeInBytes);
    int httpCacheMaximumSize() const;

    QQuickWebEngineProfile::PersistentPermissionsPolicy persistentPermissionsPolicy() const;
    void setPersistentPermissionsPolicy(
            QQuickWebEngineProfile::PersistentPermissionsPolicy persistentPermissionsPolicy);

    Q_INVOKABLE QQuickWebEngineProfile *instance();

protected:
    void componentComplete() override;
    void classBegin() override;

private:
    std::unique_ptr<QQuickWebEngineProfilePrototypePrivate> d_ptr;
};
QT_END_NAMESPACE

#endif // QQUICKWEBENGINEPROFILEPROTOTYPE_H
