// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTHENTICATOR_REQUEST_CLIENT_DELEGATE_QT_H
#define AUTHENTICATOR_REQUEST_CLIENT_DELEGATE_QT_H

#include "qtwebenginecoreglobal_p.h"
#include "content/public/browser/authenticator_request_client_delegate.h"
#include <unordered_map>
#include <QSharedPointer>

namespace QtWebEngineCore {

class AuthenticatorRequestDialogController;

class WebAuthenticationDelegateQt : public content::WebAuthenticationDelegate
{
public:
    WebAuthenticationDelegateQt();
    virtual ~WebAuthenticationDelegateQt();

    bool SupportsResidentKeys(content::RenderFrameHost *render_frame_host) override;
};

class AuthenticatorRequestClientDelegateQt : public content::AuthenticatorRequestClientDelegate
{
public:
    explicit AuthenticatorRequestClientDelegateQt(content::RenderFrameHost *render_frame_host);
    AuthenticatorRequestClientDelegateQt(const AuthenticatorRequestClientDelegateQt &) = delete;
    AuthenticatorRequestClientDelegateQt &
    operator=(const AuthenticatorRequestClientDelegateQt &) = delete;
    ~AuthenticatorRequestClientDelegateQt();

    // content::AuthenticatorRequestClientDelegate ovverrides
    void SetRelyingPartyId(const std::string &rp_id) override;
    bool DoesBlockRequestOnFailure(InterestingFailureReason reason) override;
    void RegisterActionCallbacks(base::OnceClosure cancel_callback,
                            base::RepeatingClosure start_over_callback,
                            AccountPreselectedCallback account_preselected_callback,
                            device::FidoRequestHandlerBase::RequestCallback request_callback,
                            base::RepeatingClosure bluetooth_adapter_power_on_callback) override;
    void ShouldReturnAttestation(const std::string &relying_party_id,
                                 const device::FidoAuthenticator *authenticator,
                                 bool is_enterprise_attestation,
                                 base::OnceCallback<void(bool)> callback) override;
    void SelectAccount(
            std::vector<device::AuthenticatorGetAssertionResponse> responses,
            base::OnceCallback<void(device::AuthenticatorGetAssertionResponse)> callback) override;
    void DisableUI() override;
    bool IsWebAuthnUIEnabled() override;
    void SetConditionalRequest(bool is_conditional) override;

    // device::FidoRequestHandlerBase::Observer overrides:
    // This method will not be invoked until the observer is set.
    void OnTransportAvailabilityEnumerated(
            device::FidoRequestHandlerBase::TransportAvailabilityInfo data) override;

    bool SupportsPIN() const override;
    void CollectPIN(CollectPINOptions options,
                    base::OnceCallback<void(std::u16string)> provide_pin_cb) override;
    void FinishCollectToken() override;

    // Dialog helper
    void onCancelRequest();
    void onSelectAccount(const QString &selectedAccount);
    void onCollectPin(const QString &pin);
    void onRetryRequest();

private:
    content::RenderFrameHost *m_renderFrameHost;
    bool m_isUiDisabled = false;
    bool m_isConditionalRequest = false;

    base::OnceClosure m_cancelCallback;
    base::RepeatingClosure m_startOverCallback;
    AccountPreselectedCallback m_accountPreselectedCallback;
    device::FidoRequestHandlerBase::RequestCallback m_requestCallback;
    base::RepeatingClosure m_bluetoothAdapterPowerOnCallback;

    // Select account details;
    std::vector<device::AuthenticatorGetAssertionResponse> m_authenticatorGetAssertionResponse;
    std::unordered_map<QString, int> m_userMap;
    base::OnceCallback<void(device::AuthenticatorGetAssertionResponse)> m_selectAccountCallback;

    // collect pin
    base::OnceCallback<void(std::u16string)> m_providePinCallback;

    // This member is used to keep authenticator request dialog controller alive until
    // authenticator request is completed or cancelled.
    QSharedPointer<AuthenticatorRequestDialogController> m_dialogController;
    base::WeakPtrFactory<AuthenticatorRequestClientDelegateQt> m_weakFactory;
};

}

#endif // AUTHENTICATOR_REQUEST_CLIENT_DELEGATE_QT_H
