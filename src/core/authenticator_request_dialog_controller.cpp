// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "authenticator_request_dialog_controller.h"
#include "authenticator_request_dialog_controller_p.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browser_task_traits.h"
#include "web_contents_delegate_qt.h"
#include "qwebenginewebauthuxrequest_p.h"
#include "qwebenginewebauthuxrequest.h"

using PINEntryError = QWebEngineWebAuthUXRequest::PINEntryError;
using PINEntryReason = QWebEngineWebAuthUXRequest::PINEntryReason;

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(PINEntryReason::Set, device::pin::PINEntryReason::kSet)
ASSERT_ENUMS_MATCH(PINEntryReason::Change, device::pin::PINEntryReason::kChange)
ASSERT_ENUMS_MATCH(PINEntryReason::Challenge, device::pin::PINEntryReason::kChallenge)
ASSERT_ENUMS_MATCH(PINEntryError::WrongPIN, device::pin::PINEntryError::kWrongPIN)
ASSERT_ENUMS_MATCH(PINEntryError::TooShort, device::pin::PINEntryError::kTooShort)
ASSERT_ENUMS_MATCH(PINEntryError::SameAsCurrentPIN, device::pin::PINEntryError::kSameAsCurrentPIN)
ASSERT_ENUMS_MATCH(PINEntryError::NoError, device::pin::PINEntryError::kNoError)
ASSERT_ENUMS_MATCH(PINEntryError::InvalidCharacters, device::pin::PINEntryError::kInvalidCharacters)
ASSERT_ENUMS_MATCH(PINEntryError::InternalUvLocked, device::pin::PINEntryError::kInternalUvLocked)

AuthenticatorRequestDialogControllerPrivate::AuthenticatorRequestDialogControllerPrivate(
        content::RenderFrameHost *renderFrameHost,
        base::WeakPtr<AuthenticatorRequestClientDelegateQt> authenticatorRequestDelegate)
    : m_renderFrameHost(renderFrameHost)
    , m_authenticatorRequestDelegate(authenticatorRequestDelegate)
{
}

AuthenticatorRequestDialogControllerPrivate::~AuthenticatorRequestDialogControllerPrivate()
{
    if (m_request) {
        delete m_request;
        m_request = nullptr;
    }
}

void AuthenticatorRequestDialogControllerPrivate::showWebAuthDialog()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    content::WebContents *webContent = content::WebContents::FromRenderFrameHost(m_renderFrameHost);

    if (!webContent)
        return;

    WebContentsAdapterClient *adapterClient = nullptr;
    if (webContent)
        adapterClient =
                static_cast<WebContentsDelegateQt *>(webContent->GetDelegate())->adapterClient();

    if (adapterClient) {

        QWebEngineWebAuthUXRequestPrivate *itemPrivate =
                new QWebEngineWebAuthUXRequestPrivate(q_ptr);

        m_request = new QWebEngineWebAuthUXRequest(itemPrivate);

        adapterClient->showWebAuthDialog(m_request);
        m_isDialogCreated = true;
    } else {
        cancelRequest();
    }
}

void AuthenticatorRequestDialogControllerPrivate::selectAccount(const QStringList &userList)
{
    m_userList.clear();
    m_userList = userList;
    setCurrentState(QWebEngineWebAuthUXRequest::SelectAccount);
}

void AuthenticatorRequestDialogControllerPrivate::collectPIN(QWebEngineWebAuthPINRequest pinRequest)
{
    m_pinRequest = pinRequest;
    setCurrentState(QWebEngineWebAuthUXRequest::CollectPIN);
}

void AuthenticatorRequestDialogControllerPrivate::finishCollectToken()
{
    setCurrentState(QWebEngineWebAuthUXRequest::FinishTokenCollection);
}

QStringList AuthenticatorRequestDialogControllerPrivate::userNames() const
{
    return m_userList;
}

void AuthenticatorRequestDialogControllerPrivate::finishRequest()
{
    if (!m_isDialogCreated)
        return;
    setCurrentState(QWebEngineWebAuthUXRequest::Completed);
}

void AuthenticatorRequestDialogControllerPrivate::setCurrentState(
        QWebEngineWebAuthUXRequest::WebAuthUXState uxState)
{
    if (!m_isStarted) {
        // Dialog isn't showing yet. Remember to show this step when it appears.
        m_pendingState = uxState;
        return;
    }

    m_currentState = uxState;

    if (m_isConditionalRequest)
        return;

    if (!m_isDialogCreated) {
        showWebAuthDialog();
    } else {
        Q_EMIT q_ptr->stateChanged(m_currentState);

        if (m_currentState == QWebEngineWebAuthUXRequest::Cancelled
            || m_currentState == QWebEngineWebAuthUXRequest::Completed) {
            m_isDialogCreated = false;
        }
    }
}

void AuthenticatorRequestDialogControllerPrivate::cancelRequest()
{
    setCurrentState(QWebEngineWebAuthUXRequest::Cancelled);
    content::GetUIThreadTaskRunner({})->PostTask(
            FROM_HERE,
            base::BindOnce(&AuthenticatorRequestClientDelegateQt::onCancelRequest,
                           m_authenticatorRequestDelegate));
}

