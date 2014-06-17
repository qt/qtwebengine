/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "localized_error.h"

#include "net/base/net_errors.h"
#include "ui/base/l10n/l10n_util.h"

#include "grit/generated_resources.h"

#include "type_conversion.h"

#include <QTextDocument>

// Ripped From chrome/common/localized_error.cc
namespace {

enum NAV_SUGGESTIONS {
  SUGGEST_NONE                  = 0,
  SUGGEST_RELOAD                = 1 << 0,
  SUGGEST_CHECK_CONNECTION      = 1 << 1,
  SUGGEST_DNS_CONFIG            = 1 << 2,
  SUGGEST_FIREWALL_CONFIG       = 1 << 3,
  SUGGEST_PROXY_CONFIG          = 1 << 4,
  SUGGEST_DISABLE_EXTENSION     = 1 << 5,
  SUGGEST_LEARNMORE             = 1 << 6,
  SUGGEST_VIEW_POLICIES         = 1 << 7,
  SUGGEST_CONTACT_ADMINISTRATOR = 1 << 8,
};


struct LocalizedErrorMap {
  int error_code;
  unsigned int title_resource_id; // contains placeholders.
  unsigned int heading_resource_id;
  // Detailed summary used when the error is in the main frame.
  unsigned int summary_resource_id;
  // Short one sentence description shown on mouse over when the error is in
  // a frame.
  unsigned int details_resource_id;
  int suggestions;  // Bitmap of SUGGEST_* values.
};

const LocalizedErrorMap net_error_options[] = {
  {net::ERR_TIMED_OUT,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_TIMED_OUT,
   IDS_ERRORPAGES_DETAILS_TIMED_OUT,
   SUGGEST_RELOAD | SUGGEST_CHECK_CONNECTION | SUGGEST_FIREWALL_CONFIG |
       SUGGEST_PROXY_CONFIG,
  },
  {net::ERR_CONNECTION_TIMED_OUT,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_TIMED_OUT,
   IDS_ERRORPAGES_DETAILS_TIMED_OUT,
   SUGGEST_RELOAD | SUGGEST_CHECK_CONNECTION | SUGGEST_FIREWALL_CONFIG |
       SUGGEST_PROXY_CONFIG,
  },
  {net::ERR_CONNECTION_CLOSED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_NOT_AVAILABLE,
   IDS_ERRORPAGES_DETAILS_CONNECTION_CLOSED,
   SUGGEST_RELOAD,
  },
  {net::ERR_CONNECTION_RESET,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_CONNECTION_RESET,
   IDS_ERRORPAGES_DETAILS_CONNECTION_RESET,
   SUGGEST_RELOAD | SUGGEST_CHECK_CONNECTION | SUGGEST_FIREWALL_CONFIG |
       SUGGEST_PROXY_CONFIG,
  },
  {net::ERR_CONNECTION_REFUSED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_CONNECTION_REFUSED,
   IDS_ERRORPAGES_DETAILS_CONNECTION_REFUSED,
   SUGGEST_RELOAD | SUGGEST_CHECK_CONNECTION | SUGGEST_FIREWALL_CONFIG |
       SUGGEST_PROXY_CONFIG,
  },
  {net::ERR_CONNECTION_FAILED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_NOT_AVAILABLE,
   IDS_ERRORPAGES_DETAILS_CONNECTION_FAILED,
   SUGGEST_RELOAD,
  },
  {net::ERR_NAME_NOT_RESOLVED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_NAME_NOT_RESOLVED,
   IDS_ERRORPAGES_DETAILS_NAME_NOT_RESOLVED,
   SUGGEST_RELOAD | SUGGEST_CHECK_CONNECTION | SUGGEST_DNS_CONFIG |
       SUGGEST_FIREWALL_CONFIG | SUGGEST_PROXY_CONFIG,
  },
  {net::ERR_ADDRESS_UNREACHABLE,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_ADDRESS_UNREACHABLE,
   IDS_ERRORPAGES_DETAILS_ADDRESS_UNREACHABLE,
   SUGGEST_RELOAD | SUGGEST_FIREWALL_CONFIG | SUGGEST_PROXY_CONFIG,
  },
  {net::ERR_NETWORK_ACCESS_DENIED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NETWORK_ACCESS_DENIED,
   IDS_ERRORPAGES_SUMMARY_NETWORK_ACCESS_DENIED,
   IDS_ERRORPAGES_DETAILS_NETWORK_ACCESS_DENIED,
   SUGGEST_FIREWALL_CONFIG,
  },
  {net::ERR_PROXY_CONNECTION_FAILED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_PROXY_CONNECTION_FAILED,
   IDS_ERRORPAGES_SUMMARY_PROXY_CONNECTION_FAILED,
   IDS_ERRORPAGES_DETAILS_PROXY_CONNECTION_FAILED,
   SUGGEST_PROXY_CONFIG,
  },
  {net::ERR_INTERNET_DISCONNECTED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_INTERNET_DISCONNECTED,
   IDS_ERRORPAGES_SUMMARY_INTERNET_DISCONNECTED,
   IDS_ERRORPAGES_DETAILS_INTERNET_DISCONNECTED,
   SUGGEST_NONE,
  },
  {net::ERR_FILE_NOT_FOUND,
   IDS_ERRORPAGES_TITLE_NOT_FOUND,
   IDS_ERRORPAGES_HEADING_NOT_FOUND,
   IDS_ERRORPAGES_SUMMARY_NOT_FOUND,
   IDS_ERRORPAGES_DETAILS_FILE_NOT_FOUND,
   SUGGEST_NONE,
  },
  {net::ERR_CACHE_MISS,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_CACHE_MISS,
   IDS_ERRORPAGES_SUMMARY_CACHE_MISS,
   IDS_ERRORPAGES_DETAILS_CACHE_MISS,
   SUGGEST_RELOAD,
  },
  {net::ERR_CACHE_READ_FAILURE,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_CACHE_READ_FAILURE,
   IDS_ERRORPAGES_SUMMARY_CACHE_READ_FAILURE,
   IDS_ERRORPAGES_DETAILS_CACHE_READ_FAILURE,
   SUGGEST_RELOAD,
  },
  {net::ERR_NETWORK_IO_SUSPENDED,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_NETWORK_IO_SUSPENDED,
   IDS_ERRORPAGES_SUMMARY_NETWORK_IO_SUSPENDED,
   IDS_ERRORPAGES_DETAILS_NETWORK_IO_SUSPENDED,
   SUGGEST_RELOAD,
  },
  {net::ERR_TOO_MANY_REDIRECTS,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_TOO_MANY_REDIRECTS,
   IDS_ERRORPAGES_SUMMARY_TOO_MANY_REDIRECTS,
   IDS_ERRORPAGES_DETAILS_TOO_MANY_REDIRECTS,
   SUGGEST_RELOAD | SUGGEST_LEARNMORE,
  },
  {net::ERR_EMPTY_RESPONSE,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_EMPTY_RESPONSE,
   IDS_ERRORPAGES_SUMMARY_EMPTY_RESPONSE,
   IDS_ERRORPAGES_DETAILS_EMPTY_RESPONSE,
   SUGGEST_RELOAD,
  },
  {net::ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_DUPLICATE_HEADERS,
   IDS_ERRORPAGES_SUMMARY_DUPLICATE_HEADERS,
   IDS_ERRORPAGES_DETAILS_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH,
   SUGGEST_NONE,
  },
  {net::ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_DISPOSITION,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_DUPLICATE_HEADERS,
   IDS_ERRORPAGES_SUMMARY_DUPLICATE_HEADERS,
   IDS_ERRORPAGES_DETAILS_RESPONSE_HEADERS_MULTIPLE_CONTENT_DISPOSITION,
   SUGGEST_NONE,
  },
  {net::ERR_RESPONSE_HEADERS_MULTIPLE_LOCATION,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_DUPLICATE_HEADERS,
   IDS_ERRORPAGES_SUMMARY_DUPLICATE_HEADERS,
   IDS_ERRORPAGES_DETAILS_RESPONSE_HEADERS_MULTIPLE_LOCATION,
   SUGGEST_NONE,
  },
  {net::ERR_CONTENT_LENGTH_MISMATCH,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_NOT_AVAILABLE,
   IDS_ERRORPAGES_DETAILS_CONNECTION_CLOSED,
   SUGGEST_RELOAD,
  },
  {net::ERR_INCOMPLETE_CHUNKED_ENCODING,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_NOT_AVAILABLE,
   IDS_ERRORPAGES_DETAILS_CONNECTION_CLOSED,
   SUGGEST_RELOAD,
  },
  {net::ERR_SSL_PROTOCOL_ERROR,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_SSL_PROTOCOL_ERROR,
   IDS_ERRORPAGES_SUMMARY_SSL_PROTOCOL_ERROR,
   IDS_ERRORPAGES_DETAILS_SSL_PROTOCOL_ERROR,
   SUGGEST_NONE,
  },
  {net::ERR_SSL_UNSAFE_NEGOTIATION,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_SSL_PROTOCOL_ERROR,
   IDS_ERRORPAGES_SUMMARY_SSL_PROTOCOL_ERROR,
   IDS_ERRORPAGES_DETAILS_SSL_UNSAFE_NEGOTIATION,
   SUGGEST_NONE,
  },
  {net::ERR_BAD_SSL_CLIENT_AUTH_CERT,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_BAD_SSL_CLIENT_AUTH_CERT,
   IDS_ERRORPAGES_SUMMARY_BAD_SSL_CLIENT_AUTH_CERT,
   IDS_ERRORPAGES_DETAILS_BAD_SSL_CLIENT_AUTH_CERT,
   SUGGEST_NONE,
  },
  {net::ERR_SSL_WEAK_SERVER_EPHEMERAL_DH_KEY,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_WEAK_SERVER_EPHEMERAL_DH_KEY,
   IDS_ERRORPAGES_SUMMARY_WEAK_SERVER_EPHEMERAL_DH_KEY,
   IDS_ERRORPAGES_DETAILS_SSL_PROTOCOL_ERROR,
   SUGGEST_LEARNMORE,
  },
  {net::ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_PINNING_FAILURE,
   IDS_ERRORPAGES_SUMMARY_PINNING_FAILURE,
   IDS_ERRORPAGES_DETAILS_PINNING_FAILURE,
   SUGGEST_NONE,
  },
  {net::ERR_TEMPORARILY_THROTTLED,
   IDS_ERRORPAGES_TITLE_ACCESS_DENIED,
   IDS_ERRORPAGES_HEADING_ACCESS_DENIED,
   IDS_ERRORPAGES_SUMMARY_TEMPORARILY_THROTTLED,
   IDS_ERRORPAGES_DETAILS_TEMPORARILY_THROTTLED,
   SUGGEST_NONE,
  },
  {net::ERR_BLOCKED_BY_CLIENT,
   IDS_ERRORPAGES_TITLE_BLOCKED,
   IDS_ERRORPAGES_HEADING_BLOCKED,
   IDS_ERRORPAGES_SUMMARY_BLOCKED,
   IDS_ERRORPAGES_DETAILS_BLOCKED,
   SUGGEST_RELOAD | SUGGEST_DISABLE_EXTENSION,
  },
  {net::ERR_NETWORK_CHANGED,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_NETWORK_ACCESS_DENIED,
   IDS_ERRORPAGES_SUMMARY_NETWORK_CHANGED,
   IDS_ERRORPAGES_DETAILS_NETWORK_CHANGED,
   SUGGEST_RELOAD | SUGGEST_CHECK_CONNECTION,
  },
  {net::ERR_BLOCKED_BY_ADMINISTRATOR,
   IDS_ERRORPAGES_TITLE_BLOCKED,
   IDS_ERRORPAGES_HEADING_BLOCKED_BY_ADMINISTRATOR,
   IDS_ERRORPAGES_SUMMARY_BLOCKED_BY_ADMINISTRATOR,
   IDS_ERRORPAGES_DETAILS_BLOCKED_BY_ADMINISTRATOR,
   SUGGEST_VIEW_POLICIES | SUGGEST_CONTACT_ADMINISTRATOR,
  },
};

// Special error page to be used in the case of navigating back to a page
// generated by a POST.  LocalizedError::HasStrings expects this net error code
// to also appear in the array above.
const LocalizedErrorMap repost_error = {
  net::ERR_CACHE_MISS,
  IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
  IDS_HTTP_POST_WARNING_TITLE,
  IDS_ERRORPAGES_HTTP_POST_WARNING,
  IDS_ERRORPAGES_DETAILS_CACHE_MISS,
  SUGGEST_RELOAD,
};

const LocalizedErrorMap http_error_options[] = {
  {403,
   IDS_ERRORPAGES_TITLE_ACCESS_DENIED,
   IDS_ERRORPAGES_HEADING_ACCESS_DENIED,
   IDS_ERRORPAGES_SUMMARY_FORBIDDEN,
   IDS_ERRORPAGES_DETAILS_FORBIDDEN,
   SUGGEST_NONE,
  },
  {410,
   IDS_ERRORPAGES_TITLE_NOT_FOUND,
   IDS_ERRORPAGES_HEADING_NOT_FOUND,
   IDS_ERRORPAGES_SUMMARY_GONE,
   IDS_ERRORPAGES_DETAILS_GONE,
   SUGGEST_NONE,
  },

  {500,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_HTTP_SERVER_ERROR,
   IDS_ERRORPAGES_SUMMARY_INTERNAL_SERVER_ERROR,
   IDS_ERRORPAGES_DETAILS_INTERNAL_SERVER_ERROR,
   SUGGEST_RELOAD,
  },
  {501,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_HTTP_SERVER_ERROR,
   IDS_ERRORPAGES_SUMMARY_WEBSITE_CANNOT_HANDLE,
   IDS_ERRORPAGES_DETAILS_NOT_IMPLEMENTED,
   SUGGEST_NONE,
  },
  {502,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_HTTP_SERVER_ERROR,
   IDS_ERRORPAGES_SUMMARY_BAD_GATEWAY,
   IDS_ERRORPAGES_DETAILS_BAD_GATEWAY,
   SUGGEST_RELOAD,
  },
  {503,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_HTTP_SERVER_ERROR,
   IDS_ERRORPAGES_SUMMARY_SERVICE_UNAVAILABLE,
   IDS_ERRORPAGES_DETAILS_SERVICE_UNAVAILABLE,
   SUGGEST_RELOAD,
  },
  {504,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_HTTP_SERVER_ERROR,
   IDS_ERRORPAGES_SUMMARY_GATEWAY_TIMEOUT,
   IDS_ERRORPAGES_DETAILS_GATEWAY_TIMEOUT,
   SUGGEST_RELOAD,
  },
  {505,
   IDS_ERRORPAGES_TITLE_LOAD_FAILED,
   IDS_ERRORPAGES_HEADING_HTTP_SERVER_ERROR,
   IDS_ERRORPAGES_SUMMARY_WEBSITE_CANNOT_HANDLE,
   IDS_ERRORPAGES_DETAILS_HTTP_VERSION_NOT_SUPPORTED,
   SUGGEST_NONE,
  },
};

const LocalizedErrorMap* FindErrorMapInArray(const LocalizedErrorMap* maps,
                                                   size_t num_maps,
                                                   int error_code) {
  for (size_t i = 0; i < num_maps; ++i) {
    if (maps[i].error_code == error_code)
      return &maps[i];
  }
  return NULL;
}

const LocalizedErrorMap* LookupErrorMap(bool isHttpErrorDomain,
                                        int error_code, bool is_post) {
  if (!isHttpErrorDomain) {
    // Display a different page in the special case of navigating through the
    // history to an uncached page created by a POST.
    if (is_post && error_code == net::ERR_CACHE_MISS)
      return &repost_error;
    return FindErrorMapInArray(net_error_options,
                               arraysize(net_error_options),
                               error_code);
  } else {
    return FindErrorMapInArray(http_error_options,
                               arraysize(http_error_options),
                               error_code);
  }
}
}// namespace

