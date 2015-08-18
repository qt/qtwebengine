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

#include "qwebengineurlschemehandler_p.h"
#include "qwebengineurlschemehandler_p_p.h"

#include "qwebengineprofile.h"
#include "qwebengineprofile_p.h"
#include "qwebengineurlrequestjob_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlSchemeHandler
    \brief The QWebEngineUrlSchemeHandler Base class for handling custom URL schemes.
    \since 5.5
    \internal

    To implement a custom URL scheme for QtWebEngine you must write a class derived from this class,
    and reimplement requestStarted().

    To install a custom URL scheme handler into a QtWebProfile, you only need to call the constructor
    with the correct profile. Each instance of a QWebEngineUrlSchemeHandler can only handle requests
    from a single profile.

    \inmodule QtWebEngineWidgets

*/

QWebEngineUrlSchemeHandlerPrivate::QWebEngineUrlSchemeHandlerPrivate(const QByteArray &scheme, QWebEngineUrlSchemeHandler *q, QWebEngineProfile *profile)
    : CustomUrlSchemeHandler(scheme)
    , q_ptr(q)
    , m_profile(profile)
{
}

QWebEngineUrlSchemeHandlerPrivate::~QWebEngineUrlSchemeHandlerPrivate()
{
}

bool QWebEngineUrlSchemeHandlerPrivate::handleJob(QtWebEngineCore::URLRequestCustomJobDelegate *job)
{
    QWebEngineUrlRequestJob *requestJob = new QWebEngineUrlRequestJob(job);
    q_ptr->requestStarted(requestJob);
    return true;
}

/*!
    Constructs a new URL scheme handler.

    The handler is created for \a scheme and for the \a profile.

  */
QWebEngineUrlSchemeHandler::QWebEngineUrlSchemeHandler(const QByteArray &scheme, QWebEngineProfile *profile, QObject *parent)
    : QObject(parent)
    , d_ptr(new QWebEngineUrlSchemeHandlerPrivate(scheme, this, profile))
{
    profile->d_func()->installUrlSchemeHandler(this);
}

QWebEngineUrlSchemeHandler::~QWebEngineUrlSchemeHandler()
{
    if (d_ptr->m_profile)
        d_ptr->m_profile->d_func()->removeUrlSchemeHandler(this);
}

/*!
    Returns the custom URL scheme handled.
*/
QByteArray QWebEngineUrlSchemeHandler::scheme() const
{
    return d_ptr->scheme();
}

/*!
    \fn void QWebEngineUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)

    This method is called whenever a request for the registered scheme is started.

    This method must be reimplemented by all custom URL scheme handlers.
    The request is asynchronous and does not need to be handled right away.

    \sa QWebEngineUrlRequestJob
*/

QT_END_NAMESPACE
