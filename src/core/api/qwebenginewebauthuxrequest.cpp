// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginewebauthuxrequest.h"
#include "qwebenginewebauthuxrequest_p.h"
#include "authenticator_request_dialog_controller.h"

/*!
    \qmltype WebEngineWebAuthUXRequest
    \instantiates QWebEngineWebAuthUXRequest
    \inqmlmodule QtWebEngine
    \since QtWebEngine 6.7
    \brief Encapsulates the data of a WebAuth UX request.

    Web engine's WebAuth UX requests are passed to the user in the
    \l WebEngineView::webAuthUXRequested() signal.

    For more information about how to handle web engine authenticator requests, see the
    \l{WebEngine Quick Nano Browser}{Nano Browser}.
*/

/*!
    \class QWebEngineWebAuthUXRequest
    \brief The QWebEngineWebAuthUXRequest class encapsulates the data of a WebAuth UX request.
    \since 6.7

    \inmodule QtWebEngineCore

    This class contains the information and API for WebAuth UX. WebAuth may require user
    interaction during the authentication process. These requests are handled by displaying a
    dialog to users. QtWebEngine currently supports user verification, resident credentials,
    and display request failure UX requests.

    QWebEngineWebAuthUXRequest models a WebAuth UX request throughout its life cycle,
    starting with showing a UX dialog, updating it's content through state changes, and
    finally closing the dialog.

    WebAuth UX requests are normally triggered when the authenticator requires user interaction.
    It is the QWebEnginePage's responsibility to notify the application of the new WebAuth UX
    requests, which it does by emitting the
    \l{QWebEnginePage::webAuthUXRequested}{webAuthUXRequested} signal together with a newly
    created QWebEngineWebAuthUXRequest. The application can then examine this request and
    display a WebAuth UX dialog.

    The QWebEngineWebAuthUXRequest object periodically emits the \l
    {QWebEngineWebAuthUXRequest::}{stateChanged} signal to notify potential
    observers of the current WebAuth UX states. The observers update the WebAuth dialog
    accordingly.

    For more information about how to handle web engine authenticator requests, see the
     \l{WebEngine Widgets Simple Browser Example}{Simple Browser}.
*/

/*!
    \struct QWebEngineWebAuthPINRequest
    \brief The QWebEngineWebAuthPINRequest class encapsulates the data of a PIN WebAuth UX request.
    \since 6.7

    \inmodule QtWebEngineCore

    This encapsulates the following information related to a PIN request made by an authenticator.
    \list
    \li The reason for the PIN prompt.
    \li The error details for the PIN prompt.
    \li The number of attempts remaining before a hard lock. Should be ignored unless
        \l{QWebEngineWebAuthPINRequest::reason} is
        \l{QWebEngineWebAuthUXRequest::PINEntryReason::Challenge}.
    \li The minimum PIN length the authenticator will accept for the PIN.
    \endlist
    Use this structure to update the WebAuth UX dialog when the WebAuth UX state is \l
    QWebEngineWebAuthUXRequest::CollectPIN.
*/

/*!
    \property QWebEngineWebAuthPINRequest::reason
    \brief The reason for the PIN prompt.
*/

/*!
    \property QWebEngineWebAuthPINRequest::error
    \brief The error details for the PIN prompt.
*/

/*!
    \property QWebEngineWebAuthPINRequest::remainingAttempts
    \brief The number of attempts remaining before a hard lock. Should be ignored unless
           \l{QWebEngineWebAuthPINRequest::reason} is
           \l{QWebEngineWebAuthUXRequest::PINEntryReason::Challenge}.
*/

/*!
    \property QWebEngineWebAuthPINRequest::minPinLength
    \brief The minimum PIN length the authenticator will accept for the PIN.
*/

/*!
    \enum QWebEngineWebAuthUXRequest::WebAuthUXState

    This enum describes the state of the current WebAuth UX request.

    \value NotStarted WebAuth UX request not started yet.
    \value SelectAccount The authenticator requires resident credential details.
           The application needs to display an account details dialog, and
           the user needs to select an account to proceed.
    \value CollectPIN The authenticator requires user verification.
           The application needs to display a PIN request dialog.
    \value FinishTokenCollection The authenticator requires token/user verification (like tap on
           the FIDO key) to complete the process.
    \value RequestFailed WebAuth request failed. Display error details.
    \value Cancelled  WebAuth request is cancelled. Close the WebAuth dialog.
    \value Completed WebAuth request is completed. Close the WebAuth dialog.
*/