void AuthenticatorRequestDialogControllerPrivate::retryRequest()
{
    content::GetUIThreadTaskRunner({})->PostTask(
            FROM_HERE,
            base::BindOnce(&AuthenticatorRequestClientDelegateQt::onRetryRequest,
                           m_authenticatorRequestDelegate));
}

void AuthenticatorRequestDialogControllerPrivate::sendSelectAccountResponse(
        const QString &selectedAccount)
{
    content::GetUIThreadTaskRunner({})->PostTask(
            FROM_HERE,
            base::BindOnce(&AuthenticatorRequestClientDelegateQt::onSelectAccount,
                           m_authenticatorRequestDelegate, selectedAccount));
}

QWebEngineWebAuthUXRequest::WebAuthUXState
AuthenticatorRequestDialogControllerPrivate::state() const
{
    return m_currentState;
}

void AuthenticatorRequestDialogControllerPrivate::startRequest(bool isConditionalRequest)
{
    DCHECK(!m_isStarted);

    m_isStarted = true;
    m_isConditionalRequest = isConditionalRequest;

    if (m_pendingState) {
        setCurrentState(*m_pendingState);
        m_pendingState.reset();
    }
}

void AuthenticatorRequestDialogControllerPrivate::setRelyingPartyId(const QString &rpId)
{
    m_relyingPartyId = rpId;
}

QString AuthenticatorRequestDialogControllerPrivate::relyingPartyId() const
{
    return m_relyingPartyId;
}

QWebEngineWebAuthPINRequest AuthenticatorRequestDialogControllerPrivate::pinRequest()
{
    return m_pinRequest;
}

void AuthenticatorRequestDialogControllerPrivate::handleRequestFailure(
        QWebEngineWebAuthUXRequest::RequestFailureReason reason)
{
    m_requestFailureReason = reason;
    setCurrentState(QWebEngineWebAuthUXRequest::RequestFailed);
}

void AuthenticatorRequestDialogControllerPrivate::sendCollectPinResponse(const QString &pin)
{
    content::GetUIThreadTaskRunner({})->PostTask(
            FROM_HERE,
            base::BindOnce(&AuthenticatorRequestClientDelegateQt::onCollectPin,
                           m_authenticatorRequestDelegate, pin));
}

QWebEngineWebAuthUXRequest::RequestFailureReason
AuthenticatorRequestDialogControllerPrivate::requestFailureReason() const
{
    return m_requestFailureReason;
}

AuthenticatorRequestDialogController::AuthenticatorRequestDialogController(
        AuthenticatorRequestDialogControllerPrivate *dd)
{
    Q_ASSERT(dd);
    d_ptr.reset(dd);
    d_ptr->q_ptr = this;
}

AuthenticatorRequestDialogController::~AuthenticatorRequestDialogController() { }

void AuthenticatorRequestDialogController::selectAccount(const QStringList &userList)
{
    d_ptr->selectAccount(userList);
}

void AuthenticatorRequestDialogController::collectPIN(QWebEngineWebAuthPINRequest pinRequest)
{
    d_ptr->collectPIN(pinRequest);
}

QStringList AuthenticatorRequestDialogController::userNames() const
{
    return d_ptr->userNames();
}

QWebEngineWebAuthPINRequest AuthenticatorRequestDialogController::pinRequest()
{
    return d_ptr->pinRequest();
}

void AuthenticatorRequestDialogController::reject()
{
    d_ptr->cancelRequest();
}

void AuthenticatorRequestDialogController::sendSelectAccountResponse(const QString &account)
{
    d_ptr->sendSelectAccountResponse(account);
}

void AuthenticatorRequestDialogController::finishCollectToken()
{
    d_ptr->finishCollectToken();
}

void AuthenticatorRequestDialogController::finishRequest()
{
    d_ptr->finishRequest();
}

QWebEngineWebAuthUXRequest::WebAuthUXState AuthenticatorRequestDialogController::state() const
{
    return d_ptr->state();
}

void AuthenticatorRequestDialogController::startRequest(bool bIsConditionalRequest)
{
    d_ptr->startRequest(bIsConditionalRequest);
}

void AuthenticatorRequestDialogController::setRelyingPartyId(const std::string &rpId)
{
    d_ptr->setRelyingPartyId(QString::fromStdString(rpId));
}

QString AuthenticatorRequestDialogController::relyingPartyId() const
{
    return d_ptr->relyingPartyId();
}

void AuthenticatorRequestDialogController::handleRequestFailure(
        QWebEngineWebAuthUXRequest::RequestFailureReason reason)
{
    d_ptr->handleRequestFailure(reason);
}

void AuthenticatorRequestDialogController::retryRequest()
{
    d_ptr->retryRequest();
}

void AuthenticatorRequestDialogController::sendCollectPinResponse(const QString &pin)
{
    d_ptr->sendCollectPinResponse(pin);
}

QWebEngineWebAuthUXRequest::RequestFailureReason
AuthenticatorRequestDialogController::requestFailureReason() const
{
    return d_ptr->requestFailureReason();
}
}
