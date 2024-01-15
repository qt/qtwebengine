// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTHENTICATOR_REQUEST_DIALOG_CONTROLLER_H
#define AUTHENTICATOR_REQUEST_DIALOG_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtCore/qobject.h>
#include "qwebenginewebauthuxrequest.h"

namespace content {
class WebContents;
class RenderFrameHost;
}
namespace QtWebEngineCore {

class AuthenticatorRequestDialogControllerPrivate;

class Q_WEBENGINECORE_EXPORT AuthenticatorRequestDialogController : public QObject
{
    Q_OBJECT
public:
    ~AuthenticatorRequestDialogController();
    void sendSelectAccountResponse(const QString &account);
    void sendCollectPinResponse(const QString &pin);
    QStringList userNames() const;
    QWebEngineWebAuthPinRequest pinRequest();
    void reject();
    AuthenticatorRequestDialogController(AuthenticatorRequestDialogControllerPrivate *);

    QWebEngineWebAuthUxRequest::WebAuthUxState state() const;
    QString relyingPartyId() const;
    void retryRequest();
    QWebEngineWebAuthUxRequest::RequestFailureReason requestFailureReason() const;

Q_SIGNALS:
    void stateChanged(QWebEngineWebAuthUxRequest::WebAuthUxState state);

private:
    void selectAccount(const QStringList &userList);
    void collectPin(QWebEngineWebAuthPinRequest pinRequest);
    void finishCollectToken();
    void startRequest(bool bIsConditionalRequest);
    void finishRequest();
    void setRelyingPartyId(const std::string &rpId);
    void handleRequestFailure(QWebEngineWebAuthUxRequest::RequestFailureReason reason);

    QScopedPointer<AuthenticatorRequestDialogControllerPrivate> d_ptr;
    friend class AuthenticatorRequestClientDelegateQt;
};
}

#endif // AUTHENTICATOR_REQUEST_DIALOG_CONTROLLER_H
