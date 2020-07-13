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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef CERTIFICATE_ERROR_CONTROLLER_H
#define CERTIFICATE_ERROR_CONTROLLER_H

#include "qtwebenginecoreglobal_p.h"
#include "base/callback.h"
#include "content/public/browser/certificate_request_result_type.h"
#include <QtCore/QDateTime>
#include <QtCore/QScopedPointer>
#include <QtCore/QUrl>
#include <QtNetwork/QSslCertificate>

namespace net {
class SSLInfo;
}
class GURL;

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_PRIVATE_EXPORT CertificateErrorController {
public:
    CertificateErrorController(
            int cert_error, const net::SSLInfo &ssl_info, const GURL &request_url,
            bool strict_enforcement,
            base::OnceCallback<void(content::CertificateRequestResultType)> callback);
    ~CertificateErrorController();

    // We can't use QSslError::SslErrors, because the error categories doesn't map.
    // Keep up to date with net/base/net_errors.h and net::IsCertificateError():
    enum CertificateError {
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
        CertificateErrorEnd = -218 // not an error, just an enum boundary
    };

    CertificateError error() const;
    QUrl url() const;
    bool overridable() const;
    QString errorString() const;
    QDateTime validExpiry() const;
    QList<QSslCertificate> certificateChain() const;

    bool deferred() const;
    void defer();

    bool answered() const;
    void accept(bool);

    void ignoreCertificateError() { accept(true); }
    void rejectCertificate() { accept(false); }

    void deactivate();

    CertificateErrorController::CertificateError m_certError;
    const QUrl m_requestUrl;
    QDateTime m_validExpiry;
    bool m_overridable;
    base::OnceCallback<void(content::CertificateRequestResultType)> m_callback;
    QList<QSslCertificate> m_certificateChain;

    bool m_answered = false, m_deferred = false;

private:
    Q_DISABLE_COPY(CertificateErrorController)
};

QT_END_NAMESPACE

#endif // CERTIFICATE_ERROR_CONTROLLER_H