/*!
    \enum QWebEngineWebAuthUXRequest::PINEntryReason

    This enum describes the reasons that may prompt the authenticator to ask for a PIN.

    \value Set A new PIN is being set.
    \value Change The existing PIN must be changed before using this authenticator.
    \value Challenge The existing PIN is being collected to prove user verification.
*/

/*!
    \enum QWebEngineWebAuthUXRequest::PINEntryError

    This enum describes the errors that may prompt the authenticator to ask for a PIN.

    \value NoError No error has occurred.
    \value InternalUvLocked Internal UV is locked, so we are falling back to PIN.
    \value WrongPIN The PIN the user entered does not match the authenticator PIN.
    \value TooShort The new PIN the user entered is too short.
    \value InvalidCharacters The new PIN the user entered contains invalid characters.
    \value SameAsCurrentPIN The new PIN the user entered is the same as the currently set PIN.
*/

/*!
    \enum QWebEngineWebAuthUXRequest::RequestFailureReason

    This enum describes the reason for WebAuth request failure.

    \value Timeout The authentication session has timed out.
    \value KeyNotRegistered Key is not registered with the authenticator.
    \value KeyAlreadyRegistered Key is already registered with the authenticator.
           Try to register with another Key or use another authenticator.
    \value SoftPINBlock The authenticator is blocked as the user entered the wrong key many times.
    \value HardPINBlock The authenticator is blocked as the user entered the wrong key many times
           and reset the PIN to use the specific authenticator again.
    \value AuthenticatorRemovedDuringPINEntry Authenticator removed during PIN entry.
    \value AuthenticatorMissingResidentKeys Authenticator doesn't have resident key support.
    \value AuthenticatorMissingUserVerification Authenticator doesn't
           have user verification support.
    \value AuthenticatorMissingLargeBlob Authenticator doesn't have large blob support.
    \value NoCommonAlgorithms No common algorithm.
    \value StorageFull The resident credential could not be created because the
           authenticator has insufficient storage.
    \value UserConsentDenied User consent denied.
    \value WinUserCancelled The user clicked \uicontrol Cancel in the native windows UI.
*/

/*!
    \fn void QWebEngineWebAuthUXRequest::stateChanged(WebAuthUXState state)

    This signal is emitted whenever the WebAuth UX's \a state changes.

    \sa state, WebAuthUXState
*/

/*!
    \qmlsignal void WebEngineWebAuthUXRequest::stateChanged(WebAuthUXState state)
    This signal is emitted whenever the WebAuth UX's \a state changes.

    \sa state, QWebEngineWebAuthUXRequest::WebAuthUXState
*/

/*! \internal
 */
QWebEngineWebAuthUXRequestPrivate::QWebEngineWebAuthUXRequestPrivate(
        QtWebEngineCore::AuthenticatorRequestDialogController *controller)
    : webAuthDialogController(controller)
{
    m_currentState = controller->state();
}

/*! \internal
 */
QWebEngineWebAuthUXRequestPrivate::~QWebEngineWebAuthUXRequestPrivate() { }

/*! \internal
 */
void QWebEngineWebAuthUXRequest::handleUXUpdate(WebAuthUXState currentState)
{
    Q_D(QWebEngineWebAuthUXRequest);

    d->m_currentState = currentState;

    Q_EMIT stateChanged(d->m_currentState);
}

/*! \internal
 */
QWebEngineWebAuthUXRequest::QWebEngineWebAuthUXRequest(QWebEngineWebAuthUXRequestPrivate *p)
    : d_ptr(p)
{
    connect(d_ptr->webAuthDialogController,
            &QtWebEngineCore::AuthenticatorRequestDialogController::stateChanged, this,
            &QWebEngineWebAuthUXRequest::handleUXUpdate);
}

/*! \internal
 */
QWebEngineWebAuthUXRequest::~QWebEngineWebAuthUXRequest() { }

