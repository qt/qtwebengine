/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
    \inqmlmodule QtWebEngineQuick
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
