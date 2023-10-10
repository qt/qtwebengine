// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginecertificateerror.h"

#include "certificate_error_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineCertificateError
    \brief The QWebEngineCertificateError class provides information about a certificate error.
    \since 5.4
    \inmodule QtWebEngineCore

    Provides information about a certificate error. This class is used as a parameter of
    QWebEnginePage::certificateError().
*/

/*! \internal
*/
QWebEngineCertificateError::QWebEngineCertificateError(
        const QSharedPointer<QtWebEngineCore::CertificateErrorController> &controller)
    : d(controller)
{
}

QWebEngineCertificateError::QWebEngineCertificateError(const QWebEngineCertificateError &) = default;

QWebEngineCertificateError& QWebEngineCertificateError::operator=(const QWebEngineCertificateError &) = default;

/*! \internal
*/
QWebEngineCertificateError::~QWebEngineCertificateError() = default;

/*!
    \enum QWebEngineCertificateError::Type

    This enum describes the type of certificate error encountered.

    The values of this enum type match the SSL errors Chromium provides.
    QSslError::SslError values are not used directly, because the Qt error
    categories cannot be mapped to the Chromium error categories.

    \value SslPinnedKeyNotInCertificateChain The certificate did not match the built-in public keys
            pinned for the host name.
    \value CertificateCommonNameInvalid The certificate's common name did not match the host name.
    \value CertificateDateInvalid The certificate is not valid at the current date and time.
    \value CertificateAuthorityInvalid The certificate is not signed by a trusted authority.
    \value CertificateContainsErrors The certificate contains errors.
    \value CertificateNoRevocationMechanism The certificate has no mechanism for determining if it has been revoked.
    \value CertificateUnableToCheckRevocation Revocation information for the certificate is not available.
    \value CertificateRevoked The certificate has been revoked.
    \value CertificateInvalid The certificate is invalid.
    \value CertificateWeakSignatureAlgorithm The certificate is signed using a weak signature algorithm.
    \value CertificateNonUniqueName The host name specified in the certificate is not unique.
    \value CertificateWeakKey The certificate contains a weak key.
    \value CertificateNameConstraintViolation The certificate claimed DNS names that are in violation of name constraints.
    \value CertificateValidityTooLong The certificate has a validity period that is too long. (Added in Qt 5.7)
    \value CertificateTransparencyRequired Certificate Transparency was required for this connection, but the server
            did not provide CT information that complied with the policy. (Added in Qt 5.8)
    \value CertificateKnownInterceptionBlocked The certificate is known to be
            used for interception by an entity other the device owner. (Added in
            5.15)
    \value SslObsoleteVersion The connection uses an obsolete version of SSL/TLS. (Added in Qt 6.2, deprecated in Qt 6.4)
    \value CertificateSymantecLegacy The certificate is a legacy Symantec one that's no longer valid. (Added in Qt 6.2)
*/

/*!
    \property QWebEngineCertificateError::overridable
    \brief Whether this error can be overridden and accepted.

    \sa description()
*/
bool QWebEngineCertificateError::isOverridable() const
{
    return d->overridable();
}

/*!
    Returns the URL that triggered the error.

    \sa description()
*/
QUrl QWebEngineCertificateError::url() const
{
    return d->url();
}

/*!
    Returns the type of the error.

    \sa description(), isOverridable()
*/
QWebEngineCertificateError::Type QWebEngineCertificateError::type() const
{
    return d->error();
}

/*!
    Returns a short localized human-readable description of the error.

    \sa url(), isOverridable()
*/
QString QWebEngineCertificateError::description() const
{
    return d->errorString();
}

/*!
    \since 5.14

    Marks the certificate error for delayed handling.

    This function should be called when there is a need to postpone the decision whether to accept a
    certificate, for example, while waiting for user input. When called, the function pauses the
    URL request until acceptCertificate() or rejectCertificate() is called.

    \note It is only possible to defer overridable certificate errors.

    \sa isOverridable()
*/
void QWebEngineCertificateError::defer()
{
    d->defer();
}

/*!
    \since 5.14

    Accepts the certificate and continues the loading of the requested URL.
*/
void QWebEngineCertificateError::acceptCertificate()
{
    d->ignoreCertificateError();
}

/*!
    \since 5.14

    Rejects the certificate and aborts the loading of the requested URL.
*/
void QWebEngineCertificateError::rejectCertificate()
{
    d->rejectCertificate();
}

/*!
    \since 5.14

    Returns the peer's chain of digital certificates.

    Chain starts with the peer's immediate certificate and ending with the CA's certificate.
*/
QList<QSslCertificate> QWebEngineCertificateError::certificateChain() const
{
    return d->certificateChain();
}

QT_END_NAMESPACE

#include "moc_qwebenginecertificateerror.cpp"
