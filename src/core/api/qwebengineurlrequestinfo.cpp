// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineurlrequestinfo.h"
#include "qwebengineurlrequestinfo_p.h"

#include "web_contents_adapter_client.h"

#include <memory>
#include <utility>

QT_BEGIN_NAMESPACE

// We changed the type from QScopedPointer to unique_ptr, make sure it's binary compatible:
static_assert(sizeof(QScopedPointer<QWebEngineUrlRequestInfoPrivate>)
              == sizeof(std::unique_ptr<QWebEngineUrlRequestInfoPrivate>));

ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::LinkNavigation, QWebEngineUrlRequestInfo::NavigationTypeLink)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::TypedNavigation, QWebEngineUrlRequestInfo::NavigationTypeTyped)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::FormSubmittedNavigation,
                   QWebEngineUrlRequestInfo::NavigationTypeFormSubmitted)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::BackForwardNavigation,
                   QWebEngineUrlRequestInfo::NavigationTypeBackForward)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::ReloadNavigation, QWebEngineUrlRequestInfo::NavigationTypeReload)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::OtherNavigation, QWebEngineUrlRequestInfo::NavigationTypeOther)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebContentsAdapterClient::RedirectNavigation, QWebEngineUrlRequestInfo::NavigationTypeRedirect)

/*!
    \class QWebEngineUrlRequestInfo
    \inmodule QtWebEngineCore
    \since 5.6
    \brief The QWebEngineUrlRequestInfo class provides information about URL requests.

    The QWebEngineUrlRequestInfo is useful for setting extra header fields for requests
    or for redirecting certain requests without payload data to another URL.
    This class cannot be instantiated or copied by the user, instead it will
    be created by \QWE and sent through the virtual function
    QWebEngineUrlRequestInterceptor::interceptRequest() if an interceptor has been set.
*/

/*!
    \class QWebEngineUrlRequestInterceptor
    \inmodule QtWebEngineCore
    \since 5.6
    \brief The QWebEngineUrlRequestInterceptor class provides an abstract base class for URL interception.

    Implementing the \l{QWebEngineUrlRequestInterceptor} interface and installing the
    interceptor on the profile enables intercepting, blocking, and modifying URL requests
    before they reach the networking stack of Chromium.

    You can install the interceptor on a profile via QWebEngineProfile::setUrlRequestInterceptor()
    or QQuickWebEngineProfile::setUrlRequestInterceptor().

    When using the \l{Qt WebEngine Widgets Module}, \l{QWebEnginePage::acceptNavigationRequest()}
    offers further options to accept or block requests.

    \sa interceptRequest(), QWebEngineUrlRequestInfo
*/

/*!
    \fn QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor(QObject *p = nullptr)

    Creates a new QWebEngineUrlRequestInterceptor object with \a p as parent.
*/

/*!
    \fn void QWebEngineUrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)

    Reimplementing this virtual function makes it possible to intercept URL
    requests. This method will be stalling the URL request until handled.

    \a info contains the information about the URL request and will track internally
    whether its members have been altered.

    \warning All method calls to the profile on the main thread will block until
    execution of this function is finished.
*/

QWebEngineUrlRequestInfoPrivate::QWebEngineUrlRequestInfoPrivate(
        QWebEngineUrlRequestInfo::ResourceType resource,
        QWebEngineUrlRequestInfo::NavigationType navigation, const QUrl &u, const QUrl &fpu,
        const QUrl &i, const QByteArray &m, const QHash<QByteArray, QByteArray> &h)
    : resourceType(resource)
    , navigationType(navigation)
    , shouldBlockRequest(false)
    , shouldRedirectRequest(false)
    , url(u)
    , firstPartyUrl(fpu)
    , initiator(i)
    , method(m)
    , changed(false)
    , extraHeaders(h)
{}

/*!
    \internal
*/
QWebEngineUrlRequestInfo::QWebEngineUrlRequestInfo() {}

/*!
    \internal
*/
QWebEngineUrlRequestInfo::QWebEngineUrlRequestInfo(QWebEngineUrlRequestInfo &&p)
    : d_ptr(std::move(p.d_ptr))
{
}

/*!
    \internal
*/
QWebEngineUrlRequestInfo &QWebEngineUrlRequestInfo::operator=(QWebEngineUrlRequestInfo &&p)
{
    d_ptr = std::move(p.d_ptr);
    return *this;
}

/*!
    \internal
*/

QWebEngineUrlRequestInfo::~QWebEngineUrlRequestInfo() {}

/*!
    \internal
*/

QWebEngineUrlRequestInfo::QWebEngineUrlRequestInfo(QWebEngineUrlRequestInfoPrivate *p) : d_ptr(p)
{
    d_ptr->q_ptr = this;
}

