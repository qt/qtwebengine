// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef AUTHENTICATION_DIALOG_CONTROLLER_H
#define AUTHENTICATION_DIALOG_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QObject>

namespace QtWebEngineCore {

class AuthenticationDialogControllerPrivate;

class Q_WEBENGINECORE_PRIVATE_EXPORT AuthenticationDialogController : public QObject {
    Q_OBJECT
public:
    ~AuthenticationDialogController();

    QUrl url() const;
    QString realm() const;
    QString host() const;
    bool isProxy() const;

public Q_SLOTS:
    void accept(const QString &user, const QString &password);
    void reject();

private:
    AuthenticationDialogController(AuthenticationDialogControllerPrivate *);

    QScopedPointer<AuthenticationDialogControllerPrivate> d;
    friend class LoginDelegateQt;
};

} // namespace QtWebEngineCore

#endif // AUTHENTICATION_DIALOG_CONTROLLER_H
