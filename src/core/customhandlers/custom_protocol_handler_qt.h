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

#ifndef CUSTOM_PROTOCOL_HANDLER_QT_H_
#define CUSTOM_PROTOCOL_HANDLER_QT_H_

#include "base/values.h"
#include "url/gurl.h"

#include <string>

// A single tuple of (protocol, url, title) that indicates how URLs of the
// given protocol should be rewritten to be handled.
class CustomProtocolHandlerQt {

public:
    static CustomProtocolHandlerQt CreateProtocolHandler(const std::string &protocol,
            const GURL &url, const string16 &title);

    // Creates a CustomProtocolHandlerQt with fields from the dictionary. Returns an
    // empty CustomProtocolHandlerQt if the input is invalid.
    static CustomProtocolHandlerQt CreateProtocolHandler(const base::DictionaryValue *value);

    // Returns true if the dictionary value has all the necessary fields to
    // define a CustomProtocolHandlerQt.
    static bool IsValidDict(const base::DictionaryValue *value);

    // Returns true if this handler's url has the same origin as the given one.
    bool IsSameOrigin(const CustomProtocolHandlerQt &handler) const;

    // Canonical empty CustomProtocolHandlerQt.
    static const CustomProtocolHandlerQt &EmptyProtocolHandler();

    // Interpolates the given URL into the URL template of this handler.
    GURL TranslateUrl(const GURL &url) const;

    // Returns true if the handlers are considered equivalent when determining
    // if both handlers can be registered, or if a handler has previously been
    // ignored.
    bool IsEquivalent(const CustomProtocolHandlerQt &other) const;

    // Encodes this protocol handler as a DictionaryValue. The caller is
    // responsible for deleting the returned value.
    base::DictionaryValue *Encode() const;

    const std::string &protocol() const
    {
        return m_protocol;
    }
    const GURL &url() const
    {
        return m_url;
    }
    const string16 &title() const
    {
        return m_title;
    }

    bool IsEmpty() const
    {
        return m_protocol.empty();
    }

#if !defined(NDEBUG)
    // Returns a string representation suitable for use in debugging.
    std::string ToString() const;
#endif

    bool operator==(const CustomProtocolHandlerQt &other) const;
    bool operator<(const CustomProtocolHandlerQt &other) const;

private:
    CustomProtocolHandlerQt(const std::string &protocol, const GURL &url, const string16 &title);
    CustomProtocolHandlerQt();

    std::string m_protocol;
    GURL m_url;
    string16 m_title;
};

#endif  // CUSTOM_PROTOCOL_HANDLER_QT_H_
