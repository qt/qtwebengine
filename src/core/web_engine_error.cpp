// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "web_engine_error.h"

#include "components/error_page/common/error.h"
#include "components/error_page/common/localized_error.h"
#include "net/base/net_errors.h"

#include "type_conversion.h"

#include <QString>

using namespace QtWebEngineCore;

const int WebEngineError::UserAbortedError = net::ERR_ABORTED;

namespace {
const int noError = 0;
const int connectionRelatedErrors = -100;
const int certificateErrors = -200;
const int httpErrors = -300;
const int cacheErrors = -400;
const int ftpErrors = -600;
const int certificateManagerErrors = -700;
const int dnsResolverErrors = -800;
const int endErrors = -900;
}

WebEngineError::ErrorDomain WebEngineError::toQtErrorDomain(int error_code)
{
    // net errors are always negative values, and https response codes are positive
    if (error_code > 0)
        return HttpStatusCodeDomain;

    // Chromium's ranges from net/base/net_error_list.h:
    //         0 No error
    //     1- 99 System related errors
    //   100-199 Connection related errors
    //   200-299 Certificate errors
    //   300-399 HTTP errors
    //   400-499 Cache errors
    //   500-599 Internal errors
    //   600-699 FTP errors
    //   700-799 Certificate manager errors
    //   800-899 DNS resolver errors

    if (error_code == noError)
        return WebEngineError::NoErrorDomain;
    else if (certificateErrors < error_code && error_code <= connectionRelatedErrors)
        return WebEngineError::ConnectionErrorDomain;
    else if ((httpErrors < error_code && error_code <= certificateErrors)
             || (dnsResolverErrors < error_code && error_code <= certificateManagerErrors))
        return WebEngineError::CertificateErrorDomain;
    else if (cacheErrors < error_code && error_code <= httpErrors)
        return WebEngineError::HttpErrorDomain;
    else if (certificateManagerErrors < error_code && error_code <= ftpErrors)
        return WebEngineError::FtpErrorDomain;
    else if (endErrors < error_code && error_code <= dnsResolverErrors)
        return WebEngineError::DnsErrorDomain;
    else
        return WebEngineError::InternalErrorDomain;
}

QString WebEngineError::toQtErrorDescription(int errorCode)
{
    if (errorCode < 0)
        return toQt(net::ErrorToString(errorCode));
    else if (errorCode > 0)
        return toQt(error_page::LocalizedError::GetErrorDetails(
            error_page::Error::kHttpErrorDomain, errorCode, false, false));
    return QString();
}
