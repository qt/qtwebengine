// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTHENTICATION_DIALOG_CONTROLLER_P_H
#define AUTHENTICATION_DIALOG_CONTROLLER_P_H

#include "base/memory/weak_ptr.h"

#include "login_delegate_qt.h"

namespace QtWebEngineCore {

class AuthenticationDialogControllerPrivate {

public:
    AuthenticationDialogControllerPrivate(base::WeakPtr<LoginDelegateQt> loginDelegate);
    void dialogFinished(bool accepted, const QString &user = QString(), const QString &password = QString());

    base::WeakPtr<LoginDelegateQt> loginDelegate;
    QUrl url;
    QString host;
    QString realm;
    bool isProxy;
};

} // namespace QtWebEngineCore

#endif // AUTHENTICATION_DIALOG_CONTROLLER_H
