// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "certificate_error_controller.h"

#include <net/base/net_errors.h>
#include <net/cert/x509_certificate.h>
#include <net/ssl/ssl_info.h>
#include <ui/base/l10n/l10n_util.h>
#include "chrome/grit/generated_resources.h"
#include "components/strings/grit/components_strings.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(QWebEngineCertificateError::SslPinnedKeyNotInCertificateChain,
                   net::ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateCommonNameInvalid, net::ERR_CERT_BEGIN)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateCommonNameInvalid,
                   net::ERR_CERT_COMMON_NAME_INVALID)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateDateInvalid, net::ERR_CERT_DATE_INVALID)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateAuthorityInvalid,
                   net::ERR_CERT_AUTHORITY_INVALID)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateContainsErrors,
                   net::ERR_CERT_CONTAINS_ERRORS)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateUnableToCheckRevocation,
                   net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateRevoked, net::ERR_CERT_REVOKED)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateInvalid, net::ERR_CERT_INVALID)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateWeakSignatureAlgorithm,
                   net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateNonUniqueName,
                   net::ERR_CERT_NON_UNIQUE_NAME)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateWeakKey, net::ERR_CERT_WEAK_KEY)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateNameConstraintViolation,
                   net::ERR_CERT_NAME_CONSTRAINT_VIOLATION)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateValidityTooLong,
                   net::ERR_CERT_VALIDITY_TOO_LONG)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateTransparencyRequired,
                   net::ERR_CERTIFICATE_TRANSPARENCY_REQUIRED)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateSymantecLegacy,
                   net::ERR_CERT_SYMANTEC_LEGACY)
ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateKnownInterceptionBlocked,
                   net::ERR_CERT_KNOWN_INTERCEPTION_BLOCKED)
// net::ERR_SSL_OBSOLETE_VERSION was removed again in Chromium 98
//ASSERT_ENUMS_MATCH(QWebEngineCertificateError::SslObsoleteVersion,
//                   net::ERR_SSL_OBSOLETE_VERSION)
//ASSERT_ENUMS_MATCH(QWebEngineCertificateError::CertificateErrorEnd, net::ERR_CERT_END)

// Copied from chrome/browser/ssl/ssl_error_handler.cc:
static int IsCertErrorFatal(int cert_error)
{
    switch (cert_error) {
    case net::ERR_CERT_COMMON_NAME_INVALID:
    case net::ERR_CERT_DATE_INVALID:
    case net::ERR_CERT_AUTHORITY_INVALID:
    case net::ERR_CERT_NO_REVOCATION_MECHANISM:
    case net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
    case net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
    case net::ERR_CERT_WEAK_KEY:
    case net::ERR_CERT_NAME_CONSTRAINT_VIOLATION:
    case net::ERR_CERT_VALIDITY_TOO_LONG:
    case net::ERR_CERTIFICATE_TRANSPARENCY_REQUIRED:
    case net::ERR_CERT_SYMANTEC_LEGACY:
    case net::ERR_CERT_KNOWN_INTERCEPTION_BLOCKED:
        return false;
    case net::ERR_CERT_CONTAINS_ERRORS:
    case net::ERR_CERT_REVOKED:
    case net::ERR_CERT_INVALID:
    case net::ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN:
        return true;
    default:
        NOTREACHED();
    }
    return true;
}

CertificateErrorController::CertificateErrorController(
        int cert_error, const net::SSLInfo &ssl_info, const GURL &request_url,
        bool strict_enforcement, base::OnceCallback<void(content::CertificateRequestResultType)> cb)
    : m_certError(QWebEngineCertificateError::Type(cert_error))
    , m_requestUrl(toQt(request_url))
    , m_overridable(!IsCertErrorFatal(cert_error) && !strict_enforcement)
{
    // MEMO set callback anyway even for non overridable error since chromium halts load until it's called
    //      callback will be executed either explicitly by use code or implicitly when error goes out of scope
    m_callback = std::move(cb);

    if (auto cert = ssl_info.cert.get()) {
        m_validExpiry = toQt(cert->valid_expiry());
        m_certificateChain = toCertificateChain(cert);
    }
}

CertificateErrorController::~CertificateErrorController()
{
    if (!answered())
        rejectCertificate();
}

QWebEngineCertificateError::Type CertificateErrorController::error() const
{
    return m_certError;
}

QUrl CertificateErrorController::url() const
{
    return m_requestUrl;
}

bool CertificateErrorController::overridable() const
{
    return m_overridable;
}

