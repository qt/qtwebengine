/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qquickwebengineloadrequest_p.h>

/*!
    \qmltype WebEngineLoadRequest
    \instantiates QQuickWebEngineLoadRequest
    \inqmlmodule QtWebEngine 1.0

    \brief A utility class for the WebEngineView::loadingStateChanged signal.

    This class contains information about a requested load of a web page, like the URL and
    current loading status (started, finished, failed).

    \sa WebEngineView::onLoadingStateChanged
*/
QQuickWebEngineLoadRequest::QQuickWebEngineLoadRequest(const QUrl& url, QQuickWebEngineView::LoadStatus status, const QString& errorString, int errorCode, QQuickWebEngineView::ErrorDomain errorDomain, QObject* parent)
    : QObject(parent)
    , d(new QQuickWebEngineLoadRequestPrivate(url, status, errorString, errorCode, errorDomain))
{
}

QQuickWebEngineLoadRequest::~QQuickWebEngineLoadRequest()
{
}

/*!
    \qmlproperty url WebEngineLoadRequest::url
    \brief The URL of the load request.
 */
QUrl QQuickWebEngineLoadRequest::url() const
{
    return d->url;
}

/*!
    \qmlproperty enumeration WebEngineLoadRequest::status

    The load status of a web page load request.

    \list
    \li WebEngineView::LoadStartedStatus - the page is currently loading.
    \li WebEngineView::LoadSucceededStatus - the page has been loaded with success.
    \li WebEngineView::LoadFailedStatus - the page has failed loading.
    \endlist

    \sa WebEngineLoadRequest
    \sa WebEngineView::onLoadingStateChanged
*/
QQuickWebEngineView::LoadStatus QQuickWebEngineLoadRequest::status() const
{
    return d->status;
}

/*!
    \qmlproperty string WebEngineLoadRequest::errorString
*/
QString QQuickWebEngineLoadRequest::errorString() const
{
    return d->errorString;
}

QQuickWebEngineView::ErrorDomain QQuickWebEngineLoadRequest::errorDomain() const
{
    return d->errorDomain;
}

/*!
    \qmlproperty int WebEngineLoadRequest::errorCode
*/
int QQuickWebEngineLoadRequest::errorCode() const
{
    return d->errorCode;
}
