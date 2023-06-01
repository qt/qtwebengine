// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "authentication_dialog_controller.h"
#include "authentication_dialog_controller_p.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browser_task_traits.h"

namespace QtWebEngineCore {

AuthenticationDialogControllerPrivate::AuthenticationDialogControllerPrivate(base::WeakPtr<LoginDelegateQt> loginDelegate)
    : loginDelegate(loginDelegate)
{
}

void AuthenticationDialogControllerPrivate::dialogFinished(bool accepted, const QString &user, const QString &password)
{
    content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
                   base::BindOnce(&LoginDelegateQt::sendAuthToRequester,
                                  loginDelegate, accepted, user, password));
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
    return d->url;
}

QString AuthenticationDialogController::realm() const
{
    return d->realm;
}

QString AuthenticationDialogController::host() const
{
    return d->host;
}

bool AuthenticationDialogController::isProxy() const
{
    return d->isProxy;
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
