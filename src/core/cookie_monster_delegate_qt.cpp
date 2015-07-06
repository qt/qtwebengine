/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "cookie_monster_delegate_qt.h"

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "net/cookies/cookie_util.h"

#include "api/qwebenginecookiestoreclient.h"
#include "api/qwebenginecookiestoreclient_p.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

static GURL sourceUrlForCookie(const QNetworkCookie &cookie) {
    QString urlFragment = QString("%1%2").arg(cookie.domain()).arg(cookie.path());
    return net::cookie_util::CookieOriginToURL(urlFragment.toStdString(), /* is_https */ cookie.isSecure());
}

static void onSetCookieCallback(QWebEngineCookieStoreClientPrivate *client, qint64 callbackId, bool success) {
    client->onSetCallbackResult(callbackId, success);
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

void CookieMonsterDelegateQt::setCookieMonster(net::CookieMonster* monster)
{
    m_cookieMonster = monster;

    if (m_client)
        m_client->d_func()->processPendingUserCookies();
}

void CookieMonsterDelegateQt::setClient(QWebEngineCookieStoreClient *client)
{
    m_client = client;

    if (!m_client)
        return;

    m_client->d_ptr->delegate = this;

    if (hasCookieMonster())
        m_client->d_func()->processPendingUserCookies();
}

void CookieMonsterDelegateQt::OnCookieChanged(const net::CanonicalCookie& cookie, bool removed, ChangeCause cause)
{
    if (!m_client)
        return;
    m_client->d_ptr->onCookieChanged(toQt(cookie), removed);
}

}
