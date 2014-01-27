/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "web_engine_error.h"
#include "net/base/net_errors.h"

const int WebEngineError::UserAbortedError = net::ERR_ABORTED;

namespace {
const int noError = 0;
const int systemRelatedErrors = -1;
const int connectionRelatedErrors = -100;
const int certificateErrors = -200;
const int httpErrors = -300;
const int cacheErrors = -400;
const int internalErrors = -500;
const int ftpErrors = -600;
const int certificateManagerErrors = -700;
const int dnsResolverErrors = -800;
const int endErrors = -900;
}

WebEngineError::ErrorDomain WebEngineError::toQtErrorDomain(int error_code)
{
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
