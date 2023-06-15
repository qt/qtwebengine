// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginenavigationrequest.h"

#include "qwebenginepage.h"

QT_BEGIN_NAMESPACE

class QWebEngineNavigationRequestPrivate {
public:
    QWebEngineNavigationRequestPrivate(const QUrl& url, QWebEngineNavigationRequest::NavigationType navigationType, bool mainFrame)
        : url(url)
        , navigationType(navigationType)
        , isMainFrame(mainFrame)
    {}

    QUrl url;
    QWebEngineNavigationRequest::NavigationType navigationType;
    bool isMainFrame;
    bool isAccepted = true;
};

/*!
    \class QWebEngineNavigationRequest
    \brief A utility type for the QWebEnginePage::navigationRequested signal.
    \since 6.2

    \inmodule QtWebEngineCore

    Contains information about a navigation request.

    To accept or reject a request, call accept() or reject().

    The default if not handled is to accept the navigation.

    \sa QWebEnginePage::navigationRequested
*/

/*!
    \qmltype WebEngineNavigationRequest
    \instantiates QWebEngineNavigationRequest
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.0

    \brief Represents a request for navigating to a web page as part of
    \l{WebEngineView::navigationRequested()}.

    To accept or reject a request, call accept() or reject().
*/

/*! \internal
*/
QWebEngineNavigationRequest::QWebEngineNavigationRequest(const QUrl& url, QWebEngineNavigationRequest::NavigationType navigationType, bool mainFrame, QObject* parent)
    : QObject(parent)
    , d_ptr(new QWebEngineNavigationRequestPrivate(url, navigationType, mainFrame))
{
}

QWebEngineNavigationRequest::~QWebEngineNavigationRequest()
{
}

#if QT_DEPRECATED_SINCE(6, 2)
/*!
    \qmlproperty enumeration WebEngineNavigationRequest::action

    Whether to accept or ignore the navigation request.

    \value  WebEngineNavigationRequest.AcceptRequest
            Accepts a navigation request.
    \value  WebEngineNavigationRequest.IgnoreRequest
            Ignores a navigation request.
*/
QWebEngineNavigationRequest::NavigationRequestAction QWebEngineNavigationRequest::action() const
{
    qWarning("Navigation request: action/setAction are deprecated. Please, use accept/reject methods instead.");
    Q_D(const QWebEngineNavigationRequest);
    return d->isAccepted ? AcceptRequest : IgnoreRequest;
}

/*! \internal */
void QWebEngineNavigationRequest::setAction(QWebEngineNavigationRequest::NavigationRequestAction action)
{
    qWarning("Navigation request: action/setAction are deprecated. Please, use accept/reject methods instead.");
    Q_D(QWebEngineNavigationRequest);
    bool acceptRequest = action == AcceptRequest;
    if (d->isAccepted == acceptRequest)
        return;

    acceptRequest ? accept() : reject();
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    emit actionChanged();
QT_WARNING_POP
}
#endif
/*!
    Accepts a navigation request.
*/
void QWebEngineNavigationRequest::accept()
{
    d_ptr->isAccepted = true;
}
/*!
    Rejects a navigation request.
*/
void QWebEngineNavigationRequest::reject()
{
    d_ptr->isAccepted = false;
}
/*!
    \property QWebEngineNavigationRequest::url
    \brief The URL of the web page to go to.
*/
/*!
    \qmlproperty url WebEngineNavigationRequest::url
    \readonly

    The URL of the web page to go to.
*/

QUrl QWebEngineNavigationRequest::url() const
{
    Q_D(const QWebEngineNavigationRequest);
    return d->url;
}
/*!
    \property QWebEngineNavigationRequest::navigationType
    \brief The method used to navigate to a web page.
*/
/*!
    \qmlproperty enumeration WebEngineNavigationRequest::navigationType
    \readonly

    The method used to navigate to a web page.

    \value  WebEngineNavigationRequest.LinkClickedNavigation
            Clicking a link.
    \value  WebEngineNavigationRequest.TypedNavigation
            Entering an URL on the address bar.
    \value  WebEngineNavigationRequest.FormSubmittedNavigation
            Submitting a form.
    \value  WebEngineNavigationRequest.BackForwardNavigation
            Using navigation history to go to the previous or next page.
    \value  WebEngineNavigationRequest.ReloadNavigation
            Reloading the page.
    \value  WebEngineNavigationRequest.RedirectNavigation
            Page content or server triggered a redirection or page refresh.
    \value  WebEngineNavigationRequest.OtherNavigation
            Using some other method to go to a page.
*/

QWebEngineNavigationRequest::NavigationType QWebEngineNavigationRequest::navigationType() const
{
    Q_D(const QWebEngineNavigationRequest);
    return d->navigationType;
}

/*!
    \property QWebEngineNavigationRequest::isMainFrame
    \brief Whether the navigation issue is requested for a top level page.
*/
/*!
    \qmlproperty bool WebEngineNavigationRequest::isMainFrame
    \readonly

    Whether the navigation issue is requested for a top level page.
*/

bool QWebEngineNavigationRequest::isMainFrame() const
{
    Q_D(const QWebEngineNavigationRequest);
    return d->isMainFrame;
}

/*! \internal */
bool QWebEngineNavigationRequest::isAccepted() const
{
    return d_ptr->isAccepted;
}

QT_END_NAMESPACE

#include "moc_qwebenginenavigationrequest.cpp"
