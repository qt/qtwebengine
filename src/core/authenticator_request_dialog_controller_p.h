// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTHENTICATOR_REQUEST_DIALOG_CONTROLLER_P_H
#define AUTHENTICATOR_REQUEST_DIALOG_CONTROLLER_P_H
#include <QStringList>
#include <QSharedPointer>
#include "authenticator_request_client_delegate_qt.h"
#include "authenticator_request_dialog_controller.h"

namespace content {
class WebContents;
class RenderFrameHost;
}

namespace QtWebEngineCore {

class AuthenticatorRequestDialogControllerPrivate
{

public:
    AuthenticatorRequestDialogControllerPrivate(
            content::RenderFrameHost *renderFrameHost,
            base::WeakPtr<AuthenticatorRequestClientDelegateQt> authenticatorRequestDelegate);
    ~AuthenticatorRequestDialogControllerPrivate();
    void showWebAuthDialog();
    void selectAccount(const QStringList &userList);
    QStringList userNames() const;
    QString relyingPartyId() const;
    QWebEngineWebAuthUxRequest::WebAuthUxState state() const;
    QWebEngineWebAuthPinRequest pinRequest();
    QWebEngineWebAuthUxRequest::RequestFailureReason requestFailureReason() const;
    void sendSelectAccountResponse(const QString &selectedAccount);
    void setCurrentState(QWebEngineWebAuthUxRequest::WebAuthUxState uxState);
    void setRelyingPartyId(const QString &rpId);

    // Support pin functionality
    void collectPin(QWebEngineWebAuthPinRequest pinRequestInfo);
    void finishCollectToken();
    void handleRequestFailure(QWebEngineWebAuthUxRequest::RequestFailureReason reason);
    void sendCollectPinResponse(const QString &pin);

    // Deleting dialog;
    void finishRequest();

    // cancel request
    void cancelRequest();
    void retryRequest();
    void startRequest(bool isConditionalRequest);

    AuthenticatorRequestDialogController *q_ptr;

private:
    content::RenderFrameHost *m_renderFrameHost;
    QStringList m_userList;
    QString m_pin;
    QString m_relyingPartyId;

    bool m_isStarted = false;
    bool m_isConditionalRequest = false;
    QWebEngineWebAuthUxRequest::WebAuthUxState m_currentState =
            QWebEngineWebAuthUxRequest::WebAuthUxState::NotStarted;
    base::WeakPtr<AuthenticatorRequestClientDelegateQt> m_authenticatorRequestDelegate;
    bool m_isDialogCreated = false;
    QWebEngineWebAuthPinRequest m_pinRequest;

    QWebEngineWebAuthUxRequest *m_request = nullptr;
    QWebEngineWebAuthUxRequest::RequestFailureReason m_requestFailureReason;

    // m_pendingState holds requested steps until the UI is shown. The UI is only
    // shown once the TransportAvailabilityInfo is available, but authenticators
    // may request, e.g., PIN entry prior to that.
    absl::optional<QWebEngineWebAuthUxRequest::WebAuthUxState> m_pendingState;
};

} // namespace QtWebEngineCore

#endif // AUTHENTICATOR_REQUEST_DIALOG_CONTROLLER_P_H