/*!
    \qmlproperty stringlist WebEngineWebAuthUXRequest::userNames
    \brief The available user names for the resident credential support.

    This is needed when the current WebAuth request's UX state is
    WebEngineWebAuthUXRequest.SelectAccount. The WebAuth dialog displays user names.
    The user needs to select an account to proceed.

    \sa state setSelectedAccount() QWebEngineWebAuthUXRequest::userNames
*/
/*!
    \property QWebEngineWebAuthUXRequest::userNames
    \brief The available user names for the resident credential support.
    This is needed when the current WebAuth request's UX state is \l SelectAccount.
    The WebAuth dialog displays user names. The user needs to select an account to proceed.

    \sa SelectAccount setSelectedAccount()
*/
QStringList QWebEngineWebAuthUXRequest::userNames() const
{
    const Q_D(QWebEngineWebAuthUXRequest);

    return d->webAuthDialogController->userNames();
}

/*!
    \qmlproperty string WebEngineWebAuthUXRequest::relyingPartyId
    \brief The WebAuth request's relying party id.
*/
/*!
    \property QWebEngineWebAuthUXRequest::relyingPartyId
    \brief The WebAuth request's relying party id.
*/
QString QWebEngineWebAuthUXRequest::relyingPartyId() const
{
    const Q_D(QWebEngineWebAuthUXRequest);

    return d->webAuthDialogController->relyingPartyId();
}

/*!
    \qmlproperty QWebEngineWebAuthPINRequest WebEngineWebAuthUXRequest::pinRequest
    \brief The WebAuth request's PIN request information.

    \sa QWebEngineWebAuthPINRequest
*/
/*!
    \property QWebEngineWebAuthUXRequest::pinRequest
    \brief The WebAuth request's PIN request information.

    This is needed when the current WebAuth request state is \l CollectPIN.
    WebAuth Dialog displays a PIN request dialog. The user needs to enter a PIN and
    invoke \l setPin() to proceed.

    \sa QWebEngineWebAuthPINRequest CollectPIN setPin()
*/
QWebEngineWebAuthPINRequest QWebEngineWebAuthUXRequest::pinRequest() const
{
    const Q_D(QWebEngineWebAuthUXRequest);

    return d->webAuthDialogController->pinRequest();
}

/*!
    \qmlproperty enumeration WebEngineWebAuthUXRequest::state
    \brief The WebAuth request's current UX state.

    \value WebEngineWebAuthUXRequest.NotStarted WebAuth UX request not started yet.
    \value WebEngineWebAuthUXRequest.SelectAccount The authenticator requires
           resident credential details. The application needs to display an account details dialog,
           and the user needs to select an account to proceed.
    \value WebEngineWebAuthUXRequest.CollectPIN The authenticator requires user verification.
           The application needs to display a PIN request dialog.
    \value WebEngineWebAuthUXRequest.FinishTokenCollection The authenticator requires
           token/user verification (like tap on the FIDO key) to complete the process.
    \value WebEngineWebAuthUXRequest.RequestFailed WebAuth request failed. Display error details.
    \value WebEngineWebAuthUXRequest.Cancelled  WebAuth request is cancelled.
           Close the WebAuth dialog.
    \value WebEngineWebAuthUXRequest.Completed WebAuth request is completed.
           Close the WebAuth dialog.
*/
/*!
    \property QWebEngineWebAuthUXRequest::state
    \brief The WebAuth request's current UX state.

    \l stateChanged() is emitted when the current state changes.
    Update the WebAuth dialog in reponse to the changes in state.
*/
QWebEngineWebAuthUXRequest::WebAuthUXState QWebEngineWebAuthUXRequest::state() const
{
    return d_ptr->m_currentState;
}

/*!
    \qmlmethod void WebEngineWebAuthUXRequest::setSelectedAccount(const QString &selectedAccount)
    Sends the \a selectedAccount name to the authenticator.
    This is needed when the current WebAuth request's UX state is
    WebEngineWebAuthUXRequest.SelectAccount. The WebAuth request is blocked until the user selects
    an account and invokes this method.

    \sa WebEngineWebAuthUXRequest::userNames state
*/
/*!
    Sends the \a selectedAccount name to the authenticator.
    This is needed when the current WebAuth request's UX state is \l SelectAccount.
    The WebAuth request is blocked until the user selects an account and invokes this method.

    \sa userNames SelectAccount
*/
void QWebEngineWebAuthUXRequest::setSelectedAccount(const QString &selectedAccount)
{
    Q_D(QWebEngineWebAuthUXRequest);

    d->webAuthDialogController->sendSelectAccountResponse(selectedAccount);
}

