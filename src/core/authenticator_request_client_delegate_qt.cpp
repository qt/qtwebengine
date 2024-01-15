// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "authenticator_request_client_delegate_qt.h"
#include "authenticator_request_dialog_controller.h"
#include "authenticator_request_dialog_controller_p.h"
#include "base/base64.h"
#include "profile_adapter_client.h"
#include "profile_qt.h"
#include "content/public/browser/web_contents.h"
#include "web_contents_delegate_qt.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

using RequestFailureReason = QWebEngineWebAuthUxRequest::RequestFailureReason;

WebAuthenticationDelegateQt::WebAuthenticationDelegateQt() = default;

WebAuthenticationDelegateQt::~WebAuthenticationDelegateQt() = default;

bool WebAuthenticationDelegateQt::SupportsResidentKeys(content::RenderFrameHost *render_frame_host)
{
    return true;
}

AuthenticatorRequestClientDelegateQt::AuthenticatorRequestClientDelegateQt(
        content::RenderFrameHost *render_frame_host)
    : m_renderFrameHost(render_frame_host), m_weakFactory(this)
{
    m_dialogController.reset(new AuthenticatorRequestDialogController(
            new AuthenticatorRequestDialogControllerPrivate(m_renderFrameHost,
                                                            m_weakFactory.GetWeakPtr())));
}

AuthenticatorRequestClientDelegateQt::~AuthenticatorRequestClientDelegateQt()
{
    // Currently WebAuth request is completed. Now it is possible to delete the dialog if displayed
    m_dialogController->finishRequest();
}

void AuthenticatorRequestClientDelegateQt::SetRelyingPartyId(const std::string &rp_id)
{
    m_dialogController->setRelyingPartyId(rp_id);
}

bool AuthenticatorRequestClientDelegateQt::DoesBlockRequestOnFailure(
        InterestingFailureReason reason)
{
    if (!IsWebAuthnUIEnabled())
        return false;

    switch (reason) {
    case InterestingFailureReason::kTimeout:
        m_dialogController->handleRequestFailure(RequestFailureReason::Timeout);
        break;
    case InterestingFailureReason::kAuthenticatorMissingResidentKeys:
        m_dialogController->handleRequestFailure(
                RequestFailureReason::AuthenticatorMissingResidentKeys);
        break;
    case InterestingFailureReason::kAuthenticatorMissingUserVerification:
        m_dialogController->handleRequestFailure(
                RequestFailureReason::AuthenticatorMissingUserVerification);
        break;
    case InterestingFailureReason::kAuthenticatorMissingLargeBlob:
        m_dialogController->handleRequestFailure(
                RequestFailureReason::AuthenticatorMissingLargeBlob);
        break;
    case InterestingFailureReason::kAuthenticatorRemovedDuringPINEntry:
        m_dialogController->handleRequestFailure(
                RequestFailureReason::AuthenticatorRemovedDuringPinEntry);
        break;
    case InterestingFailureReason::kHardPINBlock:
        m_dialogController->handleRequestFailure(RequestFailureReason::HardPinBlock);
        break;
    case InterestingFailureReason::kSoftPINBlock:
        m_dialogController->handleRequestFailure(RequestFailureReason::SoftPinBlock);
        break;
    case InterestingFailureReason::kKeyAlreadyRegistered:
        m_dialogController->handleRequestFailure(RequestFailureReason::KeyAlreadyRegistered);
        break;
    case InterestingFailureReason::kKeyNotRegistered:
        m_dialogController->handleRequestFailure(RequestFailureReason::KeyNotRegistered);
        break;
    case InterestingFailureReason::kNoCommonAlgorithms:
        m_dialogController->handleRequestFailure(RequestFailureReason::NoCommonAlgorithms);
        break;
    case InterestingFailureReason::kStorageFull:
        m_dialogController->handleRequestFailure(RequestFailureReason::StorageFull);
        break;
    case InterestingFailureReason::kUserConsentDenied:
        m_dialogController->handleRequestFailure(RequestFailureReason::UserConsentDenied);
        break;
    case InterestingFailureReason::kWinUserCancelled:
#if BUILDFLAG(IS_WIN)
        m_dialogController->handleRequestFailure(RequestFailureReason::WinUserCancelled);
#else
        return false;
#endif
        break;
    default:
        return false;
    }
    return true;
}
void AuthenticatorRequestClientDelegateQt::RegisterActionCallbacks(
        base::OnceClosure cancel_callback, base::RepeatingClosure start_over_callback,
        AccountPreselectedCallback account_preselected_callback,
        device::FidoRequestHandlerBase::RequestCallback request_callback,
        base::RepeatingClosure bluetooth_adapter_power_on_callback)
{
    m_cancelCallback = std::move(cancel_callback);
    m_startOverCallback = std::move(start_over_callback);
    m_accountPreselectedCallback = std::move(account_preselected_callback);
    m_requestCallback = std::move(request_callback);
    m_bluetoothAdapterPowerOnCallback = std::move(bluetooth_adapter_power_on_callback);
}

