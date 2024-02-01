// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "cookie_monster_delegate_qt.h"

#include "base/functional/bind.h"
#include "net/cookies/cookie_util.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

#include "api/qwebenginecookiestore.h"
#include "api/qwebenginecookiestore_p.h"
#include "type_conversion.h"

#include <QNetworkCookie>

namespace QtWebEngineCore {

class CookieChangeListener : public network::mojom::CookieChangeListener
{
public:
    CookieChangeListener(CookieMonsterDelegateQt *delegate) : m_delegate(delegate) { }
    ~CookieChangeListener() override = default;

    // network::mojom::CookieChangeListener:
    void OnCookieChange(const net::CookieChangeInfo &change) override
    {
        m_delegate->OnCookieChanged(change);
    }

private:
    CookieMonsterDelegateQt *m_delegate;
};

class CookieAccessFilter : public network::mojom::CookieRemoteAccessFilter
{
public:
    CookieAccessFilter(CookieMonsterDelegateQt *delegate) : m_delegate(delegate) { }
    ~CookieAccessFilter() override = default;

    void AllowedAccess(const GURL &url, const net::SiteForCookies &site_for_cookies, AllowedAccessCallback callback) override
    {
        bool allow = m_delegate->canGetCookies(toQt(site_for_cookies.first_party_url()), toQt(url));
        std::move(callback).Run(allow);
    }

private:
    CookieMonsterDelegateQt *m_delegate;
};


static GURL sourceUrlForCookie(const QNetworkCookie &cookie)
{
    QString urlFragment = QStringLiteral("%1%2").arg(cookie.domain()).arg(cookie.path());
    return net::cookie_util::CookieOriginToURL(urlFragment.toStdString(), /* is_https */ cookie.isSecure());
}

CookieMonsterDelegateQt::CookieMonsterDelegateQt()
    : m_client(nullptr)
    , m_listener(new CookieChangeListener(this))
    , m_filter(new CookieAccessFilter(this))
    , m_receiver(m_listener.get())
    , m_filterReceiver(m_filter.get())
    , m_hasFilter(false)
{
}

CookieMonsterDelegateQt::~CookieMonsterDelegateQt()
{
}

void CookieMonsterDelegateQt::AddStore(net::CookieStore *store)
{
    std::unique_ptr<net::CookieChangeSubscription> sub = store->GetChangeDispatcher().AddCallbackForAllChanges(
            base::BindRepeating(&CookieMonsterDelegateQt::OnCookieChanged,
                                // this object's destruction will deregister the subscription.
                                base::Unretained(this)));

    m_subscriptions.push_back(std::move(sub));
}

bool CookieMonsterDelegateQt::hasCookieMonster()
{
    return m_mojoCookieManager.is_bound();
}

void CookieMonsterDelegateQt::getAllCookies()
{
    m_mojoCookieManager->GetAllCookies(net::CookieStore::GetAllCookiesCallback());
}

void CookieMonsterDelegateQt::setCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    GURL gurl = origin.isEmpty() ? sourceUrlForCookie(cookie) : toGurl(origin);
    std::string cookie_line = cookie.toRawForm().toStdString();

    net::CookieInclusionStatus inclusion;
    auto canonCookie = net::CanonicalCookie::Create(gurl, cookie_line, base::Time::Now(), absl::nullopt, absl::nullopt, &inclusion);
    if (!inclusion.IsInclude()) {
        LOG(WARNING) << "QWebEngineCookieStore::setCookie() - Tried to set invalid cookie";
        return;
    }
    net::CookieOptions options;
    options.set_include_httponly();
    options.set_same_site_cookie_context(net::CookieOptions::SameSiteCookieContext::MakeInclusiveForSet());
    m_mojoCookieManager->SetCanonicalCookie(*canonCookie.get(), gurl, options, net::CookieStore::SetCookiesCallback());
}

void CookieMonsterDelegateQt::deleteCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    GURL gurl = origin.isEmpty() ? sourceUrlForCookie(cookie) : toGurl(origin);
    std::string cookie_name = cookie.name().toStdString();
    auto filter = network::mojom::CookieDeletionFilter::New();
    filter->url = gurl;
    filter->cookie_name = cookie_name;
    m_mojoCookieManager->DeleteCookies(std::move(filter), network::mojom::CookieManager::DeleteCookiesCallback());
}

void CookieMonsterDelegateQt::deleteSessionCookies()
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    auto filter = network::mojom::CookieDeletionFilter::New();
    filter->session_control = network::mojom::CookieDeletionSessionControl::SESSION_COOKIES;
    m_mojoCookieManager->DeleteCookies(std::move(filter), network::mojom::CookieManager::DeleteCookiesCallback());
}

void CookieMonsterDelegateQt::deleteAllCookies()
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    auto filter = network::mojom::CookieDeletionFilter::New();
    m_mojoCookieManager->DeleteCookies(std::move(filter), network::mojom::CookieManager::DeleteCookiesCallback());
}

void CookieMonsterDelegateQt::setMojoCookieManager(mojo::PendingRemote<network::mojom::CookieManager> cookie_manager_info)
{
    if (m_mojoCookieManager.is_bound())
        unsetMojoCookieManager();

    Q_ASSERT(!m_mojoCookieManager.is_bound());
    Q_ASSERT(!m_receiver.is_bound());

    m_mojoCookieManager.Bind(std::move(cookie_manager_info));

    m_mojoCookieManager->AddGlobalChangeListener(m_receiver.BindNewPipeAndPassRemote());
    if (m_hasFilter)
        m_mojoCookieManager->SetRemoteFilter(m_filterReceiver.BindNewPipeAndPassRemote());

    if (m_client)
        m_client->d_func()->processPendingUserCookies();
}

void CookieMonsterDelegateQt::setHasFilter(bool hasFilter)
{
    m_hasFilter = hasFilter;
    if (!m_mojoCookieManager.is_bound())
        return;
    if (m_hasFilter) {
        if (!m_filterReceiver.is_bound())
            m_mojoCookieManager->SetRemoteFilter(m_filterReceiver.BindNewPipeAndPassRemote());
    } else {
        if (m_filterReceiver.is_bound())
            m_filterReceiver.reset();
    }
}

void CookieMonsterDelegateQt::unsetMojoCookieManager()
{
    m_receiver.reset();
    m_filterReceiver.reset();
    m_mojoCookieManager.reset();
}

void CookieMonsterDelegateQt::setClient(QWebEngineCookieStore *client)
{
    m_client = client;

    if (!m_client)
        return;

    m_client->d_func()->delegate = this;

    if (hasCookieMonster())
        m_client->d_func()->processPendingUserCookies();
}

bool CookieMonsterDelegateQt::canSetCookie(const QUrl &firstPartyUrl, const QByteArray &/*cookieLine*/, const QUrl &url) const
{
    if (!m_client)
        return true;

    return m_client->d_func()->canAccessCookies(firstPartyUrl, url);
}

bool CookieMonsterDelegateQt::canGetCookies(const QUrl &firstPartyUrl, const QUrl &url) const
{
    if (!m_client)
        return true;

    return m_client->d_func()->canAccessCookies(firstPartyUrl, url);
}

void CookieMonsterDelegateQt::OnCookieChanged(const net::CookieChangeInfo &change)
{
    if (!m_client)
        return;
    m_client->d_func()->onCookieChanged(toQt(change.cookie), change.cause != net::CookieChangeCause::INSERTED);
}

} // namespace QtWebEngineCore
