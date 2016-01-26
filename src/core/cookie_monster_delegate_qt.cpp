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
#include "content/public/browser/browser_thread.h"
#include "net/cookies/cookie_util.h"

#include "api/qwebenginecookiestore.h"
#include "api/qwebenginecookiestore_p.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

static GURL sourceUrlForCookie(const QNetworkCookie &cookie) {
    QString urlFragment = QStringLiteral("%1%2").arg(cookie.domain()).arg(cookie.path());
    return net::cookie_util::CookieOriginToURL(urlFragment.toStdString(), /* is_https */ cookie.isSecure());
}

static void onSetCookieCallback(QWebEngineCookieStorePrivate *client, qint64 callbackId, bool success) {
    client->onSetCallbackResult(callbackId, success);
}

static void onDeleteCookiesCallback(QWebEngineCookieStorePrivate *client, qint64 callbackId, int numCookies) {
    client->onDeleteCallbackResult(callbackId, numCookies);
}

static void onGetAllCookiesCallback(QWebEngineCookieStorePrivate *client, qint64 callbackId, const net::CookieList& cookies) {
    QByteArray rawCookies;
    for (auto&& cookie: cookies)
        rawCookies += toQt(cookie).toRawForm() % QByteArrayLiteral("\n");

    client->onGetAllCallbackResult(callbackId, rawCookies);
}

CookieMonsterDelegateQt::CookieMonsterDelegateQt()
    : m_client(0)
    , m_cookieMonster(0)
{

}

CookieMonsterDelegateQt::~CookieMonsterDelegateQt()
{

}

bool CookieMonsterDelegateQt::hasCookieMonster()
{
    return m_cookieMonster.get();
}

void CookieMonsterDelegateQt::getAllCookies(quint64 callbackId)
{
    net::CookieMonster::GetCookieListCallback callback = base::Bind(&onGetAllCookiesCallback, m_client->d_func(), callbackId);
    m_cookieMonster->GetAllCookiesAsync(callback);
}

void CookieMonsterDelegateQt::setCookie(quint64 callbackId, const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    net::CookieStore::SetCookiesCallback callback;
    if (callbackId != CallbackDirectory::NoCallbackId)
        callback = base::Bind(&onSetCookieCallback, m_client->d_func(), callbackId);

    net::CookieOptions options;
    options.set_include_httponly();

    GURL gurl = origin.isEmpty() ? sourceUrlForCookie(cookie) : toGurl(origin);

    m_cookieMonster->SetCookieWithOptionsAsync(gurl, cookie.toRawForm().toStdString(), options, callback);
}

void CookieMonsterDelegateQt::deleteCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    GURL gurl = origin.isEmpty() ? sourceUrlForCookie(cookie) : toGurl(origin);

    m_cookieMonster->DeleteCookieAsync(gurl, cookie.name().toStdString(), base::Closure());
}

void CookieMonsterDelegateQt::deleteSessionCookies(quint64 callbackId)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    net::CookieMonster::DeleteCallback callback = base::Bind(&onDeleteCookiesCallback, m_client->d_func(), callbackId);
    m_cookieMonster->DeleteSessionCookiesAsync(callback);
}

void CookieMonsterDelegateQt::deleteAllCookies(quint64 callbackId)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    net::CookieMonster::DeleteCallback callback = base::Bind(&onDeleteCookiesCallback, m_client->d_func(), callbackId);
    m_cookieMonster->DeleteAllAsync(callback);
}

void CookieMonsterDelegateQt::setCookieMonster(net::CookieMonster* monster)
{
    if (!monster && !m_cookieMonster)
        return;

    m_cookieMonster = monster;

    if (!m_client)
        return;

    if (monster)
        m_client->d_func()->processPendingUserCookies();
    else
        m_client->d_func()->rejectPendingUserCookies();
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

bool CookieMonsterDelegateQt::canSetCookie(const QUrl &firstPartyUrl, const QByteArray &cookieLine, const QUrl &url)
{
    // TODO: should be used for FilterRequest implementation
    return true;
}

void CookieMonsterDelegateQt::OnCookieChanged(const net::CanonicalCookie& cookie, bool removed, ChangeCause cause)
{
    if (!m_client)
        return;
    m_client->d_func()->onCookieChanged(toQt(cookie), removed);
}

}
