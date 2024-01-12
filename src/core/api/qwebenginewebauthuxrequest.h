// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEWEBAUTHUXREQUEST_H
#define QWEBENGINEWEBAUTHUXREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QWebEngineWebAuthUXRequestPrivate;
struct QWebEngineWebAuthPINRequest;

class Q_WEBENGINECORE_EXPORT QWebEngineWebAuthUXRequest : public QObject
{
    Q_OBJECT
public:
    QWebEngineWebAuthUXRequest(QWebEngineWebAuthUXRequestPrivate *);
    ~QWebEngineWebAuthUXRequest();

    enum WebAuthUXState {
        NotStarted,
        SelectAccount,
        CollectPIN,
        FinishTokenCollection,
        RequestFailed,
        Cancelled,
        Completed
    };
    Q_ENUM(WebAuthUXState)

    enum class PINEntryReason : int {
        Set,
        Change,
        Challenge
    };
    Q_ENUM(PINEntryReason)

    enum class PINEntryError : int {
        NoError,
        InternalUvLocked,
        WrongPIN,
        TooShort,
        InvalidCharacters,
        SameAsCurrentPIN,
    };
    Q_ENUM(PINEntryError)

    enum class RequestFailureReason : int {
        Timeout,
        KeyNotRegistered,
        KeyAlreadyRegistered,
        SoftPINBlock,
        HardPINBlock,
        AuthenticatorRemovedDuringPINEntry,
        AuthenticatorMissingResidentKeys,
        AuthenticatorMissingUserVerification,
        AuthenticatorMissingLargeBlob,
        NoCommonAlgorithms,
        StorageFull,
        UserConsentDenied,
        WinUserCancelled,
    };
    Q_ENUM(RequestFailureReason)

    Q_PROPERTY(QStringList userNames READ userNames CONSTANT FINAL)
    Q_PROPERTY(WebAuthUXState state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString relyingPartyId READ relyingPartyId CONSTANT FINAL)
    Q_PROPERTY(QWebEngineWebAuthPINRequest pinRequest READ pinRequest CONSTANT FINAL)
    Q_PROPERTY(RequestFailureReason requestFailureReason READ requestFailureReason CONSTANT FINAL)

    QStringList userNames() const;
    QString relyingPartyId() const;
    QWebEngineWebAuthPINRequest pinRequest() const;
    WebAuthUXState state() const;
    RequestFailureReason requestFailureReason() const;

Q_SIGNALS:
    void stateChanged(QWebEngineWebAuthUXRequest::WebAuthUXState state);

public Q_SLOTS:
    void cancel();
    void retry();
    void setSelectedAccount(const QString &selectedAccount);
    void setPin(const QString &pin);

private Q_SLOTS:
    void handleUXUpdate(WebAuthUXState currentState);

protected:
    QScopedPointer<QWebEngineWebAuthUXRequestPrivate> d_ptr;

    Q_DECLARE_PRIVATE(QWebEngineWebAuthUXRequest)
};

struct Q_WEBENGINECORE_EXPORT QWebEngineWebAuthPINRequest
{
    Q_GADGET

    Q_PROPERTY(QWebEngineWebAuthUXRequest::PINEntryReason reason MEMBER reason CONSTANT FINAL)
    Q_PROPERTY(QWebEngineWebAuthUXRequest::PINEntryError error MEMBER error CONSTANT FINAL)
    Q_PROPERTY(qint32 minPinLength MEMBER minPinLength CONSTANT FINAL)
    Q_PROPERTY(qint32 remainingAttempts MEMBER remainingAttempts CONSTANT FINAL)
public:
    QWebEngineWebAuthUXRequest::PINEntryReason reason;
    QWebEngineWebAuthUXRequest::PINEntryError error =
            QWebEngineWebAuthUXRequest::PINEntryError::NoError;
    qint32 minPinLength;
    int remainingAttempts = 0;
};

QT_END_NAMESPACE

#endif // QWEBENGINEWEBAUTHUXREQUEST_H
