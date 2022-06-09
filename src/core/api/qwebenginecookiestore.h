// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECOOKIESTORE_H
#define QWEBENGINECOOKIESTORE_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkcookie.h>

#include <functional>

namespace QtWebEngineCore {
class ContentBrowserClientQt;
class CookieMonsterDelegateQt;
class ProfileAdapter;
} // namespace QtWebEngineCore

QT_BEGIN_NAMESPACE

class QWebEngineCookieStorePrivate;
class Q_WEBENGINECORE_EXPORT QWebEngineCookieStore : public QObject
{
    Q_OBJECT

public:
    struct FilterRequest {
        QUrl firstPartyUrl;
        QUrl origin;
        bool thirdParty;
        bool _reservedFlag;
        ushort _reservedType;
    };
    virtual ~QWebEngineCookieStore();

    void setCookieFilter(const std::function<bool(const FilterRequest &)> &filterCallback);
    void setCookieFilter(std::function<bool(const FilterRequest &)> &&filterCallback);
    void setCookie(const QNetworkCookie &cookie, const QUrl &origin = QUrl());
    void deleteCookie(const QNetworkCookie &cookie, const QUrl &origin = QUrl());
    void deleteSessionCookies();
    void deleteAllCookies();
    void loadAllCookies();

Q_SIGNALS:
    void cookieAdded(const QNetworkCookie &cookie);
    void cookieRemoved(const QNetworkCookie &cookie);

private:
    explicit QWebEngineCookieStore(QObject *parent = nullptr);
    friend class QtWebEngineCore::ContentBrowserClientQt;
    friend class QtWebEngineCore::CookieMonsterDelegateQt;
    friend class QtWebEngineCore::ProfileAdapter;
    Q_DISABLE_COPY(QWebEngineCookieStore)
    Q_DECLARE_PRIVATE(QWebEngineCookieStore)
    QScopedPointer<QWebEngineCookieStorePrivate> d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QWebEngineCookieStore *)

#endif // QWEBENGINECOOKIESTORE_H
