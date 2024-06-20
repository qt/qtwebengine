// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginewebauthuxrequest.h"
#include "qwebenginewebauthuxrequest_p.h"
#include "authenticator_request_dialog_controller.h"

/*!
    \qmltype WebEngineWebAuthUxRequest
    \nativetype QWebEngineWebAuthUxRequest
    \inqmlmodule QtWebEngine
    \since QtWebEngine 6.7
    \brief Encapsulates the data of a WebAuth UX request.

    Web engine's WebAuth UX requests are passed to the user in the
    \l WebEngineView::webAuthUxRequested() signal.

    For more information about how to handle web engine authenticator requests, see the
    \l{WebEngine Quick Nano Browser}{Nano Browser}.
*/

/*!
    \class QWebEngineWebAuthUxRequest
    \brief The QWebEngineWebAuthUxRequest class encapsulates the data of a WebAuth UX request.
    \since 6.7

    \inmodule QtWebEngineCore

    This class contains the information and API for WebAuth UX. WebAuth may require user
    interaction during the authentication process. These requests are handled by displaying a
    dialog to users. QtWebEngine currently supports user verification, resident credentials,
    and display request failure UX requests.

    QWebEngineWebAuthUxRequest models a WebAuth UX request throughout its life cycle,
    starting with showing a UX dialog, updating it's content through state changes, and
    finally closing the dialog.

    WebAuth UX requests are normally triggered when the authenticator requires user interaction.
    It is the QWebEnginePage's responsibility to notify the application of the new WebAuth UX
    requests, which it does by emitting the
    \l{QWebEnginePage::webAuthUxRequested}{webAuthUxRequested} signal together with a newly
    created QWebEngineWebAuthUxRequest. The application can then examine this request and
    display a WebAuth UX dialog.

    The QWebEngineWebAuthUxRequest object periodically emits the \l
    {QWebEngineWebAuthUxRequest::}{stateChanged} signal to notify potential
    observers of the current WebAuth UX states. The observers update the WebAuth dialog
    accordingly.

    For more information about how to handle web engine authenticator requests, see the
     \l{WebEngine Widgets Simple Browser Example}{Simple Browser}.
*/

/*!
    \struct QWebEngineWebAuthPinRequest
    \brief The QWebEngineWebAuthPinRequest class encapsulates the data of a PIN WebAuth UX request.
    \since 6.7

    \inmodule QtWebEngineCore

    This encapsulates the following information related to a PIN request made by an authenticator.
    \list
    \li The reason for the PIN prompt.
    \li The error details for the PIN prompt.
    \li The number of attempts remaining before a hard lock. Should be ignored unless
        \l{QWebEngineWebAuthPinRequest::reason} is
        \l{QWebEngineWebAuthUxRequest::PinEntryReason::Challenge}.
    \li The minimum PIN length the authenticator will accept for the PIN.
    \endlist
    Use this structure to update the WebAuth UX dialog when the WebAuth UX state is \l
    QWebEngineWebAuthUxRequest::WebAuthUxState::CollectPin.
*/

/*!
    \property QWebEngineWebAuthPinRequest::reason
    \brief The reason for the PIN prompt.
*/

/*!
    \property QWebEngineWebAuthPinRequest::error
    \brief The error details for the PIN prompt.
*/

/*!
    \property QWebEngineWebAuthPinRequest::remainingAttempts
    \brief The number of attempts remaining before a hard lock. Should be ignored unless
           \l{QWebEngineWebAuthPinRequest::reason} is
           \l{QWebEngineWebAuthUxRequest::PinEntryReason::Challenge}.
*/

/*!
    \property QWebEngineWebAuthPinRequest::minPinLength
    \brief The minimum PIN length the authenticator will accept for the PIN.
*/

/*!
    \qmlvaluetype webEngineWebAuthPinRequest
    \ingroup qmlvaluetypes
    \inqmlmodule QtWebEngine
    \since QtWebEngine 6.8
    \brief Encapsulates the data of a PIN WebAuth UX request.

    This encapsulates the following information related to a PIN request made by an authenticator.
    \list
    \li The reason for the PIN prompt.
    \li The error details for the PIN prompt.
    \li The number of attempts remaining before a hard lock. Should be ignored unless
        \l{webEngineWebAuthPinRequest::reason} is
        \c{WebEngineWebAuthUxRequest.PinEntryReason.Challenge}.
    \li The minimum PIN length that the authenticator will accept for the PIN.
    \endlist
    Use this structure to update the WebAuth UX dialog when the WebAuth UX state is \l
    WebEngineWebAuthUxRequest.WebAuthUxState.CollectPin.
*/

