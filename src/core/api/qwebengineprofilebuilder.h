// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPROFILEBUILDER_H
#define QWEBENGINEPROFILEBUILDER_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtWebEngineCore/qwebengineprofile.h>

#include <memory>

QT_BEGIN_NAMESPACE

struct QWebEngineProfileBuilderPrivate;
class QWebEngineProfileBuilder
{
public:
    Q_WEBENGINECORE_EXPORT QWebEngineProfileBuilder();
    Q_WEBENGINECORE_EXPORT ~QWebEngineProfileBuilder();
    Q_WEBENGINECORE_EXPORT QWebEngineProfile *createProfile(const QString &storageName, QObject *parent = nullptr) const;
    Q_WEBENGINECORE_EXPORT static QWebEngineProfile *createOffTheRecordProfile(QObject *parent = nullptr);
    Q_WEBENGINECORE_EXPORT QWebEngineProfileBuilder &setPersistentStoragePath(const QString &path);
    Q_WEBENGINECORE_EXPORT QWebEngineProfileBuilder &setCachePath(const QString &path);
    Q_WEBENGINECORE_EXPORT QWebEngineProfileBuilder &setHttpCacheType(QWebEngineProfile::HttpCacheType httpCacheType);
    Q_WEBENGINECORE_EXPORT QWebEngineProfileBuilder &setPersistentCookiesPolicy(
                                    QWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy);
    Q_WEBENGINECORE_EXPORT QWebEngineProfileBuilder &setHttpCacheMaximumSize(int maxSizeInBytes);
    Q_WEBENGINECORE_EXPORT QWebEngineProfileBuilder &setPersistentPermissionsPolicy(
            QWebEngineProfile::PersistentPermissionsPolicy persistentPermissionPolicy);

private:
    Q_DISABLE_COPY_MOVE(QWebEngineProfileBuilder)
    std::unique_ptr<QWebEngineProfileBuilderPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEPROFILEBUILDER_H
