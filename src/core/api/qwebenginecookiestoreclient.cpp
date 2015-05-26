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

#include "qwebenginecookiestoreclient.h"
#include "qwebenginecookiestoreclient_p.h"

#include <QByteArray>
#include <QUrl>

QT_BEGIN_NAMESPACE

using namespace QtWebEngineCore;

QWebEngineCookieStoreClientPrivate::QWebEngineCookieStoreClientPrivate(QWebEngineCookieStoreClient* q)
    : m_nextCallbackId(CallbackDirectory::ReservedCallbackIdsEnd)
    , m_deleteSessionCookiesPending(false)
    , m_deleteAllCookiesPending(false)
    , m_getAllCookiesPending(false)
    , delegate(0)
    , q_ptr(q)
{
}

QWebEngineCookieStoreClientPrivate::~QWebEngineCookieStoreClientPrivate()
{

}

void QWebEngineCookieStoreClientPrivate::processPendingUserCookies()
{
    Q_ASSERT(delegate);
    Q_ASSERT(delegate->hasCookieMonster());

    if (m_getAllCookiesPending) {
        m_getAllCookiesPending = false;
        delegate->getAllCookies(CallbackDirectory::GetAllCookiesCallbackId);
    }

    if (m_deleteAllCookiesPending) {
        m_deleteAllCookiesPending = false;
        delegate->deleteAllCookies(CallbackDirectory::DeleteAllCookiesCallbackId);
    }

    if (m_deleteSessionCookiesPending) {
        m_deleteSessionCookiesPending = false;
        delegate->deleteSessionCookies(CallbackDirectory::DeleteSessionCookiesCallbackId);
    }

    if (m_pendingUserCookies.isEmpty())
        return;

    Q_FOREACH (const auto &cookieData, m_pendingUserCookies) {
        if (cookieData.callbackId == CallbackDirectory::DeleteCookieCallbackId)
            delegate->deleteCookie(cookieData.cookie, cookieData.origin);
        else
            delegate->setCookie(cookieData.callbackId, cookieData.cookie, cookieData.origin);
    }

    m_pendingUserCookies.clear();
}

void QWebEngineCookieStoreClientPrivate::setCookie(const QWebEngineCallback<bool> &callback, const QNetworkCookie &cookie, const QUrl &origin)
{
    const quint64 currentCallbackId = callback ? m_nextCallbackId++ : static_cast<quint64>(CallbackDirectory::NoCallbackId);

    if (currentCallbackId != CallbackDirectory::NoCallbackId)
        callbackDirectory.registerCallback(currentCallbackId, callback);

    if (!delegate || !delegate->hasCookieMonster()) {
        m_pendingUserCookies.append(CookieData{ currentCallbackId, cookie, origin });
        return;
    }

    delegate->setCookie(currentCallbackId, cookie, origin);
}

void QWebEngineCookieStoreClientPrivate::deleteCookie(const QNetworkCookie &cookie, const QUrl &url)
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_pendingUserCookies.append(CookieData{ CallbackDirectory::DeleteCookieCallbackId, cookie, url });
        return;
    }

    delegate->deleteCookie(cookie, url);
}

void QWebEngineCookieStoreClientPrivate::deleteSessionCookies()
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_deleteSessionCookiesPending = true;
        return;
    }

    delegate->deleteSessionCookies(CallbackDirectory::DeleteSessionCookiesCallbackId);
}

void QWebEngineCookieStoreClientPrivate::deleteAllCookies()
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_deleteAllCookiesPending = true;
        m_deleteSessionCookiesPending = false;
        return;
    }

    delegate->deleteAllCookies(CallbackDirectory::DeleteAllCookiesCallbackId);
}

void QWebEngineCookieStoreClientPrivate::getAllCookies()
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_getAllCookiesPending = true;
        return;
    }

    delegate->getAllCookies(CallbackDirectory::GetAllCookiesCallbackId);
}

void QWebEngineCookieStoreClientPrivate::onGetAllCallbackResult(qint64 callbackId, const QByteArray &cookieList)
{
    callbackDirectory.invoke(callbackId, cookieList);
}
void QWebEngineCookieStoreClientPrivate::onSetCallbackResult(qint64 callbackId, bool success)
{
    callbackDirectory.invoke(callbackId, success);
}

void QWebEngineCookieStoreClientPrivate::onDeleteCallbackResult(qint64 callbackId, int numCookies)
{
    callbackDirectory.invoke(callbackId, numCookies);
}

void QWebEngineCookieStoreClientPrivate::onCookieChanged(const QNetworkCookie &cookie, bool removed)
{
    Q_Q(QWebEngineCookieStoreClient);
    if (removed)
        Q_EMIT q->cookieRemoved(cookie);
    else
        Q_EMIT q->cookieAdded(cookie);
}

QWebEngineCookieStoreClient::QWebEngineCookieStoreClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new QWebEngineCookieStoreClientPrivate(this))
{

}

QWebEngineCookieStoreClient::~QWebEngineCookieStoreClient()
{

}

void QWebEngineCookieStoreClient::setCookieWithCallback(const QNetworkCookie &cookie, const QWebEngineCallback<bool> &resultCallback, const QUrl &origin)
{
    Q_D(QWebEngineCookieStoreClient);
    d->setCookie(resultCallback, cookie, origin);
}

void QWebEngineCookieStoreClient::setCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    setCookieWithCallback(cookie, QWebEngineCallback<bool>(), origin);
}

void QWebEngineCookieStoreClient::deleteCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_D(QWebEngineCookieStoreClient);
    d->deleteCookie(cookie, origin);
}

void QWebEngineCookieStoreClient::getAllCookies(const QWebEngineCallback<const QByteArray&> &resultCallback)
{
    Q_D(QWebEngineCookieStoreClient);
    if (d->m_getAllCookiesPending) {
        d->callbackDirectory.invokeEmpty(resultCallback);
        return;
    }
    d->callbackDirectory.registerCallback(CallbackDirectory::GetAllCookiesCallbackId, resultCallback);
    d->getAllCookies();
}

void QWebEngineCookieStoreClient::deleteSessionCookiesWithCallback(const QWebEngineCallback<int> &resultCallback)
{
    Q_D(QWebEngineCookieStoreClient);
    if (d->m_deleteAllCookiesPending || d->m_deleteSessionCookiesPending) {
        d->callbackDirectory.invokeEmpty(resultCallback);
        return;
    }
    d->callbackDirectory.registerCallback(CallbackDirectory::DeleteSessionCookiesCallbackId, resultCallback);
    d->deleteSessionCookies();
}

void QWebEngineCookieStoreClient::deleteAllCookiesWithCallback(const QWebEngineCallback<int> &resultCallback)
{
    Q_D(QWebEngineCookieStoreClient);
    if (d->m_deleteAllCookiesPending) {
        d->callbackDirectory.invokeEmpty(resultCallback);
        return;
    }
    d->callbackDirectory.registerCallback(CallbackDirectory::DeleteAllCookiesCallbackId, resultCallback);
    d->deleteAllCookies();
}

void QWebEngineCookieStoreClient::deleteSessionCookies()
{
    deleteSessionCookiesWithCallback(QWebEngineCallback<int>());
}

void QWebEngineCookieStoreClient::deleteAllCookies()
{
    deleteAllCookiesWithCallback(QWebEngineCallback<int>());
}

QT_END_NAMESPACE
