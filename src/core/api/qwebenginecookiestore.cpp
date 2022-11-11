// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginecookiestore.h"
#include "qwebenginecookiestore_p.h"

#include "net/base/registry_controlled_domains/registry_controlled_domain.h"

#include "net/cookie_monster_delegate_qt.h"

#include <QByteArray>
#include <QUrl>

namespace {

inline GURL toGurl(const QUrl &url)
{
    return GURL(url.toString().toStdString());
}

} // namespace

QT_BEGIN_NAMESPACE

using namespace QtWebEngineCore;

QWebEngineCookieStorePrivate::QWebEngineCookieStorePrivate(QWebEngineCookieStore *q)
    : q_ptr(q)
    , m_deleteSessionCookiesPending(false)
    , m_deleteAllCookiesPending(false)
    , m_getAllCookiesPending(false)
    , delegate(nullptr)
{}

void QWebEngineCookieStorePrivate::processPendingUserCookies()
{
    Q_ASSERT(delegate);
    Q_ASSERT(delegate->hasCookieMonster());

    if (m_getAllCookiesPending) {
        m_getAllCookiesPending = false;
        delegate->getAllCookies();
    }

    if (m_deleteAllCookiesPending) {
        m_deleteAllCookiesPending = false;
        delegate->deleteAllCookies();
    }

    if (m_deleteSessionCookiesPending) {
        m_deleteSessionCookiesPending = false;
        delegate->deleteSessionCookies();
    }

    if (bool(filterCallback))
        delegate->setHasFilter(true);

    if (m_pendingUserCookies.isEmpty())
        return;

    for (const CookieData &cookieData : std::as_const(m_pendingUserCookies)) {
        if (cookieData.wasDelete)
            delegate->deleteCookie(cookieData.cookie, cookieData.origin);
        else
            delegate->setCookie(cookieData.cookie, cookieData.origin);
    }

    m_pendingUserCookies.clear();
}

void QWebEngineCookieStorePrivate::rejectPendingUserCookies()
{
    m_getAllCookiesPending = false;
    m_deleteAllCookiesPending = false;
    m_deleteSessionCookiesPending = false;
    m_pendingUserCookies.clear();
}

void QWebEngineCookieStorePrivate::setCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_pendingUserCookies.append(CookieData{ false, cookie, origin });
        return;
    }

    delegate->setCookie(cookie, origin);
}

void QWebEngineCookieStorePrivate::deleteCookie(const QNetworkCookie &cookie, const QUrl &url)
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_pendingUserCookies.append(CookieData{ true, cookie, url });
        return;
    }

    delegate->deleteCookie(cookie, url);
}

void QWebEngineCookieStorePrivate::deleteSessionCookies()
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_deleteSessionCookiesPending = true;
        return;
    }

    delegate->deleteSessionCookies();
}

void QWebEngineCookieStorePrivate::deleteAllCookies()
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_deleteAllCookiesPending = true;
        m_deleteSessionCookiesPending = false;
        return;
    }

    delegate->deleteAllCookies();
}

void QWebEngineCookieStorePrivate::getAllCookies()
{
    if (!delegate || !delegate->hasCookieMonster()) {
        m_getAllCookiesPending = true;
        return;
    }

    delegate->getAllCookies();
}


void QWebEngineCookieStorePrivate::onCookieChanged(const QNetworkCookie &cookie, bool removed)
{
    if (removed)
        Q_EMIT q_ptr->cookieRemoved(cookie);
    else
        Q_EMIT q_ptr->cookieAdded(cookie);
}

bool QWebEngineCookieStorePrivate::canAccessCookies(const QUrl &firstPartyUrl, const QUrl &url) const
{
    if (!filterCallback)
        return true;

    // Empty first-party URL indicates a first-party request (see net/base/static_cookie_policy.cc)
    bool thirdParty = !firstPartyUrl.isEmpty() &&
            !net::registry_controlled_domains::SameDomainOrHost(toGurl(url),
                                                                toGurl(firstPartyUrl),
                                                                net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);

    QWebEngineCookieStore::FilterRequest request = { firstPartyUrl, url, thirdParty, false, 0 };
    return filterCallback(request);
}

/*!
    \class QWebEngineCookieStore
    \inmodule QtWebEngineCore
    \since 5.6
    \brief The QWebEngineCookieStore class provides access to Chromium's cookies.

    The class allows to access HTTP cookies of Chromium for a specific profile.
    It can be used to synchronize cookies of Chromium and the QNetworkAccessManager, as well as
    to set, delete, and intercept cookies during navigation.
    Because cookie operations are asynchronous, the user can choose to provide a callback function
    to get notified about the success of the operation.
    The signal handlers for removal and addition should not be used to execute heavy tasks,
    because they might block the IO thread in case of a blocking connection.

    Use QWebEngineProfile::cookieStore() and QQuickWebEngineProfile::cookieStore()
    to access the cookie store object for a specific profile.
*/

/*!
    \fn void QWebEngineCookieStore::cookieAdded(const QNetworkCookie &cookie)

    This signal is emitted whenever a new \a cookie is added to the cookie store.
*/

/*!
    \fn void QWebEngineCookieStore::cookieRemoved(const QNetworkCookie &cookie)

    This signal is emitted whenever a \a cookie is deleted from the cookie store.
*/

/*!
    Creates a new QWebEngineCookieStore object with \a parent.
*/

QWebEngineCookieStore::QWebEngineCookieStore(QObject *parent)
    : QObject(parent)
    , d_ptr(new QWebEngineCookieStorePrivate(this))
{
}

