// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#include "base/functional/callback.h"
#include "content/public/browser/certificate_request_result_type.h"

#include "qwebenginecertificateerror.h"
#include <QtCore/QDateTime>
#include <QtCore/QScopedPointer>
#include <QtCore/QUrl>
#include <QtNetwork/QSslCertificate>

namespace net {
class SSLInfo;
}
class GURL;

namespace QtWebEngineCore {

class Q_WEBENGINECORE_PRIVATE_EXPORT CertificateErrorController {
public:
    CertificateErrorController(
            int cert_error, const net::SSLInfo &ssl_info, const GURL &request_url,
            bool strict_enforcement,
            base::OnceCallback<void(content::CertificateRequestResultType)> callback);
    ~CertificateErrorController();

    QWebEngineCertificateError::Type error() const;
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

    QWebEngineCertificateError::Type m_certError;
    const QUrl m_requestUrl;
    QDateTime m_validExpiry;
    bool m_overridable;
    base::OnceCallback<void(content::CertificateRequestResultType)> m_callback;
    QList<QSslCertificate> m_certificateChain;

    bool m_answered = false, m_deferred = false;

private:
    Q_DISABLE_COPY(CertificateErrorController)
};

}
#endif // CERTIFICATE_ERROR_CONTROLLER_H
