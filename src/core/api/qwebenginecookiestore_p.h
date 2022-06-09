// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECOOKIESTORE_P_H
#define QWEBENGINECOOKIESTORE_P_H

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

#include "qtwebenginecoreglobal_p.h"

#include "qwebenginecookiestore.h"

#include <QList>
#include <QNetworkCookie>
#include <QUrl>

namespace QtWebEngineCore {
class CookieMonsterDelegateQt;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineCookieStorePrivate
{
    Q_DECLARE_PUBLIC(QWebEngineCookieStore)
    struct CookieData {
        bool wasDelete;
        QNetworkCookie cookie;
        QUrl origin;
    };
    friend class QTypeInfo<CookieData>;
    QWebEngineCookieStore *q_ptr;

public:
    std::function<bool(const QWebEngineCookieStore::FilterRequest &)> filterCallback;
    QList<CookieData> m_pendingUserCookies;
    bool m_deleteSessionCookiesPending;
    bool m_deleteAllCookiesPending;
    bool m_getAllCookiesPending;

    QtWebEngineCore::CookieMonsterDelegateQt *delegate;

    QWebEngineCookieStorePrivate(QWebEngineCookieStore *q);

    void processPendingUserCookies();
    void rejectPendingUserCookies();
    void setCookie(const QNetworkCookie &cookie, const QUrl &origin);
    void deleteCookie(const QNetworkCookie &cookie, const QUrl &url);
    void deleteSessionCookies();
    void deleteAllCookies();
    void getAllCookies();

    bool canAccessCookies(const QUrl &firstPartyUrl, const QUrl &url) const;

    void onCookieChanged(const QNetworkCookie &cookie, bool removed);
};

Q_DECLARE_TYPEINFO(QWebEngineCookieStorePrivate::CookieData, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

#endif // QWEBENGINECOOKIESTORE_P_H
