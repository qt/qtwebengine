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

#include <cookie_monster_delegate_qt.h>

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

bool QWebEngineCookieStoreClientPrivate::canSetCookie(const QUrl &firstPartyUrl, const QByteArray &cookieLine, const QUrl &url)
{
    if (filterCallback) {
        QWebEngineCookieStoreClient::FilterRequest request;
        request.accepted = true;
        request.firstPartyUrl = firstPartyUrl;
        request.cookieLine = cookieLine;
        request.cookieSource = url;
        callbackDirectory.invokeDirectly<QWebEngineCookieStoreClient::FilterRequest&>(filterCallback, request);
        return request.accepted;
    }
    return true;
}

/*!
    \class QWebEngineCookieStoreClient
    \inmodule QtWebEngineCore
    \since 5.6
    \brief The QWebEngineCookieStoreClient class provides access to Chromium's cookies.

    The class allows to access HTTP cookies of Chromium for a specific profile.
    It can be used to synchronize cookies of Chromium and the QNetworkAccessManager, as well as
    to set, delete, and intercept cookies during navigation.
    Because cookie operations are asynchronous, the user can choose to provide a callback function
    to get notified about the success of the operation.
    The signal handlers for removal and addition should not be used to execute heavy tasks,
    because they might block the IO thread in case of a blocking connection.
*/

/*!
    \class QWebEngineCookieStoreClient::FilterRequest
    \inmodule QtWebEngineCore
    \since 5.6

    The structure specifies properties of a cookie, and whether it should accepted or not. It is
    used as an argument to a filter installed via setCookieFilter().
*/

/*!
    \variable QWebEngineCookieStoreClient::FilterRequest::accepted
    Whether the cookie shall be accepted. The default is \c true.
    \variable QWebEngineCookieStoreClient::FilterRequest::firstPartyUrl
    URL of page that triggered the setting of the cookie.
    \variable QWebEngineCookieStoreClient::FilterRequest::cookieLine
    Content of the cookie.
    \variable QWebEngineCookieStoreClient::FilterRequest::cookieSource
    URL of site that sets the cookie.
*/


/*!
    \fn void QWebEngineCookieStoreClient::cookieAdded(const QNetworkCookie &cookie)

    This signal is emitted whenever a new \a cookie is added to the cookie store.
*/

/*!
    \fn void QWebEngineCookieStoreClient::cookieRemoved(const QNetworkCookie &cookie)

    This signal is emitted whenever a \a cookie is deleted from the cookie store.
*/

/*!
    Creates a new QWebEngineCookieStoreClient object with \a parent.
*/

QWebEngineCookieStoreClient::QWebEngineCookieStoreClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new QWebEngineCookieStoreClientPrivate(this))
{

}

/*!
    Destroys this QWebEngineCookieStoreClient object.
*/

QWebEngineCookieStoreClient::~QWebEngineCookieStoreClient()
{

}

/*!
    \fn void setCookieWithCallback(const QNetworkCookie &cookie, FunctorOrLambda resultCallback, const QUrl &origin = QUrl())

    Adds \a cookie to the cookie store. When the operation finishes, \a resultCallback will be executed
    on the caller thread.
    It is possible to provide an optional \a origin URL argument to limit the scope of the cookie.
    The provided URL should also include the scheme.

    \sa setCookie()
*/

void QWebEngineCookieStoreClient::setCookieWithCallback(const QNetworkCookie &cookie, const QWebEngineCallback<bool> &resultCallback, const QUrl &origin)
{
    Q_D(QWebEngineCookieStoreClient);
    d->setCookie(resultCallback, cookie, origin);
}

/*!
    Adds \a cookie to the cookie store. This function is provided for convenience and is
    equivalent to calling setCookieWithCallback() with an empty callback.
    It is possible to provide an optional \a origin URL argument to limit the scope of the cookie.
    The provided URL should also include the scheme.

    \sa setCookieWithCallback()
*/

void QWebEngineCookieStoreClient::setCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    setCookieWithCallback(cookie, QWebEngineCallback<bool>(), origin);
}

/*!
    Deletes \a cookie from the cookie store.
    It is possible to provide an optional \a origin URL argument to limit the scope of the
    cookie to be deleted.
    The provided URL should also include the scheme.
*/

void QWebEngineCookieStoreClient::deleteCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_D(QWebEngineCookieStoreClient);
    d->deleteCookie(cookie, origin);
}

/*!
    \fn void QWebEngineCookieStoreClient::getAllCookies(FunctorOrLambda resultCallback)

    Requests all the cookies in the cookie store. When the asynchronous operation finishes,
    \a resultCallback will be called with a QByteArray as the argument containing the cookies.
    This QByteArray can be parsed using QNetworkCookie::parseCookies().

    \sa deleteCookie()
*/

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

/*!
    \fn void QWebEngineCookieStoreClient::deleteSessionCookiesWithCallback(FunctorOrLambda resultCallback)

    Deletes all the session cookies in the cookie store. Session cookies do not have an
    expiration date assigned to them.
    When the asynchronous operation finishes, \a resultCallback will be called with the
    number of cookies deleted as the argument.
*/

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

/*!
    \fn void QWebEngineCookieStoreClient::deleteAllCookiesWithCallback(FunctorOrLambda resultCallback)

    Deletes all the cookies in the cookie store. When the asynchronous operation finishes,
    \a resultCallback will be called with the number of cookies deleted as the argument.

    \sa deleteSessionCookiesWithCallback(), getAllCookies()
*/

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

/*!
    Deletes all the session cookies in the cookie store.

    \sa deleteSessionCookiesWithCallback()
*/

void QWebEngineCookieStoreClient::deleteSessionCookies()
{
    deleteSessionCookiesWithCallback(QWebEngineCallback<int>());
}

/*!
    Deletes all the cookies in the cookie store.

    \sa deleteAllCookiesWithCallback(), getAllCookies()
*/

void QWebEngineCookieStoreClient::deleteAllCookies()
{
    deleteAllCookiesWithCallback(QWebEngineCallback<int>());
}

/*!
  \fn void QWebEngineCookieStoreClient::setCookieFilter(FunctorOrLambda filterCallback)

    Installs a cookie filter that can reject cookies before they are added to the cookie store.
    The \a filterCallback must be a lambda or functor taking FilterRequest structure. If the
    cookie is to be rejected, the filter can set FilterRequest::accepted to \c false.

    The callback should not be used to execute heavy tasks since it is running on the
    IO thread and therefore blocks the Chromium networking.

    \sa deleteAllCookiesWithCallback(), getAllCookies()
*/
void QWebEngineCookieStoreClient::setCookieFilter(const QWebEngineCallback<QWebEngineCookieStoreClient::FilterRequest&> &filter)
{
    Q_D(QWebEngineCookieStoreClient);
    d->filterCallback = filter;
}

QT_END_NAMESPACE
