// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginenewwindowrequest.h"
#include "qwebenginenewwindowrequest_p.h"

#include "qwebenginepage.h"

#include "web_contents_adapter.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineNewWindowRequest
    \brief A utility type for the QWebEnginePage::newWindowRequested() signal.
    \since 6.2

    \inmodule QtWebEngineCore

    Contains information about a request to load a page in a separate web engine view.

    \sa QWebEnginePage::newWindowRequested()
*/

/*!
    \qmltype WebEngineNewWindowRequest
    \instantiates QWebEngineNewWindowRequest
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.12

    \brief A utility type for the WebEngineView::newWindowRequested signal.

    Contains information about a request to load a page in a separate web engine view.

    \sa WebEngineView::newWindowRequested
*/

/*!
    \enum QWebEngineNewWindowRequest::DestinationType

    This enum describes the type of window requested:

    \value  InNewWindow
            In a separate window.
    \value  InNewTab
            In a tab of the same window.
    \value  InNewDialog
            In a window without a tab bar, toolbar, or URL bar.
    \value  InNewBackgroundTab
            In a tab of the same window, without hiding the currently visible web engine view.
*/

/*!
    \qmlproperty enumeration WebEngineNewWindowRequest::DestinationType

    Describes how to open a new view:

    \value WebEngineNewWindowRequest.InNewWindow
            In a separate window.
    \value WebEngineNewWindowRequest.InNewTab
            In a tab of the same window.
    \value WebEngineNewWindowRequest.InNewDialog
            In a window without a tab bar, toolbar, or URL bar.
    \value WebEngineNewWindowRequest.InNewBackgroundTab
            In a tab of the same window, without hiding the currently visible web engine view.
*/

QWebEngineNewWindowRequest::QWebEngineNewWindowRequest(QWebEngineNewWindowRequest::DestinationType destination,
                                                       const QRect &geometry,
                                                       const QUrl &url,
                                                       bool userInitiated,
                                                       QSharedPointer<QtWebEngineCore::WebContentsAdapter> adapter,
                                                       QObject *parent)
    : QObject(parent)
    , d_ptr(new QWebEngineNewWindowRequestPrivate{destination, geometry, url, adapter, userInitiated})
{
}

QWebEngineNewWindowRequest::~QWebEngineNewWindowRequest()
{
}

/*!
    \property QWebEngineNewWindowRequest::destination
    \brief The type of window that is requested.
*/
/*!
    \qmlproperty WebEngineNewWindowRequest::DestinationType WebEngineNewWindowRequest::destination
    \brief The type of window that is requested.
*/
QWebEngineNewWindowRequest::DestinationType QWebEngineNewWindowRequest::destination() const
{
    return d_ptr->destination;
}

/*!
    \property QWebEngineNewWindowRequest::requestedUrl
    \brief The URL that is requested for the new page.
*/
/*!
    \qmlproperty QUrl WebEngineNewWindowRequest::requestedUrl
    \brief The URL that is requested for the new page.
    \since QtWebEngine 1.5
 */
QUrl QWebEngineNewWindowRequest::requestedUrl() const
{
    return d_ptr->requestedUrl;
}

/*!
    \property QWebEngineNewWindowRequest::requestedGeometry
    \brief The size that is requested for the new page.
*/
/*!
    \qmlproperty QRect WebEngineNewWindowRequest::requestedGeometry
    \brief The size that is requested for the new page.
    \since QtWebEngine 2.0
 */
QRect QWebEngineNewWindowRequest::requestedGeometry() const
{
    return d_ptr->requestedGeometry;
}

/*!
    \property QWebEngineNewWindowRequest::userInitiated
    Whether this page request was directly triggered as the result of a keyboard or mouse event.

    You can use this property to block automatic \e popups.
*/
/*!
    \qmlproperty bool WebEngineNewWindowRequest::userInitiated
    Whether this window request was directly triggered as the result of a keyboard or mouse event.

    You can use this property to block automatic \e popups.
 */
bool QWebEngineNewWindowRequest::isUserInitiated() const
{
    return d_ptr->isUserInitiated;
}

/*!
    Opens the requested window in the view represented by \a page.

    \sa QWebEnginePage::newWindowRequested
*/
void QWebEngineNewWindowRequest::openIn(QWebEnginePage *page)
{
    if (!page) {
        qWarning("Trying to open a QWebEngineNewWindowRequest in an invalid QWebEnginePage.");
        return;
    }
    page->acceptAsNewWindow(*this);
}

QT_END_NAMESPACE

#include "moc_qwebenginenewwindowrequest.cpp"
