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

#include "authentication_dialog_controller.h"
#include "authentication_dialog_controller_p.h"

#include "content/public/browser/browser_thread.h"

namespace QtWebEngineCore {

AuthenticationDialogControllerPrivate::AuthenticationDialogControllerPrivate(ResourceDispatcherHostLoginDelegateQt *loginDelegate)
    : loginDelegate(loginDelegate)
{
}

void AuthenticationDialogControllerPrivate::dialogFinished(bool accepted, const QString &user, const QString &password)
{
    content::BrowserThread::PostTask(
        content::BrowserThread::IO, FROM_HERE,
        base::Bind(&ResourceDispatcherHostLoginDelegateQt::sendAuthToRequester, loginDelegate, accepted, user, password));
}

AuthenticationDialogController::AuthenticationDialogController(AuthenticationDialogControllerPrivate *dd)
{
    Q_ASSERT(dd);
    d.reset(dd);
}

AuthenticationDialogController::~AuthenticationDialogController()
{
}

QUrl AuthenticationDialogController::url() const
{
    return d->loginDelegate->url();
}

QString AuthenticationDialogController::realm() const
{
    return d->loginDelegate->realm();
}

QString AuthenticationDialogController::host() const
{
    return d->loginDelegate->host();
}

bool AuthenticationDialogController::isProxy() const
{
    return d->loginDelegate->isProxy();
}

void AuthenticationDialogController::accept(const QString &user, const QString &password)
{
    d->dialogFinished(true, user, password);
}

void AuthenticationDialogController::reject()
{
    d->dialogFinished(false);
}

} // namespace QtWebEngineCore
