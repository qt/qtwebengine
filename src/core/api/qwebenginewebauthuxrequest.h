// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEWEBAUTHUXREQUEST_H
#define QWEBENGINEWEBAUTHUXREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qobject.h>

namespace QtWebEngineCore {
class AuthenticatorRequestDialogControllerPrivate;
}

QT_BEGIN_NAMESPACE

class QWebEngineWebAuthUxRequestPrivate;
struct QWebEngineWebAuthPinRequest;

class Q_WEBENGINECORE_EXPORT QWebEngineWebAuthUxRequest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList userNames READ userNames CONSTANT FINAL)
    Q_PROPERTY(WebAuthUxState state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString relyingPartyId READ relyingPartyId CONSTANT FINAL)
    Q_PROPERTY(QWebEngineWebAuthPinRequest pinRequest READ pinRequest CONSTANT FINAL)
    Q_PROPERTY(RequestFailureReason requestFailureReason READ requestFailureReason CONSTANT FINAL)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
public:
    enum class WebAuthUxState {
        NotStarted,
        SelectAccount,
        CollectPin,
        FinishTokenCollection,
        RequestFailed,
        Cancelled,
        Completed,
    };
    Q_ENUM(WebAuthUxState)

    enum class PinEntryReason {
        Set,
        Change,
        Challenge,
    };
    Q_ENUM(PinEntryReason)

    enum class PinEntryError {
        NoError,
        InternalUvLocked,
        WrongPin,
        TooShort,
        InvalidCharacters,
        SameAsCurrentPin,
    };
    Q_ENUM(PinEntryError)

    enum class RequestFailureReason {
        Timeout,
        KeyNotRegistered,
        KeyAlreadyRegistered,
        SoftPinBlock,
        HardPinBlock,
        AuthenticatorRemovedDuringPinEntry,
        AuthenticatorMissingResidentKeys,
        AuthenticatorMissingUserVerification,
        AuthenticatorMissingLargeBlob,
        NoCommonAlgorithms,
        StorageFull,
        UserConsentDenied,
        WinUserCancelled,
    };
    Q_ENUM(RequestFailureReason)

    ~QWebEngineWebAuthUxRequest() override;

    QStringList userNames() const;
    QString relyingPartyId() const;
    QWebEngineWebAuthPinRequest pinRequest() const;
    WebAuthUxState state() const;
    RequestFailureReason requestFailureReason() const;

Q_SIGNALS:
    void stateChanged(QWebEngineWebAuthUxRequest::WebAuthUxState state);

public Q_SLOTS:
    void cancel();
    void retry();
    void setSelectedAccount(const QString &selectedAccount);
    void setPin(const QString &pin);

protected:
    explicit QWebEngineWebAuthUxRequest(QWebEngineWebAuthUxRequestPrivate *);

    std::unique_ptr<QWebEngineWebAuthUxRequestPrivate> d_ptr;
    friend class QtWebEngineCore::AuthenticatorRequestDialogControllerPrivate;
    Q_DECLARE_PRIVATE(QWebEngineWebAuthUxRequest)
};

struct QWebEngineWebAuthPinRequest
{
    Q_GADGET_EXPORT(Q_WEBENGINECORE_EXPORT)

    Q_PROPERTY(QWebEngineWebAuthUxRequest::PinEntryReason reason MEMBER reason CONSTANT FINAL)
    Q_PROPERTY(QWebEngineWebAuthUxRequest::PinEntryError error MEMBER error CONSTANT FINAL)
    Q_PROPERTY(qint32 minPinLength MEMBER minPinLength CONSTANT FINAL)
    Q_PROPERTY(int remainingAttempts MEMBER remainingAttempts CONSTANT FINAL)
public:
    QWebEngineWebAuthUxRequest::PinEntryReason reason;
    QWebEngineWebAuthUxRequest::PinEntryError error;
    qint32 minPinLength;
    int remainingAttempts;
};

QT_END_NAMESPACE

#endif // QWEBENGINEWEBAUTHUXREQUEST_H
