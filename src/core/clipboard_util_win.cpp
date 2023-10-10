// Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <string>

#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"

// These functions are copied from ui/base/clipboard/clipboard_util_win.cc

namespace QtWebEngineCore {

// Helper method for converting from text/html to MS CF_HTML.
// Documentation for the CF_HTML format is available at
// http://msdn.microsoft.com/en-us/library/aa767917(VS.85).aspx
// HtmlToCFHtml is based on similar method in
// WebCore/platform/win/ClipboardUtilitiesWin.cpp.
std::string HtmlToCFHtml(const std::string &html, const std::string &base_url)
{
    if (html.empty())
        return std::string();

#define MAX_DIGITS 10
#define MAKE_NUMBER_FORMAT_1(digits) MAKE_NUMBER_FORMAT_2(digits)
#define MAKE_NUMBER_FORMAT_2(digits) "%0" #digits "u"
#define NUMBER_FORMAT MAKE_NUMBER_FORMAT_1(MAX_DIGITS)

    static const char *header = "Version:0.9\r\n"
                                "StartHTML:" NUMBER_FORMAT "\r\n"
                                "EndHTML:" NUMBER_FORMAT "\r\n"
                                "StartFragment:" NUMBER_FORMAT "\r\n"
                                "EndFragment:" NUMBER_FORMAT "\r\n";
    static const char *source_url_prefix = "SourceURL:";

    static const char *start_markup = "<html>\r\n<body>\r\n<!--StartFragment-->";
    static const char *end_markup = "<!--EndFragment-->\r\n</body>\r\n</html>";

    // Calculate offsets
    size_t start_html_offset = strlen(header) - strlen(NUMBER_FORMAT) * 4 + MAX_DIGITS * 4;
    if (!base_url.empty()) {
        start_html_offset += strlen(source_url_prefix) + base_url.length() + 2; // Add 2 for \r\n.
    }
    size_t start_fragment_offset = start_html_offset + strlen(start_markup);
    size_t end_fragment_offset = start_fragment_offset + html.length();
    size_t end_html_offset = end_fragment_offset + strlen(end_markup);

    std::string result = base::StringPrintf(header, start_html_offset, end_html_offset,
                                            start_fragment_offset, end_fragment_offset);
    if (!base_url.empty()) {
        result += source_url_prefix;
        result += base_url;
        result += "\r\n";
    }
    result += start_markup;
    result += html;
    result += end_markup;

#undef MAX_DIGITS
#undef MAKE_NUMBER_FORMAT_1
#undef MAKE_NUMBER_FORMAT_2
#undef NUMBER_FORMAT

    return result;
}

void CFHtmlExtractMetadata(const std::string &cf_html, std::string *base_url, size_t *html_start,
                           size_t *fragment_start, size_t *fragment_end)
{
    // Obtain base_url if present.
    if (base_url) {
        static constexpr char kSrcUrlStr[] = "SourceURL:";
        size_t line_start = cf_html.find(kSrcUrlStr);
        if (line_start != std::string::npos) {
            size_t src_end = cf_html.find("\n", line_start);
            size_t src_start = line_start + strlen(kSrcUrlStr);
            if (src_end != std::string::npos && src_start != std::string::npos) {
                *base_url = cf_html.substr(src_start, src_end - src_start);
                base::TrimWhitespaceASCII(*base_url, base::TRIM_ALL, base_url);
            }
        }
    }

    // Find the markup between "<!--StartFragment-->" and "<!--EndFragment-->".
    // If the comments cannot be found, like copying from OpenOffice Writer,
    // we simply fall back to using StartFragment/EndFragment bytecount values
    // to determine the fragment indexes.
    std::string cf_html_lower = base::ToLowerASCII(cf_html);
    size_t markup_start = cf_html_lower.find("<html", 0);
    if (html_start) {
        *html_start = markup_start;
    }
    size_t tag_start = cf_html.find("<!--StartFragment", markup_start);
    if (tag_start == std::string::npos) {
        static constexpr char kStartFragmentStr[] = "StartFragment:";
        size_t start_fragment_start = cf_html.find(kStartFragmentStr);
        if (start_fragment_start != std::string::npos) {
            *fragment_start = static_cast<size_t>(
                    atoi(cf_html.c_str() + start_fragment_start + strlen(kStartFragmentStr)));
        }

        static constexpr char kEndFragmentStr[] = "EndFragment:";
        size_t end_fragment_start = cf_html.find(kEndFragmentStr);
        if (end_fragment_start != std::string::npos) {
            *fragment_end = static_cast<size_t>(
                    atoi(cf_html.c_str() + end_fragment_start + strlen(kEndFragmentStr)));
        }
    } else {
        *fragment_start = cf_html.find('>', tag_start) + 1;
        size_t tag_end = cf_html.rfind("<!--EndFragment", std::string::npos);
        *fragment_end = cf_html.rfind('<', tag_end);
    }
}

} // namespace QtWebEngineCore
