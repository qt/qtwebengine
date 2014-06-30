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

#include "base/i18n/rtl.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "chrome/common/net/net_error_info.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "ui/base/l10n/l10n_util.h"
#include "third_party/WebKit/public/platform/WebURLError.h"

#include "grit/generated_resources.h"
#include "grit/chromium_strings.h"

#include "type_conversion.h"
#include <QCoreApplication>


// Ripped From chrome/common/localized_error.cc
namespace {

// Some error pages have no details.
const unsigned int kErrorPagesNoDetails = 0;

static const char kRedirectLoopLearnMoreUrl[] =
    "about:blank";
static const char kWeakDHKeyLearnMoreUrl[] =
    "http://sites.google.com/a/chromium.org/dev/"
    "err_ssl_weak_server_ephemeral_dh_key";

const char kDnsProbeErrorDomain[] = "dnsprobe";

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

const LocalizedErrorMap dns_probe_error_options[] = {
  {chrome_common_net::DNS_PROBE_POSSIBLE,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_DNS_PROBE_RUNNING,
   IDS_ERRORPAGES_DETAILS_DNS_PROBE_RUNNING,
   SUGGEST_RELOAD,
  },

  // DNS_PROBE_NOT_RUN is not here; NetErrorHelper will restore the original
  // error, which might be one of several DNS-related errors.

  {chrome_common_net::DNS_PROBE_STARTED,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_DNS_PROBE_RUNNING,
   IDS_ERRORPAGES_DETAILS_DNS_PROBE_RUNNING,
   // Include SUGGEST_RELOAD so the More button doesn't jump when we update.
   SUGGEST_RELOAD,
  },

  // DNS_PROBE_FINISHED_UNKNOWN is not here; NetErrorHelper will restore the
  // original error, which might be one of several DNS-related errors.

  {chrome_common_net::DNS_PROBE_FINISHED_NO_INTERNET,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_INTERNET_DISCONNECTED,
   IDS_ERRORPAGES_SUMMARY_INTERNET_DISCONNECTED,
   IDS_ERRORPAGES_DETAILS_INTERNET_DISCONNECTED,
   SUGGEST_RELOAD | SUGGEST_CHECK_CONNECTION | SUGGEST_FIREWALL_CONFIG,
  },
  {chrome_common_net::DNS_PROBE_FINISHED_BAD_CONFIG,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_NAME_NOT_RESOLVED,
   IDS_ERRORPAGES_DETAILS_NAME_NOT_RESOLVED,
   SUGGEST_RELOAD | SUGGEST_DNS_CONFIG | SUGGEST_FIREWALL_CONFIG,
  },
  {chrome_common_net::DNS_PROBE_FINISHED_NXDOMAIN,
   IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
   IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
   IDS_ERRORPAGES_SUMMARY_NAME_NOT_RESOLVED,
   IDS_ERRORPAGES_DETAILS_NAME_NOT_RESOLVED,
   SUGGEST_RELOAD,
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

const LocalizedErrorMap* LookupErrorMap(const std::string& error_domain,
                                        int error_code, bool is_post) {
  if (error_domain == net::kErrorDomain) {
    // Display a different page in the special case of navigating through the
    // history to an uncached page created by a POST.
    if (is_post && error_code == net::ERR_CACHE_MISS)
      return &repost_error;
    return FindErrorMapInArray(net_error_options,
                               arraysize(net_error_options),
                               error_code);
  } else if (error_domain == LocalizedError::kHttpErrorDomain) {
    return FindErrorMapInArray(http_error_options,
                               arraysize(http_error_options),
                               error_code);
  } else if (error_domain == kDnsProbeErrorDomain) {
    const LocalizedErrorMap* map =
        FindErrorMapInArray(dns_probe_error_options,
                            arraysize(dns_probe_error_options),
                            error_code);
    DCHECK(map);
    return map;
  } else {
    NOTREACHED();
    return NULL;
  }
}

// Returns a dictionary containing the strings for the settings menu under the
// wrench, and the advanced settings button.
base::DictionaryValue* GetStandardMenuItemsText() {
  base::DictionaryValue* standard_menu_items_text = new base::DictionaryValue();
  standard_menu_items_text->SetString("settingsTitle",
      l10n_util::GetStringUTF16(IDS_SETTINGS_TITLE));
  standard_menu_items_text->SetString("advancedTitle",
      l10n_util::GetStringUTF16(IDS_SETTINGS_SHOW_ADVANCED_SETTINGS));
  return standard_menu_items_text;
}

// Gets the icon class for a given |error_domain| and |error_code|.
const char* GetIconClassForError(const std::string& error_domain,
                                 int error_code) {
  if ((error_code == net::ERR_INTERNET_DISCONNECTED &&
       error_domain == net::kErrorDomain) ||
      (error_code == chrome_common_net::DNS_PROBE_FINISHED_NO_INTERNET &&
       error_domain == kDnsProbeErrorDomain))
    return "icon-offline";

  return "icon-generic";
}
}// namespace

const char LocalizedError::kHttpErrorDomain[] = "http";

void LocalizedError::GetStrings(int error_code,
                                const std::string& error_domain,
                                const GURL& failed_url,
                                bool is_post,
                                const std::string& locale,
                                const std::string& accept_languages,
                                base::DictionaryValue* error_strings) {
  bool rtl = base::i18n::IsRTL();
  error_strings->SetString("textdirection", rtl ? "rtl" : "ltr");

  // Grab the strings and settings that depend on the error type.  Init
  // options with default values.
  LocalizedErrorMap options = {
    0,
    IDS_ERRORPAGES_TITLE_NOT_AVAILABLE,
    IDS_ERRORPAGES_HEADING_NOT_AVAILABLE,
    IDS_ERRORPAGES_SUMMARY_NOT_AVAILABLE,
    kErrorPagesNoDetails,
    SUGGEST_NONE,
  };

  const LocalizedErrorMap* error_map = LookupErrorMap(error_domain, error_code,
                                                      is_post);
  if (error_map)
    options = *error_map;

  // If we got "access denied" but the url was a file URL, then we say it was a
  // file instead of just using the "not available" default message. Just adding
  // ERR_ACCESS_DENIED to the map isn't sufficient, since that message may be
  // generated by some OSs when the operation doesn't involve a file URL.
  if (error_domain == net::kErrorDomain &&
      error_code == net::ERR_ACCESS_DENIED &&
      failed_url.scheme() == "file") {
    options.title_resource_id = IDS_ERRORPAGES_TITLE_ACCESS_DENIED;
    options.heading_resource_id = IDS_ERRORPAGES_HEADING_FILE_ACCESS_DENIED;
    options.summary_resource_id = IDS_ERRORPAGES_SUMMARY_FILE_ACCESS_DENIED;
    options.details_resource_id = IDS_ERRORPAGES_DETAILS_FILE_ACCESS_DENIED;
    options.suggestions = SUGGEST_NONE;
  }

  base::string16 failed_url_string(net::FormatUrl(
      failed_url, accept_languages, net::kFormatUrlOmitNothing,
      net::UnescapeRule::NORMAL, NULL, NULL, NULL));
  // URLs are always LTR.
  if (rtl)
    base::i18n::WrapStringWithLTRFormatting(&failed_url_string);
  error_strings->SetString("title",
      l10n_util::GetStringFUTF16(options.title_resource_id, failed_url_string));
  error_strings->SetString("heading",
      l10n_util::GetStringUTF16(options.heading_resource_id));

  std::string icon_class = GetIconClassForError(error_domain, error_code);
  error_strings->SetString("iconClass", icon_class);

  base::DictionaryValue* summary = new base::DictionaryValue;
  summary->SetString("msg",
      l10n_util::GetStringUTF16(options.summary_resource_id));
  summary->SetString("failedUrl", failed_url_string);
  summary->SetString("hostName", net::IDNToUnicode(failed_url.host(),
                                                   accept_languages));
  summary->SetString("productName", toString16(qApp->applicationName()));

  error_strings->SetString(
      "more", l10n_util::GetStringUTF16(IDS_ERRORPAGES_BUTTON_MORE));
  error_strings->SetString(
      "less", l10n_util::GetStringUTF16(IDS_ERRORPAGES_BUTTON_LESS));
  error_strings->Set("summary", summary);

  if (options.details_resource_id != kErrorPagesNoDetails) {
    error_strings->SetString(
        "errorDetails", l10n_util::GetStringUTF16(options.details_resource_id));
  }

  base::string16 error_string;
  if (error_domain == net::kErrorDomain) {
    // Non-internationalized error string, for debugging Chrome itself.
    std::string ascii_error_string = net::ErrorToString(error_code);
    // Remove the leading "net::" from the returned string.
    base::RemoveChars(ascii_error_string, "net:", &ascii_error_string);
    error_string = ASCIIToUTF16(ascii_error_string);
  } else if (error_domain == kDnsProbeErrorDomain) {
    std::string ascii_error_string =
        chrome_common_net::DnsProbeStatusToString(error_code);
    error_string = ASCIIToUTF16(ascii_error_string);
  } else {
    DCHECK_EQ(LocalizedError::kHttpErrorDomain, error_domain);
    error_string = base::IntToString16(error_code);
  }
  error_strings->SetString("errorCode",
      l10n_util::GetStringFUTF16(IDS_ERRORPAGES_ERROR_CODE, error_string));

  base::ListValue* suggestions = new base::ListValue();

  // Platform specific instructions for diagnosing network issues on OSX and
  // Windows.
#if defined(OS_MACOSX) || defined(OS_WIN)
  if (error_domain == net::kErrorDomain &&
      error_code == net::ERR_INTERNET_DISCONNECTED) {
    int platform_string_id =
        IDS_ERRORPAGES_SUMMARY_INTERNET_DISCONNECTED_PLATFORM;
#if defined(OS_WIN)
    // Different versions of Windows have different instructions.
    base::win::Version windows_version = base::win::GetVersion();
    if (windows_version < base::win::VERSION_VISTA) {
      // XP, XP64, and Server 2003.
      platform_string_id =
          IDS_ERRORPAGES_SUMMARY_INTERNET_DISCONNECTED_PLATFORM_XP;
    } else if (windows_version == base::win::VERSION_VISTA) {
      // Vista
      platform_string_id =
          IDS_ERRORPAGES_SUMMARY_INTERNET_DISCONNECTED_PLATFORM_VISTA;
    }
#endif  // defined(OS_WIN)
    // Lead with the general error description, and suffix with the platform
    // dependent portion of the summary section.
    summary->SetString("msg",
        l10n_util::GetStringFUTF16(
            IDS_ERRORPAGES_SUMMARY_INTERNET_DISCONNECTED_INSTRUCTIONS_TEMPLATE,
            l10n_util::GetStringUTF16(options.summary_resource_id),
            l10n_util::GetStringUTF16(platform_string_id)));
  }
#endif  // defined(OS_MACOSX) || defined(OS_WIN)

  if (options.suggestions & SUGGEST_RELOAD) {
    if (!is_post) {
      base::DictionaryValue* reload_button = new base::DictionaryValue;
      reload_button->SetString("msg",
          l10n_util::GetStringUTF16(IDS_ERRORPAGES_BUTTON_RELOAD));
      reload_button->SetString("reloadUrl", failed_url.spec());
      error_strings->Set("reload", reload_button);
    } else {
      // If the page was created by a post, it can't be reloaded in the same
      // way, so just add a suggestion instead.
      // TODO(mmenke):  Make the reload button bring up the repost confirmation
      //                dialog for pages resulting from posts.
      base::DictionaryValue* suggest_reload_repost = new base::DictionaryValue;
      suggest_reload_repost->SetString("header",
          l10n_util::GetStringUTF16(
              IDS_ERRORPAGES_SUGGESTION_RELOAD_REPOST_HEADER));
      suggest_reload_repost->SetString("body",
          l10n_util::GetStringUTF16(
              IDS_ERRORPAGES_SUGGESTION_RELOAD_REPOST_BODY));
      suggestions->Append(suggest_reload_repost);
    }
  }

  if (options.suggestions & SUGGEST_CHECK_CONNECTION) {
    base::DictionaryValue* suggest_check_connection = new base::DictionaryValue;
    suggest_check_connection->SetString("header",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_CHECK_CONNECTION_HEADER));
    suggest_check_connection->SetString("body",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_CHECK_CONNECTION_BODY));
    suggestions->Append(suggest_check_connection);
  }