/*!
    \qmlproperty enumeration QtWebEngine::webEngineWebAuthPinRequest::reason
    \brief The reason for the PIN prompt.

    \value WebEngineWebAuthUxRequest.PinEntryReason.Set A new PIN is being set.
    \value WebEngineWebAuthUxRequest.PinEntryReason.Change The existing PIN must be changed before using this authenticator.
    \value WebEngineWebAuthUxRequest.PinEntryReason.Challenge The existing PIN is being collected to prove user verification.
*/

/*!
    \qmlproperty enumeration QtWebEngine::webEngineWebAuthPinRequest::error
    \brief The error details for the PIN prompt.

    \value WebEngineWebAuthUxRequest.PinEntryError.NoError No error has occurred.
    \value WebEngineWebAuthUxRequest.PinEntryError.InternalUvLocked Internal UV is locked, so we are falling back to PIN.
    \value WebEngineWebAuthUxRequest.PinEntryError.WrongPin The PIN the user entered does not match the authenticator PIN.
    \value WebEngineWebAuthUxRequest.PinEntryError.TooShort The new PIN the user entered is too short.
    \value WebEngineWebAuthUxRequest.PinEntryError.InvalidCharacters The new PIN the user entered contains invalid characters.
    \value WebEngineWebAuthUxRequest.PinEntryError.SameAsCurrentPin The new PIN the user entered is the same as the currently set PIN.
*/

/*!
    \qmlproperty int QtWebEngine::webEngineWebAuthPinRequest::remainingAttempts
    \brief The number of attempts remaining before a hard lock. Should be ignored unless
           \l{WebEngineWebAuthPinRequest::reason} is
           \c{WebEngineWebAuthUxRequest.PinEntryReason.Challenge}.
*/

/*!
    \qmlproperty int QtWebEngine::webEngineWebAuthPinRequest::minPinLength
    \brief The minimum PIN length that the authenticator will accept for the PIN.
*/

/*!
    \enum QWebEngineWebAuthUxRequest::WebAuthUxState

    This enum describes the state of the current WebAuth UX request.

    \value NotStarted WebAuth UX request not started yet.
    \value SelectAccount The authenticator requires resident credential details.
           The application needs to display an account details dialog, and
           the user needs to select an account to proceed.
    \value CollectPin The authenticator requires user verification.
           The application needs to display a PIN request dialog.
    \value FinishTokenCollection The authenticator requires token/user verification (like tap on
           the FIDO key) to complete the process.
    \value RequestFailed WebAuth request failed. Display error details.
    \value Cancelled  WebAuth request is cancelled. Close the WebAuth dialog.
    \value Completed WebAuth request is completed. Close the WebAuth dialog.
*/

/*!
    \enum QWebEngineWebAuthUxRequest::PinEntryReason

    This enum describes the reasons that may prompt the authenticator to ask for a PIN.

    \value Set A new PIN is being set.
    \value Change The existing PIN must be changed before using this authenticator.
    \value Challenge The existing PIN is being collected to prove user verification.
*/

/*!
    \enum QWebEngineWebAuthUxRequest::PinEntryError

    This enum describes the errors that may prompt the authenticator to ask for a PIN.

    \value NoError No error has occurred.
    \value InternalUvLocked Internal UV is locked, so we are falling back to PIN.
    \value WrongPin The PIN the user entered does not match the authenticator PIN.
    \value TooShort The new PIN the user entered is too short.
    \value InvalidCharacters The new PIN the user entered contains invalid characters.
    \value SameAsCurrentPin The new PIN the user entered is the same as the currently set PIN.
*/

/*!
    \enum QWebEngineWebAuthUxRequest::RequestFailureReason

    This enum describes the reason for WebAuth request failure.

    \value Timeout The authentication session has timed out.
    \value KeyNotRegistered Key is not registered with the authenticator.
    \value KeyAlreadyRegistered Key is already registered with the authenticator.
           Try to register with another Key or use another authenticator.
    \value SoftPinBlock The authenticator is blocked as the user entered the wrong key many times.
    \value HardPinBlock The authenticator is blocked as the user entered the wrong key many times
           and reset the PIN to use the specific authenticator again.
    \value AuthenticatorRemovedDuringPinEntry Authenticator removed during PIN entry.
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
    \fn void QWebEngineWebAuthUxRequest::stateChanged(WebAuthUxState state)

    This signal is emitted whenever the WebAuth UX's \a state changes.

    \sa state, WebAuthUxState
*/

/*!
    \qmlsignal void WebEngineWebAuthUxRequest::stateChanged(WebAuthUxState state)
    This signal is emitted whenever the WebAuth UX's \a state changes.

    \sa state, QWebEngineWebAuthUxRequest::WebAuthUxState
*/

/*! \internal
 */
QWebEngineWebAuthUxRequestPrivate::QWebEngineWebAuthUxRequestPrivate(
        QtWebEngineCore::AuthenticatorRequestDialogController *controller)
    : webAuthDialogController(controller)
{
    m_currentState = controller->state();
}

