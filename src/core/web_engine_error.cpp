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

WebEngineError::ErrorDomain WebEngineError::toQtErrorDomain(int error_code)
{
    switch (error_code) {
        case net::ERR_FAILED:
        case net::ERR_ABORTED:
        case net::ERR_FILE_NOT_FOUND:
        case net::ERR_TIMED_OUT:
        case net::ERR_FILE_TOO_BIG:
        case net::ERR_ACCESS_DENIED:
        case net::ERR_OUT_OF_MEMORY:
        case net::ERR_SOCKET_NOT_CONNECTED:
        case net::ERR_FILE_EXISTS:
        case net::ERR_FILE_PATH_TOO_LONG:
        case net::ERR_CONNECTION_CLOSED:
        case net::ERR_CONNECTION_RESET:
        case net::ERR_CONNECTION_REFUSED:
        case net::ERR_CONNECTION_ABORTED:
        case net::ERR_CONNECTION_FAILED:
            return WebEngineError::NetworkErrorDomain;
        case net::ERR_INVALID_URL:
        case net::ERR_DISALLOWED_URL_SCHEME:
        case net::ERR_UNKNOWN_URL_SCHEME:
        case net::ERR_TOO_MANY_REDIRECTS:
        case net::ERR_UNSAFE_REDIRECT:
        case net::ERR_INVALID_RESPONSE:
        case net::ERR_EMPTY_RESPONSE:
        case net::ERR_RESPONSE_HEADERS_TOO_BIG:
        case net::ERR_CACHE_MISS:
        case net::ERR_CACHE_READ_FAILURE:
        case net::ERR_CACHE_WRITE_FAILURE:
        case net::ERR_CACHE_OPERATION_NOT_SUPPORTED:
        case net::ERR_CACHE_OPEN_FAILURE:
        case net::ERR_CACHE_CREATE_FAILURE:
        case net::ERR_CACHE_RACE:
        case net::ERR_CACHE_CHECKSUM_READ_FAILURE:
        case net::ERR_CACHE_CHECKSUM_MISMATCH:
        case net::ERR_INSECURE_RESPONSE:
        case net::ERR_NO_PRIVATE_KEY_FOR_CERT:
        case net::ERR_ADD_USER_CERT_FAILED:
            return WebEngineError::HttpErrorDomain;
        default:
            return WebEngineError::InternalErrorDomain;
    }
}