/*!
    \qmlmethod void WebEngineWebAuthUXRequest::setPin(const QString &pin)
    Sends the \a pin to the authenticator that prompts for a PIN.
    This is needed when the current WebAuth request's UX state is
    WebEngineWebAuthUXRequest.CollectPIN. The WebAuth request is blocked until
    the user responds with a PIN.

    \sa QWebEngineWebAuthPINRequest state
*/
/*!
    Sends the \a pin to the authenticator that prompts for a PIN.
    This is needed when the current WebAuth request's UX state is \l CollectPIN.
    The WebAuth request is blocked until the user responds with a PIN.

    \sa QWebEngineWebAuthPINRequest CollectPIN
*/
void QWebEngineWebAuthUXRequest::setPin(const QString &pin)
{
    Q_D(QWebEngineWebAuthUXRequest);
    d->webAuthDialogController->sendCollectPinResponse(pin);
}

/*!
    \qmlmethod void WebEngineWebAuthUXRequest::cancel()
    Cancels the current WebAuth request.

    \sa QWebEngineWebAuthUXRequest::Cancelled, WebEngineWebAuthUXRequest::stateChanged()
*/
/*!
    Cancels the current WebAuth request.

    \sa QWebEngineWebAuthUXRequest::Cancelled, stateChanged()
*/
void QWebEngineWebAuthUXRequest::cancel()
{
    Q_D(QWebEngineWebAuthUXRequest);

    d->webAuthDialogController->reject();
}

/*!
    \qmlmethod void WebEngineWebAuthUXRequest::retry()
    Retries the current WebAuth request.

    \sa  stateChanged()
*/
/*!
    Retries the current WebAuth request.

    \sa  stateChanged()
*/
void QWebEngineWebAuthUXRequest::retry()
{
    const Q_D(QWebEngineWebAuthUXRequest);

    d->webAuthDialogController->retryRequest();
}

/*!
    \qmlproperty enumeration WebEngineWebAuthUXRequest::requestFailureReason
    \brief The WebAuth request's failure reason.

    \value WebEngineWebAuthUXRequest.Timeout The authentication session has timed out.
    \value WebEngineWebAuthUXRequest.KeyNotRegistered Key is not registered with the authenticator.
    \value WebEngineWebAuthUXRequest.KeyAlreadyRegistered Key is already registered with
           the authenticator. Try to register with another key or use another authenticator.
    \value WebEngineWebAuthUXRequest.SoftPINBlock The authenticator is blocked as the user
           entered the wrong key many times.
    \value WebEngineWebAuthUXRequest.HardPINBlock The authenticator is blocked as the user entered
           the wrong key many times and reset the PIN to use the specific authenticator again.
    \value WebEngineWebAuthUXRequest.AuthenticatorRemovedDuringPINEntry Authenticator
           removed during PIN entry.
    \value WebEngineWebAuthUXRequest.AuthenticatorMissingResidentKeys Authenticator doesn't
           have resident key support.
    \value WebEngineWebAuthUXRequest.AuthenticatorMissingUserVerification Authenticator doesn't
           have user verification support.
    \value WebEngineWebAuthUXRequest.AuthenticatorMissingLargeBlob Authenticator doesn't have
           large blob support.
    \value WebEngineWebAuthUXRequest.NoCommonAlgorithms No common algorithm.
    \value WebEngineWebAuthUXRequest.StorageFull The resident credential could not be created
           because the authenticator has insufficient storage.
    \value WebEngineWebAuthUXRequest.UserConsentDenied User consent denied.
    \value WebEngineWebAuthUXRequest.WinUserCancelled The user clicked \uicontrol Cancel
           in the native windows UI.

    \sa  stateChanged()
*/
/*!
    \property QWebEngineWebAuthUXRequest::requestFailureReason
    \brief The WebAuth request's failure reason.

    \sa  stateChanged()  QWebEngineWebAuthUXRequest::RequestFailureReason
*/
QWebEngineWebAuthUXRequest::RequestFailureReason
QWebEngineWebAuthUXRequest::requestFailureReason() const
{
    const Q_D(QWebEngineWebAuthUXRequest);

    return d->webAuthDialogController->requestFailureReason();
}

#include "moc_qwebenginewebauthuxrequest.cpp"