  if (options.suggestions & SUGGEST_DNS_CONFIG) {
    base::DictionaryValue* suggest_dns_config = new base::DictionaryValue;
    suggest_dns_config->SetString("header",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_DNS_CONFIG_HEADER));
    suggest_dns_config->SetString("body",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_DNS_CONFIG_BODY));
    suggestions->Append(suggest_dns_config);

    base::DictionaryValue* suggest_network_prediction =
        GetStandardMenuItemsText();
    suggest_network_prediction->SetString("header",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_NETWORK_PREDICTION_HEADER));
    suggest_network_prediction->SetString("body",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_NETWORK_PREDICTION_BODY));
    suggest_network_prediction->SetString(
        "noNetworkPredictionTitle",
        l10n_util::GetStringUTF16(
            IDS_NETWORK_PREDICTION_ENABLED_DESCRIPTION));
    suggestions->Append(suggest_network_prediction);
  }

  if (options.suggestions & SUGGEST_FIREWALL_CONFIG) {
    base::DictionaryValue* suggest_firewall_config = new base::DictionaryValue;
    suggest_firewall_config->SetString("header",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_FIREWALL_CONFIG_HEADER));
    suggest_firewall_config->SetString("body",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_FIREWALL_CONFIG_BODY));
    suggestions->Append(suggest_firewall_config);
  }

  if (options.suggestions & SUGGEST_PROXY_CONFIG) {
    base::DictionaryValue* suggest_proxy_config = GetStandardMenuItemsText();
    suggest_proxy_config->SetString("header",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_PROXY_CONFIG_HEADER));
    suggest_proxy_config->SetString("body",
        l10n_util::GetStringFUTF16(IDS_ERRORPAGES_SUGGESTION_PROXY_CONFIG_BODY,
            l10n_util::GetStringUTF16(
                IDS_ERRORPAGES_SUGGESTION_PROXY_DISABLE_PLATFORM)));
    suggest_proxy_config->SetString("proxyTitle",
        l10n_util::GetStringUTF16(IDS_OPTIONS_PROXIES_CONFIGURE_BUTTON));

    suggestions->Append(suggest_proxy_config);
  }

  if (options.suggestions & SUGGEST_DISABLE_EXTENSION) {
    base::DictionaryValue* suggest_disable_extension =
        new base::DictionaryValue;
    // There's only a header for this suggestion.
    suggest_disable_extension->SetString("header",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_DISABLE_EXTENSION_HEADER));
    suggestions->Append(suggest_disable_extension);
  }

  if (options.suggestions & SUGGEST_VIEW_POLICIES) {
    base::DictionaryValue* suggest_view_policies = new base::DictionaryValue;
    suggest_view_policies->SetString(
        "header",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_VIEW_POLICIES_HEADER));
    suggest_view_policies->SetString(
        "body",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_VIEW_POLICIES_BODY));
    suggestions->Append(suggest_view_policies);
  }

  if (options.suggestions & SUGGEST_CONTACT_ADMINISTRATOR) {
    base::DictionaryValue* suggest_contant_administrator =
        new base::DictionaryValue;
    suggest_contant_administrator->SetString(
        "body",
        l10n_util::GetStringUTF16(
            IDS_ERRORPAGES_SUGGESTION_CONTACT_ADMINISTRATOR_BODY));
    suggestions->Append(suggest_contant_administrator);
  }

  if (options.suggestions & SUGGEST_LEARNMORE) {
    GURL learn_more_url;
    switch (options.error_code) {
      case net::ERR_TOO_MANY_REDIRECTS:
        learn_more_url = GURL(kRedirectLoopLearnMoreUrl);
        break;
      case net::ERR_SSL_WEAK_SERVER_EPHEMERAL_DH_KEY:
        learn_more_url = GURL(kWeakDHKeyLearnMoreUrl);
        break;
      default:
        break;
    }

    if (learn_more_url.is_valid()) {
      // Add the language parameter to the URL.
      std::string query = learn_more_url.query() + "&hl=" + locale;
      GURL::Replacements repl;
      repl.SetQueryStr(query);
      learn_more_url = learn_more_url.ReplaceComponents(repl);

      base::DictionaryValue* suggest_learn_more = new base::DictionaryValue;
      // There's only a body for this suggestion.
      suggest_learn_more->SetString("body",
          l10n_util::GetStringUTF16(IDS_ERRORPAGES_SUGGESTION_LEARNMORE_BODY));
      suggest_learn_more->SetString("learnMoreUrl", learn_more_url.spec());
      suggestions->Append(suggest_learn_more);
    }
  }

  error_strings->Set("suggestions", suggestions);
}

base::string16 LocalizedError::GetErrorDetails(const blink::WebURLError& error,
                                               bool is_post) {
  const LocalizedErrorMap* error_map =
      LookupErrorMap(error.domain.utf8(), error.reason, is_post);
  if (error_map)
    return l10n_util::GetStringUTF16(error_map->details_resource_id);
  else
    return l10n_util::GetStringUTF16(IDS_ERRORPAGES_DETAILS_UNKNOWN);
}
