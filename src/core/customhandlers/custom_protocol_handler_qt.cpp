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

#include "custom_protocol_handler_qt.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/escape.h"

CustomProtocolHandlerQt::CustomProtocolHandlerQt(const std::string &protocol, const GURL &url,
        const string16 &title)
        : m_protocol(protocol), m_url(url), m_title(title)
{
}

CustomProtocolHandlerQt CustomProtocolHandlerQt::CreateProtocolHandler(const std::string &protocol,
        const GURL &url, const string16 &title)
{
    std::string lowerProtocol = StringToLowerASCII(protocol);
    return CustomProtocolHandlerQt(lowerProtocol, url, title);
}

CustomProtocolHandlerQt::CustomProtocolHandlerQt()
{
}

bool CustomProtocolHandlerQt::IsValidDict(const base::DictionaryValue *value)
{
    return value->HasKey("protocol") && value->HasKey("url") && value->HasKey("title");
}

bool CustomProtocolHandlerQt::IsSameOrigin(const CustomProtocolHandlerQt &handler) const
{
    return handler.url().GetOrigin() == m_url.GetOrigin();
}

const CustomProtocolHandlerQt &CustomProtocolHandlerQt::EmptyProtocolHandler()
{
    static const CustomProtocolHandlerQt * const kEmpty = new CustomProtocolHandlerQt();
    return *kEmpty;
}

CustomProtocolHandlerQt CustomProtocolHandlerQt::CreateProtocolHandler(
        const base::DictionaryValue *value)
{
    if (!IsValidDict(value)) {
        return EmptyProtocolHandler();
    }
    std::string protocol, url;
    string16 title;
    value->GetString("protocol", &protocol);
    value->GetString("url", &url);
    value->GetString("title", &title);
    return CustomProtocolHandlerQt::CreateProtocolHandler(protocol, GURL(url), title);
}

GURL CustomProtocolHandlerQt::TranslateUrl(const GURL &url) const
{
    std::string translatedUrlSpec(m_url.spec());
    ReplaceSubstringsAfterOffset(&translatedUrlSpec, 0, "%s",
            net::EscapeQueryParamValue(url.spec(), true));
    return GURL(translatedUrlSpec);
}

base::DictionaryValue *CustomProtocolHandlerQt::Encode() const
{
    base::DictionaryValue *d = new base::DictionaryValue();
    d->Set("protocol", new base::StringValue(m_protocol));
    d->Set("url", new base::StringValue(m_url.spec()));
    d->Set("title", new base::StringValue(m_title));
    return d;
}

#if !defined(NDEBUG)
std::string CustomProtocolHandlerQt::ToString() const
{
    return "{ protocol=" + m_protocol
            + ", url=" + m_url.spec()
            + ", title=" + UTF16ToASCII(m_title)
            + " }";
}
#endif

bool CustomProtocolHandlerQt::operator==(const CustomProtocolHandlerQt &other) const
{
    return (m_protocol == other.m_protocol) && (m_url == other.m_url) && (m_title == other.m_title);
}

bool CustomProtocolHandlerQt::IsEquivalent(const CustomProtocolHandlerQt &other) const
{
    return (m_protocol == other.m_protocol) && (m_url == other.m_url);
}

bool CustomProtocolHandlerQt::operator<(const CustomProtocolHandlerQt &other) const
{
    return m_title < other.m_title;
}
