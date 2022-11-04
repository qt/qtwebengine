// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef COOKIE_MONSTER_DELEGATE_QT_H
#define COOKIE_MONSTER_DELEGATE_QT_H

#include "qtwebenginecoreglobal_p.h"

// We need to work around Chromium using 'signals' as a variable name in headers:
#ifdef signals
#define StAsH_signals signals
#undef signals
#endif
#include "base/memory/ref_counted.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "net/cookies/cookie_store.h"
#include "services/network/public/mojom/cookie_manager.mojom-forward.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#ifdef StAsH_signals
#define signals StAsH_signals
#undef StAsH_signals
#endif

#include <QPointer>

QT_FORWARD_DECLARE_CLASS(QNetworkCookie)
QT_FORWARD_DECLARE_CLASS(QWebEngineCookieStore)

namespace QtWebEngineCore {

class CookieMonsterDelegateQtPrivate;

class Q_WEBENGINECORE_PRIVATE_EXPORT CookieMonsterDelegateQt : public base::RefCountedThreadSafe<CookieMonsterDelegateQt>
{
    QPointer<QWebEngineCookieStore> m_client;
    std::vector<std::unique_ptr<net::CookieChangeSubscription>> m_subscriptions;

    mojo::Remote<network::mojom::CookieManager> m_mojoCookieManager;
    std::unique_ptr<network::mojom::CookieChangeListener> m_listener;
    std::unique_ptr<network::mojom::CookieRemoteAccessFilter> m_filter;
    mojo::Receiver<network::mojom::CookieChangeListener> m_receiver;
    mojo::Receiver<network::mojom::CookieRemoteAccessFilter> m_filterReceiver;
    bool m_hasFilter;
public:
    CookieMonsterDelegateQt();
    ~CookieMonsterDelegateQt();

    bool hasCookieMonster();

    void setCookie(const QNetworkCookie &cookie, const QUrl &origin);
    void deleteCookie(const QNetworkCookie &cookie, const QUrl &origin);
    void getAllCookies();
    void deleteSessionCookies();
    void deleteAllCookies();

    void setClient(QWebEngineCookieStore *client);
    void setMojoCookieManager(mojo::PendingRemote<network::mojom::CookieManager> cookie_manager_info);
    void unsetMojoCookieManager();
    void setHasFilter(bool b);

    bool canSetCookie(const QUrl &firstPartyUrl, const QByteArray &cookieLine, const QUrl &url) const;
    bool canGetCookies(const QUrl &firstPartyUrl, const QUrl &url) const;

    void AddStore(net::CookieStore *store);
    void OnCookieChanged(const net::CookieChangeInfo &change);
};

} // namespace QtWebEngineCore

#endif // COOKIE_MONSTER_DELEGATE_QT_H
