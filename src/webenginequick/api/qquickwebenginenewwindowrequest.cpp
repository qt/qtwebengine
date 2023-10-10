// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginenewwindowrequest_p.h"
#include "qquickwebengineview_p.h"

#include "web_contents_adapter_client.h"

QT_BEGIN_NAMESPACE

/*!
    \internal
*/
QQuickWebEngineNewWindowRequest::QQuickWebEngineNewWindowRequest(DestinationType dest, const QRect &rect, const QUrl &url,
                                                             bool user,
                                                             QSharedPointer<QtWebEngineCore::WebContentsAdapter> adapter,
                                                             QObject *parent)
        : QWebEngineNewWindowRequest(dest, rect, url, user, adapter, parent)
{
}

/*!
    \qmlmethod WebEngineNewWindowRequest::openIn(WebEngineView view)
    Opens the requested page in the new web engine view \a view. State and history of the
    view and the page possibly loaded in it will be lost.
    \sa WebEngineView::newWindowRequested
*/

/*!
    \internal
*/
void QQuickWebEngineNewWindowRequest::openIn(QQuickWebEngineView *view)
{
    if (!view) {
        qWarning("Trying to open a WebEngineNewWindowRequest in an invalid WebEngineView.");
        return;
    }
    view->acceptAsNewWindow(this);
}

QT_END_NAMESPACE

#include "moc_qquickwebenginenewwindowrequest_p.cpp"