/*!
    Destroys this QWebEngineCookieStore object.
*/

QWebEngineCookieStore::~QWebEngineCookieStore() {}

/*!
    Adds \a cookie to the cookie store.
    \note If \a cookie specifies a QNetworkCookie::domain() that does not start with a dot,
    a dot is automatically prepended. To limit the cookie to the exact server,
    omit QNetworkCookie::domain() and set \a origin instead.

    The provided URL should also include the scheme.

    \note This operation is asynchronous.
*/

void QWebEngineCookieStore::setCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    d_ptr->setCookie(cookie, origin);
}

/*!
    Deletes \a cookie from the cookie store.
    It is possible to provide an optional \a origin URL argument to limit the scope of the
    cookie to be deleted.

    \note This operation is asynchronous.
*/

void QWebEngineCookieStore::deleteCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    d_ptr->deleteCookie(cookie, origin);
}

/*!
    Loads all the cookies into the cookie store. The cookieAdded() signal is emitted on every
    loaded cookie. Cookies are loaded automatically when the store gets initialized, which
    in most cases happens on loading the first URL. However, calling this function is useful
    if cookies should be listed before entering the web content.

    \note This operation is asynchronous.
*/

void QWebEngineCookieStore::loadAllCookies()
{
    if (d_ptr->m_getAllCookiesPending)
        return;
    //this will trigger cookieAdded signal
    d_ptr->getAllCookies();
}

/*!
    Deletes all the session cookies in the cookie store. Session cookies do not have an
    expiration date assigned to them.

    \note This operation is asynchronous.
    \sa loadAllCookies()
*/

void QWebEngineCookieStore::deleteSessionCookies()
{
    if (d_ptr->m_deleteAllCookiesPending || d_ptr->m_deleteSessionCookiesPending)
        return;
    d_ptr->deleteSessionCookies();
}

/*!
    Deletes all the cookies in the cookie store.
    \note This operation is asynchronous.
    \sa loadAllCookies()
*/

void QWebEngineCookieStore::deleteAllCookies()
{
    if (d_ptr->m_deleteAllCookiesPending)
        return;
    d_ptr->deleteAllCookies();
}

/*!
    \since 5.11

    Installs a cookie filter that can prevent sites and resources from using cookies.
    The \a filterCallback must be a lambda or functor taking a FilterRequest structure. If the
    cookie access is to be accepted, the filter function should return \c true; otherwise
    it should return \c false.

    The following code snippet illustrates how to set a cookie filter:

    \code
    profile->cookieStore()->setCookieFilter(
        [&allowThirdPartyCookies](const QWebEngineCookieStore::FilterRequest &request)
        { return !request.thirdParty || allowThirdPartyCookies; }
    );
    \endcode

    You can unset the filter with a \c nullptr argument.

    The callback should not be used to execute heavy tasks since it is running on the
    IO thread and therefore blocks the Chromium networking.

    \note The cookie filter also controls other features with tracking capabilities similar to
    those of cookies; including IndexedDB, DOM storage, filesystem API, service workers,
    and AppCache.

    \sa deleteAllCookies(), loadAllCookies()
*/
void QWebEngineCookieStore::setCookieFilter(const std::function<bool(const FilterRequest &)> &filterCallback)
{
    bool changed = bool(d_ptr->filterCallback) != bool(filterCallback);
    d_ptr->filterCallback = filterCallback;
    if (changed && d_ptr->delegate)
        d_ptr->delegate->setHasFilter(bool(d_ptr->filterCallback));
}

/*!
    \since 5.11
    \overload
*/
void QWebEngineCookieStore::setCookieFilter(std::function<bool(const FilterRequest &)> &&filterCallback)
{
    bool changed = bool(d_ptr->filterCallback) != bool(filterCallback);
    d_ptr->filterCallback = std::move(filterCallback);
    if (changed && d_ptr->delegate)
        d_ptr->delegate->setHasFilter(bool(d_ptr->filterCallback));
}

/*!
    \class QWebEngineCookieStore::FilterRequest
    \inmodule QtWebEngineCore
    \since 5.11

    \brief The QWebEngineCookieStore::FilterRequest struct is used in conjunction with QWebEngineCookieStore::setCookieFilter() and is
    the type \a filterCallback operates on.

    \sa QWebEngineCookieStore::setCookieFilter()
*/

/*!
    \variable QWebEngineCookieStore::FilterRequest::firstPartyUrl
    \brief The URL that was navigated to.

    The site that would be showing in the location bar if the application has one.

    Can be used to white-list or black-list cookie access or third-party cookie access
    for specific sites visited.

    \sa origin, thirdParty
*/

/*!
    \variable QWebEngineCookieStore::FilterRequest::_reservedFlag
    \internal
*/

/*!
    \variable QWebEngineCookieStore::FilterRequest::_reservedType
    \internal
*/

/*!
    \variable QWebEngineCookieStore::FilterRequest::origin
    \brief The URL of the script or content accessing a cookie.

    Can be used to white-list or black-list third-party cookie access
    for specific services.

    \sa firstPartyUrl, thirdParty
*/

/*!
    \variable QWebEngineCookieStore::FilterRequest::thirdParty
    \brief Whether this is considered a third-party access.

    This is calculated by comparing FilterRequest::origin and FilterRequest::firstPartyUrl and
    checking if they share a common origin that is not a top-domain (like .com or .co.uk),
    or a known hosting site with independently owned subdomains.

    \sa firstPartyUrl, origin
*/

QT_END_NAMESPACE

#include "moc_qwebenginecookiestore.cpp"