bool CertificateErrorController::deferred() const
{
    return m_deferred;
}

void CertificateErrorController::defer()
{
    m_deferred = true;
}

bool CertificateErrorController::answered() const
{
    return m_answered;
}

void CertificateErrorController::accept(bool accepted)
{
    if (answered())
        return;

    m_answered = true;
    if (m_callback)
        std::move(m_callback)
                .Run(accepted ? content::CERTIFICATE_REQUEST_RESULT_TYPE_CONTINUE
                              : content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY);
}

void CertificateErrorController::deactivate()
{
    m_callback.Reset();
}

static QString getQStringForMessageId(int message_id) {
    std::u16string string = l10n_util::GetStringUTF16(message_id);
    return toQt(string);
}

QString CertificateErrorController::errorString() const
{
    // Try to use chromiums translation of the error strings, though not all are
    // consistently described and we need to use versions that does not contain HTML
    // formatted text.
    switch (m_certError) {
    case QWebEngineCertificateError::SslPinnedKeyNotInCertificateChain:
        return getQStringForMessageId(IDS_CERT_ERROR_SUMMARY_PINNING_FAILURE_DETAILS);
    case QWebEngineCertificateError::CertificateCommonNameInvalid:
        return getQStringForMessageId(IDS_CERT_ERROR_COMMON_NAME_INVALID_DESCRIPTION);
    case QWebEngineCertificateError::CertificateDateInvalid:
        if (QDateTime::currentDateTime() > m_validExpiry)
            return getQStringForMessageId(IDS_CERT_ERROR_EXPIRED_DESCRIPTION);
        else
            return getQStringForMessageId(IDS_CERT_ERROR_NOT_YET_VALID_DESCRIPTION);
    case QWebEngineCertificateError::CertificateAuthorityInvalid:
    case QWebEngineCertificateError::CertificateKnownInterceptionBlocked:
    case QWebEngineCertificateError::CertificateSymantecLegacy:
        return getQStringForMessageId(IDS_CERT_ERROR_AUTHORITY_INVALID_DESCRIPTION);
    case QWebEngineCertificateError::CertificateContainsErrors:
        return getQStringForMessageId(IDS_CERT_ERROR_CONTAINS_ERRORS_DESCRIPTION);
    case QWebEngineCertificateError::CertificateNoRevocationMechanism:
        return getQStringForMessageId(IDS_CERT_ERROR_NO_REVOCATION_MECHANISM_DESCRIPTION);
    case QWebEngineCertificateError::CertificateRevoked:
        return getQStringForMessageId(IDS_CERT_ERROR_REVOKED_CERT_DESCRIPTION);
    case QWebEngineCertificateError::CertificateInvalid:
        return getQStringForMessageId(IDS_CERT_ERROR_INVALID_CERT_DESCRIPTION);
    case QWebEngineCertificateError::CertificateWeakSignatureAlgorithm:
        return getQStringForMessageId(IDS_CERT_ERROR_WEAK_SIGNATURE_ALGORITHM_DESCRIPTION);
    case QWebEngineCertificateError::CertificateNonUniqueName:
        return getQStringForMessageId(IDS_PAGE_INFO_SECURITY_TAB_NON_UNIQUE_NAME);
    case QWebEngineCertificateError::CertificateWeakKey:
        return getQStringForMessageId(IDS_CERT_ERROR_WEAK_KEY_DESCRIPTION);
    case QWebEngineCertificateError::CertificateNameConstraintViolation:
        return getQStringForMessageId(IDS_CERT_ERROR_NAME_CONSTRAINT_VIOLATION_DESCRIPTION);
    case QWebEngineCertificateError::CertificateValidityTooLong:
        return getQStringForMessageId(IDS_CERT_ERROR_VALIDITY_TOO_LONG_DESCRIPTION);
    case QWebEngineCertificateError::CertificateTransparencyRequired:
        return getQStringForMessageId(IDS_CERT_ERROR_CERTIFICATE_TRANSPARENCY_REQUIRED_DESCRIPTION);
    case QWebEngineCertificateError::SslObsoleteVersion:
        return getQStringForMessageId(IDS_SSL_ERROR_OBSOLETE_VERSION_DESCRIPTION);
    case QWebEngineCertificateError::CertificateUnableToCheckRevocation: // Deprecated in Chromium.
    default:
        break;
    }

    return getQStringForMessageId(IDS_CERT_ERROR_UNKNOWN_ERROR_DESCRIPTION);
}

QList<QSslCertificate> CertificateErrorController::certificateChain() const
{
    return m_certificateChain;
}

}
