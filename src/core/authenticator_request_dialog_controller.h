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
    QWebEngineWebAuthPINRequest pinRequest();
    void reject();
    AuthenticatorRequestDialogController(AuthenticatorRequestDialogControllerPrivate *);

    QWebEngineWebAuthUXRequest::WebAuthUXState state() const;
    QString relyingPartyId() const;
    void retryRequest();
    QWebEngineWebAuthUXRequest::RequestFailureReason requestFailureReason() const;

Q_SIGNALS:
    void stateChanged(QWebEngineWebAuthUXRequest::WebAuthUXState state);

private:
    void selectAccount(const QStringList &userList);
    void collectPIN(QWebEngineWebAuthPINRequest pinRequest);
    void finishCollectToken();
    void startRequest(bool bIsConditionalRequest);
    void finishRequest();
    void setRelyingPartyId(const std::string &rpId);
    void handleRequestFailure(QWebEngineWebAuthUXRequest::RequestFailureReason reason);

    QScopedPointer<AuthenticatorRequestDialogControllerPrivate> d_ptr;
    friend class AuthenticatorRequestClientDelegateQt;
};
}

#endif // AUTHENTICATOR_REQUEST_DIALOG_CONTROLLER_H
