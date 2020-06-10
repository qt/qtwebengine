/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "cookie_monster_delegate_qt.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/task/post_task.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "net/cookies/cookie_util.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

#include "api/qwebenginecookiestore.h"
#include "api/qwebenginecookiestore_p.h"
#include "type_conversion.h"

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

    DISALLOW_COPY_AND_ASSIGN(CookieChangeListener);
};

class CookieAccessFilter : public network::mojom::CookieRemoteAccessFilter
{
public:
    CookieAccessFilter(CookieMonsterDelegateQt *delegate) : m_delegate(delegate) { }
    ~CookieAccessFilter() override = default;

    void AllowedAccess(const GURL &url, const net::SiteForCookies &site_for_cookies, AllowedAccessCallback callback) override
    {
        bool allow = m_delegate->canGetCookies(toQt(site_for_cookies.RepresentativeUrl()), toQt(url));
        std::move(callback).Run(allow);
    }

private:
    CookieMonsterDelegateQt *m_delegate;

    DISALLOW_COPY_AND_ASSIGN(CookieAccessFilter);
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

void CookieMonsterDelegateQt::getAllCookies(quint64 callbackId)
{
    m_mojoCookieManager->GetAllCookies(base::BindOnce(&CookieMonsterDelegateQt::GetAllCookiesCallbackOnUIThread, this, callbackId));
}

void CookieMonsterDelegateQt::setCookie(quint64 callbackId, const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    net::CookieStore::SetCookiesCallback callback;

    GURL gurl = origin.isEmpty() ? sourceUrlForCookie(cookie) : toGurl(origin);
    std::string cookie_line = cookie.toRawForm().toStdString();

    if (callbackId != CallbackDirectory::NoCallbackId)
        callback = base::BindOnce(&CookieMonsterDelegateQt::SetCookieCallbackOnUIThread, this, callbackId);
    net::CanonicalCookie::CookieInclusionStatus inclusion;
    auto canonCookie = net::CanonicalCookie::Create(gurl, cookie_line, base::Time::Now(), base::nullopt, &inclusion);
    if (!inclusion.IsInclude()) {
        LOG(WARNING) << "QWebEngineCookieStore::setCookie() - Tried to set invalid cookie";
        return;
    }
    net::CookieOptions options;
    options.set_include_httponly();
    m_mojoCookieManager->SetCanonicalCookie(*canonCookie.get(), gurl.scheme(), options, std::move(callback));
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

void CookieMonsterDelegateQt::deleteSessionCookies(quint64 callbackId)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    network::mojom::CookieManager::DeleteCookiesCallback callback =
        base::BindOnce(&CookieMonsterDelegateQt::DeleteCookiesCallbackOnUIThread, this, callbackId);
    auto filter = network::mojom::CookieDeletionFilter::New();
    filter->session_control = network::mojom::CookieDeletionSessionControl::SESSION_COOKIES;
    m_mojoCookieManager->DeleteCookies(std::move(filter), std::move(callback));
}

void CookieMonsterDelegateQt::deleteAllCookies(quint64 callbackId)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    network::mojom::CookieManager::DeleteCookiesCallback callback =
        base::BindOnce(&CookieMonsterDelegateQt::DeleteCookiesCallbackOnUIThread, this, callbackId);
    auto filter = network::mojom::CookieDeletionFilter::New();
    m_mojoCookieManager->DeleteCookies(std::move(filter), std::move(callback));
}

void CookieMonsterDelegateQt::setMojoCookieManager(network::mojom::CookieManagerPtrInfo cookie_manager_info)
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

void CookieMonsterDelegateQt::GetAllCookiesCallbackOnUIThread(qint64 callbackId, const net::CookieList &cookies)
{
    QByteArray rawCookies = QByteArray::fromStdString(net::CanonicalCookie::BuildCookieLine(cookies));
    if (m_client)
        m_client->d_func()->onGetAllCallbackResult(callbackId, rawCookies);
}

void CookieMonsterDelegateQt::SetCookieCallbackOnUIThread(qint64 callbackId, net::CanonicalCookie::CookieInclusionStatus status)
{
    if (m_client)
        m_client->d_func()->onSetCallbackResult(callbackId, status.IsInclude());
}

void CookieMonsterDelegateQt::DeleteCookiesCallbackOnUIThread(qint64 callbackId, uint numCookies)
{
    if (m_client)
        m_client->d_func()->onDeleteCallbackResult(callbackId, numCookies);
}

} // namespace QtWebEngineCore
