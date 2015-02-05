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
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
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

#include <QSharedPointer>

QT_BEGIN_NAMESPACE

QWebEngineUrlSchemeHandlerPrivate::QWebEngineUrlSchemeHandlerPrivate(const QByteArray &scheme, QWebEngineUrlSchemeHandler *q, QWebEngineProfile *profile)
    : CustomUrlSchemeHandler(scheme)
    , q_ptr(q)
    , m_profile(profile)
{
}

QWebEngineUrlSchemeHandlerPrivate::~QWebEngineUrlSchemeHandlerPrivate()
{
}

bool QWebEngineUrlSchemeHandlerPrivate::handleJob(URLRequestCustomJobDelegate *job)
{
    QWebEngineUrlRequestJob *requestJob = new QWebEngineUrlRequestJob(job);
    q_ptr->requestStarted(requestJob);
    return true;
}

QWebEngineUrlSchemeHandler::QWebEngineUrlSchemeHandler(const QByteArray &scheme, QWebEngineProfile *profile)
    : QObject(profile)
    , d_ptr(new QWebEngineUrlSchemeHandlerPrivate(scheme, this, profile))
{
    profile->d_func()->installUrlSchemeHandler(this);
}

QWebEngineUrlSchemeHandler::~QWebEngineUrlSchemeHandler()
{
    d_ptr->m_profile->d_func()->removeUrlSchemeHandler(this);
}

QByteArray QWebEngineUrlSchemeHandler::scheme() const
{
    return d_ptr->scheme();
}

QT_END_NAMESPACE
