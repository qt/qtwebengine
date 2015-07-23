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

#include "qwebengineurlrequestjob_p.h"

#include "qwebengineprofile.h"

#include "url_request_custom_job_delegate.h"

using QtWebEngineCore::URLRequestCustomJobDelegate;

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlRequestJob
    \brief The QWebEngineUrlRequestJob class represents a custom URL request.
    \since 5.5
    \internal

    A QWebEngineUrlRequestJob is given to QWebEngineUrlSchemeHandler::requestStarted() and must
    be handled by the derived implementations of class.

    A job can be handled by calling either setReply(), redirect() or setError().

    The class is owned by QtWebEngine and does not need to be deleted. Note QtWebEngine may delete
    the job when it is no longer needed, so the signal QObject::destroyed() must be monitored if
    a pointer to the object is stored.

    \inmodule QtWebEngineWidgets
*/

/*!
    \internal
 */
QWebEngineUrlRequestJob::QWebEngineUrlRequestJob(URLRequestCustomJobDelegate * p)
    : QObject(p) // owned by the jobdelegate and deleted when the job is done
    , d_ptr(p)
{
}

/*!
    \internal
 */
QWebEngineUrlRequestJob::~QWebEngineUrlRequestJob()
{
}

/*!
    Returns the url requested.
 */
QUrl QWebEngineUrlRequestJob::requestUrl() const
{
    return d_ptr->url();
}

/*!
    Sets the reply for the request to \a device with the mime-type \a contentType.
 */
void QWebEngineUrlRequestJob::setReply(const QByteArray &contentType, QIODevice *device)
{
    d_ptr->setReply(contentType, device);
}

/*!
    Fails the request with error \a error.
 */
void QWebEngineUrlRequestJob::setError(Error r)
{
    d_ptr->fail((URLRequestCustomJobDelegate::Error)r);
}

/*!
    Tell the request is redirected to \a url.
 */
void QWebEngineUrlRequestJob::setRedirect(const QUrl &url)
{
    d_ptr->redirect(url);
}

QT_END_NAMESPACE