/*!
    \enum QWebEngineUrlRequestInfo::ResourceType

    This enum type holds the type of the requested resource:

    \value ResourceTypeMainFrame Top level page.
    \value ResourceTypeSubFrame  Frame or iframe.
    \value ResourceTypeStylesheet  A CSS stylesheet.
    \value ResourceTypeScript  An external script.
    \value ResourceTypeImage  An image (JPG, GIF, PNG, and so on).
    \value ResourceTypeFontResource  A font.
    \value ResourceTypeSubResource  An "other" subresource.
    \value ResourceTypeObject  An object (or embed) tag for a plugin or a resource that a plugin requested.
    \value ResourceTypeMedia  A media resource.
    \value ResourceTypeWorker  The main resource of a dedicated worker.
    \value ResourceTypeSharedWorker  The main resource of a shared worker.
    \value ResourceTypePrefetch  An explicitly requested prefetch.
    \value ResourceTypeFavicon  A favicon.
    \value ResourceTypeXhr  An XMLHttpRequest.
    \value ResourceTypePing  A ping request for <a ping>.
    \value ResourceTypeServiceWorker  The main resource of a service worker.
    \value ResourceTypeCspReport  A report of Content Security Policy (CSP)
           violations. CSP reports are in JSON format and they are delivered by
           HTTP POST requests to specified servers. (Added in Qt 5.7)
    \value ResourceTypePluginResource  A resource requested by a plugin. (Added in Qt 5.7)
    \value ResourceTypeNavigationPreloadMainFrame  A main-frame service worker navigation preload request. (Added in Qt 5.14)
    \value ResourceTypeNavigationPreloadSubFrame  A sub-frame service worker navigation preload request. (Added in Qt 5.14)
    \value ResourceTypeWebSocket  A WebSocket request. (Added in Qt 6.4)
    \value ResourceTypeUnknown  Unknown request type.

    \note For forward compatibility all values not matched should be treated as unknown,
    not just \c ResourceTypeUnknown.
*/

/*!
    Returns the resource type of the request.

    \sa ResourceType
*/

QWebEngineUrlRequestInfo::ResourceType QWebEngineUrlRequestInfo::resourceType() const
{
    return d_ptr->resourceType;
}

/*!
    \enum QWebEngineUrlRequestInfo::NavigationType

    This enum type describes the navigation type of the request:

    \value NavigationTypeLink Navigation initiated by clicking a link.
    \value NavigationTypeTyped Navigation explicitly initiated by typing a URL.
    \value NavigationTypeFormSubmitted Navigation submits a form.
    \value NavigationTypeBackForward Navigation initiated by a history action.
    \value NavigationTypeReload Navigation initiated by refreshing the page.
    \value NavigationTypeRedirect Navigation triggered automatically by page content or remote server. (Added in Qt 5.14)
    \value NavigationTypeOther None of the above.
*/

/*!
    Returns the navigation type of the request.

    \sa NavigationType
*/

QWebEngineUrlRequestInfo::NavigationType QWebEngineUrlRequestInfo::navigationType() const
{
    return d_ptr->navigationType;
}

/*!
    Returns the requested URL.
*/

QUrl QWebEngineUrlRequestInfo::requestUrl() const
{
    return d_ptr->url;
}

/*!
    Returns the first party URL of the request.
    The first party URL is the URL of the page that issued the request.
*/

QUrl QWebEngineUrlRequestInfo::firstPartyUrl() const
{
    return d_ptr->firstPartyUrl;
}

/*!
    Returns the origin URL of the document that initiated
    the navigation of a frame to another frame.

    \since 5.14
 */

QUrl QWebEngineUrlRequestInfo::initiator() const
{
    return d_ptr->initiator;
}

/*!
    Returns the HTTP method of the request (for example, GET or POST).
*/

QByteArray QWebEngineUrlRequestInfo::requestMethod() const
{
    return d_ptr->method;
}

/*!
    \internal
*/
bool QWebEngineUrlRequestInfo::changed() const
{
    return d_ptr->changed;
}

/*!
    \internal
*/
void QWebEngineUrlRequestInfo::resetChanged()
{
    d_ptr->changed = false;
}

/*!
    Redirects this request to \a url.
    It is only possible to redirect requests that do not have payload data, such as GET requests.
*/

void QWebEngineUrlRequestInfo::redirect(const QUrl &url)
{
    d_ptr->changed = true;
    d_ptr->url = url;
    d_ptr->shouldRedirectRequest = true;
}

/*!
    Blocks this request if \a shouldBlock is true, so that it will not proceed.

    This function can be used to prevent navigating away from a given domain, for example.
*/

void QWebEngineUrlRequestInfo::block(bool shouldBlock)
{
    d_ptr->changed = true;
    d_ptr->shouldBlockRequest = shouldBlock;
}

/*!
    Sets the request header \a name to \a value for this request.
*/

void QWebEngineUrlRequestInfo::setHttpHeader(const QByteArray &name, const QByteArray &value)
{
    // Headers are case insentive, so we need to compare manually
    for (auto it = d_ptr->extraHeaders.begin(); it != d_ptr->extraHeaders.end(); ++it) {
        if (it.key().compare(name, Qt::CaseInsensitive) == 0) {
            d_ptr->extraHeaders.erase(it);
            break;
        }
    }

    d_ptr->extraHeaders.insert(name, value);
}

/*!
    Returns the request headers.
    \since 6.5
    \note Not all headers are visible at this stage as Chromium will add
    security and proxy headers at a later stage.
*/

QHash<QByteArray, QByteArray> QWebEngineUrlRequestInfo::httpHeaders() const
{
    return d_ptr->extraHeaders;
}

QT_END_NAMESPACE
