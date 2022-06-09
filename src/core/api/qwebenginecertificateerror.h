// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECERTIFICATEERROR_H
#define QWEBENGINECERTIFICATEERROR_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qsslcertificate.h>

namespace QtWebEngineCore {
class WebContentsDelegateQt;
class CertificateErrorController;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineCertificateError
{
    Q_GADGET
    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)
    Q_PROPERTY(Type type READ type CONSTANT FINAL)
    Q_PROPERTY(QString description READ description CONSTANT FINAL)
    Q_PROPERTY(bool overridable READ isOverridable CONSTANT FINAL)

public:
    QWebEngineCertificateError(const QWebEngineCertificateError &other);
    QWebEngineCertificateError &operator=(const QWebEngineCertificateError &other);
    ~QWebEngineCertificateError();

    // Keep this identical to NET_ERROR in net_error_list.h, or add mapping layer.
    enum Type {
        SslPinnedKeyNotInCertificateChain = -150,
        CertificateCommonNameInvalid = -200,
        CertificateDateInvalid = -201,
        CertificateAuthorityInvalid = -202,
        CertificateContainsErrors = -203,
        CertificateNoRevocationMechanism = -204,
        CertificateUnableToCheckRevocation = -205,
        CertificateRevoked = -206,
        CertificateInvalid = -207,
        CertificateWeakSignatureAlgorithm = -208,
        CertificateNonUniqueName = -210,
        CertificateWeakKey = -211,
        CertificateNameConstraintViolation = -212,
        CertificateValidityTooLong = -213,
        CertificateTransparencyRequired = -214,
        CertificateSymantecLegacy = -215,
        CertificateKnownInterceptionBlocked = -217,
        SslObsoleteVersion = -218,
    };
    Q_ENUM(Type)

    Type type() const;
    QUrl url() const;
    bool isOverridable() const;
    QString description() const;

    Q_INVOKABLE void defer();
    Q_INVOKABLE void rejectCertificate();
    Q_INVOKABLE void acceptCertificate();

    QList<QSslCertificate> certificateChain() const;

private:
    friend class QtWebEngineCore::WebContentsDelegateQt;
    QWebEngineCertificateError(
            const QSharedPointer<QtWebEngineCore::CertificateErrorController> &controller);
    QSharedPointer<QtWebEngineCore::CertificateErrorController> d;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QWebEngineCertificateError)

#endif // QWEBENGINECERTIFICATEERROR_H
