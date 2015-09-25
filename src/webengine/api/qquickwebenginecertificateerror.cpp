/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qquickwebenginecertificateerror_p.h>
#include "certificate_error_controller.h"
QT_BEGIN_NAMESPACE

class QQuickWebEngineCertificateErrorPrivate {
public:
    QQuickWebEngineCertificateErrorPrivate(const QSharedPointer<CertificateErrorController> &controller)
        : weakRefCertErrorController(controller),
          error(static_cast<QQuickWebEngineCertificateError::Error>(static_cast<int>(controller->error()))),
          description(controller->errorString()),
          overridable(controller->overridable()),
          async(false),
          answered(false)
    {
    }

    const QWeakPointer<CertificateErrorController> weakRefCertErrorController;
    QQuickWebEngineCertificateError::Error error;
    QString description;
    bool overridable;
    bool async;
    bool answered;
};

/*!
    \qmltype WebEngineCertificateError
    \instantiates QQuickWebEngineCertificateError
    \inqmlmodule QtWebEngine 1.1
    \since QtWebEngine 1.1

    \brief A utility class for accepting or denying certificate exceptions when a certificate error occurs.

    This class contains information about a certificate error that happened and provides a way to accept or
    deny a certificate exception.

    \sa WebEngineView::certificateError
*/
QQuickWebEngineCertificateError::QQuickWebEngineCertificateError(const QSharedPointer<CertificateErrorController> &controller, QObject *parent)
    : QObject(parent)
    , d_ptr(new QQuickWebEngineCertificateErrorPrivate(controller))
{
}

QQuickWebEngineCertificateError::~QQuickWebEngineCertificateError()
{
    rejectCertificate();
}


/*!
  \qmlmethod void WebEngineCertificateError::defer()

  This function should be called when there is a need to postpone the decision to ignore or not the certificate error. This is useful to
  wait for user input. When called it will pause the url request until WebEngineCertificateError::ignoreCertificateError() or
  WebEngineCertificateError::rejectCertificate() is called.
 */
void QQuickWebEngineCertificateError::defer()
{
    Q_D(QQuickWebEngineCertificateError);
    d->async = true;
}
/*!
  \qmlmethod void WebEngineCertificateError::ignoreCertificateError()

  The certificate error is ignored and the WebEngineView continues to load the requested url.
 */
void QQuickWebEngineCertificateError::ignoreCertificateError()
{
    Q_D(QQuickWebEngineCertificateError);

    d->answered = true;

    QSharedPointer<CertificateErrorController> strongRefCert = d->weakRefCertErrorController.toStrongRef();
    if (strongRefCert)
        strongRefCert->accept(true);
}

/*!
  \qmlmethod void WebEngineCertificateError::rejectCertificate()

  The WebEngineView stops loading the requested url.
 */
void QQuickWebEngineCertificateError::rejectCertificate()
{
    Q_D(QQuickWebEngineCertificateError);

    d->answered = true;

    QSharedPointer<CertificateErrorController> strongRefCert = d->weakRefCertErrorController.toStrongRef();
    if (strongRefCert)
        strongRefCert->accept(false);
}

/*!
    \qmlproperty url WebEngineCertificateError::url
    \readonly

    The URL that triggered the error.
 */
QUrl QQuickWebEngineCertificateError::url() const
{
    Q_D(const QQuickWebEngineCertificateError);
    QSharedPointer<CertificateErrorController> strongRefCert = d->weakRefCertErrorController.toStrongRef();
    if (strongRefCert)
        return strongRefCert->url();
    return QUrl();
}

/*!
    \qmlproperty enumeration WebEngineCertificateError::error
    \readonly

    The type of the error.

    \value SslPinnedKeyNotInCertificateChain The certificate did not match the built-in public key pins for the host name.
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
*/
QQuickWebEngineCertificateError::Error QQuickWebEngineCertificateError::error() const
{
    Q_D(const QQuickWebEngineCertificateError);
    return d->error;
}

/*!
    \qmlproperty string WebEngineCertificateError::description
    \readonly

    A short localized human-readable description of the error.
*/
QString QQuickWebEngineCertificateError::description() const
{
    Q_D(const QQuickWebEngineCertificateError);
    return d->description;
}

/*!
    \qmlproperty bool WebEngineCertificateError::overridable
    \readonly

    A boolean that indicates if the certificate error can be overridden and accepted.
*/
bool QQuickWebEngineCertificateError::overridable() const
{
    Q_D(const QQuickWebEngineCertificateError);
    return d->overridable;
}

bool QQuickWebEngineCertificateError::deferred() const
{
    Q_D(const QQuickWebEngineCertificateError);
    return d->async;
}

bool QQuickWebEngineCertificateError::answered() const
{
    Q_D(const QQuickWebEngineCertificateError);
    return d->answered;
}

QT_END_NAMESPACE

