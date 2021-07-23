/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