/*! \internal
 */
QWebEngineWebAuthUxRequestPrivate::~QWebEngineWebAuthUxRequestPrivate() { }

/*! \internal
 */
QWebEngineWebAuthUxRequest::QWebEngineWebAuthUxRequest(QWebEngineWebAuthUxRequestPrivate *p)
    : d_ptr(p)
{
    connect(d_ptr->webAuthDialogController,
            &QtWebEngineCore::AuthenticatorRequestDialogController::stateChanged,
            [this](WebAuthUxState currentState) {
                Q_D(QWebEngineWebAuthUxRequest);
                d->m_currentState = currentState;
                Q_EMIT stateChanged(d->m_currentState);
            });
}

/*! \internal
 */
QWebEngineWebAuthUxRequest::~QWebEngineWebAuthUxRequest() { }

/*!
    \qmlproperty stringlist WebEngineWebAuthUxRequest::userNames
    \brief The available user names for the resident credential support.

    This is needed when the current WebAuth request's UX state is
    WebEngineWebAuthUxRequest.WebAuthUxState.SelectAccount. The
    WebAuth dialog displays user names. The user needs to select an
    account to proceed.

    \sa state setSelectedAccount() QWebEngineWebAuthUxRequest::userNames
*/
/*!
    \property QWebEngineWebAuthUxRequest::userNames
    \brief The available user names for the resident credential support.
    This is needed when the current WebAuth request's UX state is \l SelectAccount.
    The WebAuth dialog displays user names. The user needs to select an account to proceed.

    \sa SelectAccount setSelectedAccount()
*/
QStringList QWebEngineWebAuthUxRequest::userNames() const
{
    const Q_D(QWebEngineWebAuthUxRequest);

    return d->webAuthDialogController->userNames();
}

/*!
    \qmlproperty string WebEngineWebAuthUxRequest::relyingPartyId
    \brief The WebAuth request's relying party id.
*/
/*!
    \property QWebEngineWebAuthUxRequest::relyingPartyId
    \brief The WebAuth request's relying party id.
*/
QString QWebEngineWebAuthUxRequest::relyingPartyId() const
{
    const Q_D(QWebEngineWebAuthUxRequest);

    return d->webAuthDialogController->relyingPartyId();
}

/*!
    \qmlproperty QWebEngineWebAuthPinRequest WebEngineWebAuthUxRequest::pinRequest
    \brief The WebAuth request's PIN request information.

    \sa QWebEngineWebAuthPinRequest
*/
/*!
    \property QWebEngineWebAuthUxRequest::pinRequest
    \brief The WebAuth request's PIN request information.

    This is needed when the current WebAuth request state is \l CollectPin.
    WebAuth Dialog displays a PIN request dialog. The user needs to enter a PIN and
    invoke \l setPin() to proceed.

    \sa QWebEngineWebAuthPinRequest CollectPin setPin()
*/
QWebEngineWebAuthPinRequest QWebEngineWebAuthUxRequest::pinRequest() const
{
    const Q_D(QWebEngineWebAuthUxRequest);

    return d->webAuthDialogController->pinRequest();
}

/*!
    \qmlproperty enumeration WebEngineWebAuthUxRequest::state
    \brief The WebAuth request's current UX state.

    \value WebEngineWebAuthUxRequest.WebAuthUxState.NotStarted WebAuth UX request not started yet.
    \value WebEngineWebAuthUxRequest.WebAuthUxState.SelectAccount The authenticator requires
           resident credential details. The application needs to display an account details dialog,
           and the user needs to select an account to proceed.
    \value WebEngineWebAuthUxRequest.WebAuthUxState.CollectPin The authenticator requires user verification.
           The application needs to display a PIN request dialog.
    \value WebEngineWebAuthUxRequest.WebAuthUxState.FinishTokenCollection The authenticator requires
           token/user verification (like tap on the FIDO key) to complete the process.
    \value WebEngineWebAuthUxRequest.WebAuthUxState.RequestFailed WebAuth request failed. Display error details.
    \value WebEngineWebAuthUxRequest.WebAuthUxState.Cancelled  WebAuth request is cancelled.
           Close the WebAuth dialog.
    \value WebEngineWebAuthUxRequest.WebAuthUxState.Completed WebAuth request is completed.
           Close the WebAuth dialog.
*/
/*!
    \property QWebEngineWebAuthUxRequest::state
    \brief The WebAuth request's current UX state.

    \l stateChanged() is emitted when the current state changes.
    Update the WebAuth dialog in reponse to the changes in state.
*/
QWebEngineWebAuthUxRequest::WebAuthUxState QWebEngineWebAuthUxRequest::state() const
{
    return d_ptr->m_currentState;
}

