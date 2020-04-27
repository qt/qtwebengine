/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginecertificateerror.h"

#include "certificate_error_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineCertificateError
    \brief The QWebEngineCertificateError class provides information about a certificate error.
    \since 5.4
    \inmodule QtWebEngineWidgets

    Provides information about a certificate error. This class is used as a parameter of
    QWebEnginePage::certificateError().
*/

class QWebEngineCertificateErrorPrivate : public QSharedData {
public:
    QWebEngineCertificateErrorPrivate(int error, QUrl url, bool overridable, QString errorDescription);

    ~QWebEngineCertificateErrorPrivate() {
        if (deferred && !answered)
            rejectCertificate();
    }

    void resolveError(bool accept) {
        if (answered)
            return;
        answered = true;
        if (overridable) {
            if (auto ctl = controller.lock())
                ctl->accept(accept);
        }
    }

    void ignoreCertificateError() { resolveError(true); }
    void rejectCertificate() { resolveError(false); }

    QWebEngineCertificateError::Error error;
    QUrl url;
    bool overridable;
    QString errorDescription;
    QList<QSslCertificate> certificateChain;

    bool answered = false, deferred = false;
    QWeakPointer<CertificateErrorController> controller;

    Q_DISABLE_COPY(QWebEngineCertificateErrorPrivate)
};

QWebEngineCertificateErrorPrivate::QWebEngineCertificateErrorPrivate(int error, QUrl url, bool overridable, QString errorDescription)
    : error(QWebEngineCertificateError::Error(error))
    , url(url)
    , overridable(overridable)
    , errorDescription(errorDescription)
{ }

/*! \internal
*/
QWebEngineCertificateError::QWebEngineCertificateError(int error, QUrl url, bool overridable, QString errorDescription)
    : d(new QWebEngineCertificateErrorPrivate(error, url, overridable, errorDescription))
{ }

/*! \internal
*/
QWebEngineCertificateError::QWebEngineCertificateError(const QSharedPointer<CertificateErrorController> &controller)
    : d(new QWebEngineCertificateErrorPrivate(controller->error(), controller->url(),
                                              controller->overridable(), controller->errorString()))
{
    d->controller = controller;
    d->certificateChain = controller->certificateChain();
}

QWebEngineCertificateError::QWebEngineCertificateError(const QWebEngineCertificateError &) = default;

QWebEngineCertificateError& QWebEngineCertificateError::operator=(const QWebEngineCertificateError &) = default;

/*! \internal
*/
QWebEngineCertificateError::~QWebEngineCertificateError()
{

}

/*!
    \enum QWebEngineCertificateError::Error

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
*/

/*!
    Returns whether this error can be overridden and accepted.

    \sa error(), errorDescription()
*/
bool QWebEngineCertificateError::isOverridable() const
{
    return d->overridable;
}

/*!
    Returns the URL that triggered the error.

    \sa error(), errorDescription()
*/
QUrl QWebEngineCertificateError::url() const
{
    return d->url;
}

/*!
    Returns the type of the error.

    \sa errorDescription(), isOverridable()
*/
QWebEngineCertificateError::Error QWebEngineCertificateError::error() const
{
    return d->error;
}

/*!
    Returns a short localized human-readable description of the error.

    \sa error(), url(), isOverridable()
*/
QString QWebEngineCertificateError::errorDescription() const
{
    return d->errorDescription;
}

/*!
    \since 5.14

    Marks the certificate error for delayed handling.

    This function should be called when there is a need to postpone the decision whether to ignore a
    certificate error, for example, while waiting for user input. When called, the function pauses the
    URL request until ignoreCertificateError() or rejectCertificate() is called.

    \note It is only possible to defer overridable certificate errors.

    \sa isOverridable(), deferred()
*/
void QWebEngineCertificateError::defer()
{
    if (isOverridable())
        d->deferred = true;
}

/*!
    \since 5.14

    Returns whether the decision for error handling was delayed and the URL load was halted.
*/
bool QWebEngineCertificateError::deferred() const
{
    return d->deferred;
}

/*!
    \since 5.14

    Ignores the certificate error and continues the loading of the requested URL.
*/
void QWebEngineCertificateError::ignoreCertificateError()
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

    Returns \c true if the error was explicitly rejected or ignored.
*/
bool QWebEngineCertificateError::answered() const
{
    return d->answered;
}

/*!
    \since 5.14

    Returns the peer's chain of digital certificates.

    Chain starts with the peer's immediate certificate and ending with the CA's certificate.
*/
QList<QSslCertificate> QWebEngineCertificateError::certificateChain() const
{
    return d->certificateChain;
}

QT_END_NAMESPACE