void AuthenticatorRequestClientDelegateQt::ShouldReturnAttestation(
        const std::string &relying_party_id, const device::FidoAuthenticator *authenticator,
        bool is_enterprise_attestation, base::OnceCallback<void(bool)> callback)
{
    std::move(callback).Run(!is_enterprise_attestation);
}

void AuthenticatorRequestClientDelegateQt::SelectAccount(
        std::vector<device::AuthenticatorGetAssertionResponse> responses,
        base::OnceCallback<void(device::AuthenticatorGetAssertionResponse)> callback)
{
    if (m_isUiDisabled) {
        // Requests with UI disabled should never reach account selection.
        DCHECK(IsVirtualEnvironmentEnabled());
        std::move(callback).Run(std::move(responses.at(0)));
        return;
    }

    if (m_isConditionalRequest) {
        return;
    }

    m_authenticatorGetAssertionResponse = std::move(responses);
    m_selectAccountCallback = std::move(callback);

    QStringList userList;
    int nIndex = -1;
    for (const auto &response : m_authenticatorGetAssertionResponse) {
        nIndex++;
        const auto &user_entity = response.user_entity;
        const bool has_user_identifying_info = user_entity && user_entity->name;
        if (has_user_identifying_info) {
            QString userName = toQt(*response.user_entity->name);
            m_userMap[userName] = nIndex;
            userList.append(userName);
        }
    }
    m_dialogController->selectAccount(userList);
}

void AuthenticatorRequestClientDelegateQt::DisableUI()
{
    m_isUiDisabled = true;
}

bool AuthenticatorRequestClientDelegateQt::IsWebAuthnUIEnabled()
{
    return !m_isUiDisabled;
}

void AuthenticatorRequestClientDelegateQt::SetConditionalRequest(bool is_conditional)
{
    m_isConditionalRequest = is_conditional;
}

// This method will not be invoked until the observer is set.
void AuthenticatorRequestClientDelegateQt::OnTransportAvailabilityEnumerated(
        device::FidoRequestHandlerBase::TransportAvailabilityInfo data)
{
    // Show dialog only after this step;
    // If m_isUiDisabled is set or another UI request in progress return
    if (m_isUiDisabled
        || m_dialogController->state() != QWebEngineWebAuthUxRequest::WebAuthUxState::NotStarted)
        return;

    // Start WebAuth UX
    // we may need to pass data as well. for SelectAccount and SupportPin it is not required,
    // skipping that for the timebeing.
    m_dialogController->startRequest(m_isConditionalRequest);
}

bool AuthenticatorRequestClientDelegateQt::SupportsPIN() const
{
    return true;
}

void AuthenticatorRequestClientDelegateQt::CollectPIN(
        CollectPINOptions options, base::OnceCallback<void(std::u16string)> provide_pin_cb)
{

    m_providePinCallback = std::move(provide_pin_cb);
    QWebEngineWebAuthPinRequest pinRequestInfo;

    pinRequestInfo.reason = static_cast<QWebEngineWebAuthUxRequest::PinEntryReason>(options.reason);
    pinRequestInfo.error = static_cast<QWebEngineWebAuthUxRequest::PinEntryError>(options.error);
    pinRequestInfo.remainingAttempts = options.attempts;
    pinRequestInfo.minPinLength = options.min_pin_length;
    m_dialogController->collectPin(pinRequestInfo);
}

void AuthenticatorRequestClientDelegateQt::FinishCollectToken()
{
    m_dialogController->finishCollectToken();
}

void AuthenticatorRequestClientDelegateQt::onCancelRequest()
{
    if (!m_cancelCallback)
        return;

    std::move(m_cancelCallback).Run();
}

void AuthenticatorRequestClientDelegateQt::onSelectAccount(const QString &selectedAccount)
{
    if (!m_selectAccountCallback)
        return;

    if (m_userMap.find(selectedAccount) != m_userMap.end()) {
        std::move(m_selectAccountCallback)
                .Run(std::move(m_authenticatorGetAssertionResponse.at(m_userMap[selectedAccount])));
    } else {
        onCancelRequest();
    }
}

void AuthenticatorRequestClientDelegateQt::onCollectPin(const QString &pin)
{
    if (!m_providePinCallback)
        return;
    std::move(m_providePinCallback).Run(pin.toStdU16String());
}

void AuthenticatorRequestClientDelegateQt::onRetryRequest()
{
    DCHECK(m_startOverCallback);
    m_startOverCallback.Run();
}
}