/*!
    \qmlmethod void WebEngineWebAuthUxRequest::setSelectedAccount(const QString &selectedAccount)

    Sends the \a selectedAccount name to the authenticator.
    This is needed when the current WebAuth request's UX state is
    WebEngineWebAuthUxRequest.WebAuthUxState.SelectAccount. The
    WebAuth request is blocked until the user selects an account and
    invokes this method.

    \sa WebEngineWebAuthUxRequest::userNames state
*/
/*!
    Sends the \a selectedAccount name to the authenticator.
    This is needed when the current WebAuth request's UX state is \l SelectAccount.
    The WebAuth request is blocked until the user selects an account and invokes this method.

    \sa userNames SelectAccount
*/
void QWebEngineWebAuthUxRequest::setSelectedAccount(const QString &selectedAccount)
{
    Q_D(QWebEngineWebAuthUxRequest);

    d->webAuthDialogController->sendSelectAccountResponse(selectedAccount);
}

/*!
    \qmlmethod void WebEngineWebAuthUxRequest::setPin(const QString &pin)
    Sends the \a pin to the authenticator that prompts for a PIN.
    This is needed when the current WebAuth request's UX state is
    WebEngineWebAuthUxRequest.WebAuthUxState.CollectPin. The WebAuth
    request is blocked until the user responds with a PIN.

    \sa QWebEngineWebAuthPinRequest state
*/
/*!
    Sends the \a pin to the authenticator that prompts for a PIN.
    This is needed when the current WebAuth request's UX state is \l CollectPin.
    The WebAuth request is blocked until the user responds with a PIN.

    \sa QWebEngineWebAuthPinRequest CollectPin
*/
void QWebEngineWebAuthUxRequest::setPin(const QString &pin)
{
    Q_D(QWebEngineWebAuthUxRequest);
    d->webAuthDialogController->sendCollectPinResponse(pin);
}

/*!
    \qmlmethod void WebEngineWebAuthUxRequest::cancel()
    Cancels the current WebAuth request.

    \sa QWebEngineWebAuthUxRequest::Cancelled, WebEngineWebAuthUxRequest::stateChanged()
*/
/*!
    Cancels the current WebAuth request.

    \sa QWebEngineWebAuthUxRequest::Cancelled, stateChanged()
*/
void QWebEngineWebAuthUxRequest::cancel()
{
    Q_D(QWebEngineWebAuthUxRequest);

    d->webAuthDialogController->reject();
}

/*!
    \qmlmethod void WebEngineWebAuthUxRequest::retry()
    Retries the current WebAuth request.

    \sa  stateChanged()
*/
/*!
    Retries the current WebAuth request.

    \sa  stateChanged()
*/
void QWebEngineWebAuthUxRequest::retry()
{
    const Q_D(QWebEngineWebAuthUxRequest);

    d->webAuthDialogController->retryRequest();
}

/*!
    \qmlproperty enumeration WebEngineWebAuthUxRequest::requestFailureReason
    \brief The WebAuth request's failure reason.

    \value WebEngineWebAuthUxRequest.RequestFailureReason.Timeout The authentication session has timed out.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.KeyNotRegistered Key is not registered with the authenticator.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.KeyAlreadyRegistered Key is already registered with
           the authenticator. Try to register with another key or use another authenticator.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.SoftPinBlock The authenticator is blocked as the user
           entered the wrong key many times.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.HardPinBlock The authenticator is blocked as the user entered
           the wrong key many times and reset the PIN to use the specific authenticator again.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.AuthenticatorRemovedDuringPinEntry Authenticator
           removed during PIN entry.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.AuthenticatorMissingResidentKeys Authenticator doesn't
           have resident key support.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.AuthenticatorMissingUserVerification Authenticator doesn't
           have user verification support.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.AuthenticatorMissingLargeBlob Authenticator doesn't have
           large blob support.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.NoCommonAlgorithms No common algorithm.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.StorageFull The resident credential could not be created
           because the authenticator has insufficient storage.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.UserConsentDenied User consent denied.
    \value WebEngineWebAuthUxRequest.RequestFailureReason.WinUserCancelled The user clicked \uicontrol Cancel
           in the native windows UI.

    \sa  stateChanged()
*/
/*!
    \property QWebEngineWebAuthUxRequest::requestFailureReason
    \brief The WebAuth request's failure reason.

    \sa  stateChanged()  QWebEngineWebAuthUxRequest::RequestFailureReason
*/
QWebEngineWebAuthUxRequest::RequestFailureReason
QWebEngineWebAuthUxRequest::requestFailureReason() const
{
    const Q_D(QWebEngineWebAuthUxRequest);

    return d->webAuthDialogController->requestFailureReason();
}

#include "moc_qwebenginewebauthuxrequest.cpp"
