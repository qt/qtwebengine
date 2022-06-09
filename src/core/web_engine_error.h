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

#ifndef WEB_ENGINE_ERROR_H
#define WEB_ENGINE_ERROR_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

class Q_WEBENGINECORE_PRIVATE_EXPORT WebEngineError
{

public:
    enum ErrorDomain {
         NoErrorDomain,
         InternalErrorDomain,
         ConnectionErrorDomain,
         CertificateErrorDomain,
         HttpErrorDomain,
         FtpErrorDomain,
         DnsErrorDomain,
         HttpStatusCodeDomain
    };

    static const int UserAbortedError;

    static ErrorDomain toQtErrorDomain(int error_code);
    static QString toQtErrorDescription(int errorCode);
};

#endif // WEB_ENGINE_ERROR_H