LocalizedError::LocalizedError(int errorCode, bool isHttpErrorDomain, bool isPost)
    : errorCode(errorCode)
    , isHttpErrorDomain(isHttpErrorDomain)
    , isPost(isPost)
{
}

QString LocalizedError::heading() const
{
    const LocalizedErrorMap* errorMap = LookupErrorMap(isHttpErrorDomain, errorCode, isPost);
    if (errorMap)
        return toQt(l10n_util::GetStringUTF16(errorMap->heading_resource_id));
    // Text is "Unknown error."
    return toQt(l10n_util::GetStringUTF16(IDS_ERRORPAGES_DETAILS_UNKNOWN));
}

QString LocalizedError::summary() const
{
    const LocalizedErrorMap* errorMap = LookupErrorMap(isHttpErrorDomain, errorCode, isPost);
    if (errorMap) {
        QTextDocument doc;
        doc.setHtml(toQt(l10n_util::GetStringUTF16(errorMap->summary_resource_id)));
        return doc.toPlainText();
    }
    return QString();
}

QString LocalizedError::internalName() const
{
    return QString::fromLatin1(net::ErrorToString(errorCode));
}

std::string LocalizedError::details() const
{
    const LocalizedErrorMap* errorMap = LookupErrorMap(isHttpErrorDomain, errorCode, isPost);
    if (errorMap)
        return l10n_util::GetStringUTF8(errorMap->details_resource_id);
    return l10n_util::GetStringUTF8(IDS_ERRORPAGES_DETAILS_UNKNOWN);
}
