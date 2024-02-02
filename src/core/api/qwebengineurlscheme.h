// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEURLSCHEME_H
#define QWEBENGINEURLSCHEME_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qshareddata.h>

namespace QtWebEngineCore {
class WebEngineContext;
}

QT_BEGIN_NAMESPACE

class QWebEngineUrlSchemePrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineUrlScheme
{
    Q_GADGET
public:
    enum class Syntax {
        HostPortAndUserInformation,
        HostAndPort,
        Host,
        Path,
    };

    enum SpecialPort {
        PortUnspecified = -1
    };

    enum Flag {
        SecureScheme = 0x1,
        LocalScheme = 0x2,
        LocalAccessAllowed = 0x4,
        NoAccessAllowed = 0x8,
        ServiceWorkersAllowed = 0x10,
        ViewSourceAllowed = 0x20,
        ContentSecurityPolicyIgnored = 0x40,
        CorsEnabled = 0x80,
        FetchApiAllowed = 0x100,
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAG(Flags)

    QWebEngineUrlScheme();
    explicit QWebEngineUrlScheme(const QByteArray &name);

    QWebEngineUrlScheme(const QWebEngineUrlScheme &that);
    QWebEngineUrlScheme &operator=(const QWebEngineUrlScheme &that);

    QWebEngineUrlScheme(QWebEngineUrlScheme &&that);
    QWebEngineUrlScheme &operator=(QWebEngineUrlScheme &&that);

    ~QWebEngineUrlScheme();

    bool operator==(const QWebEngineUrlScheme &that) const;
    bool operator!=(const QWebEngineUrlScheme &that) const { return !(*this == that); }

    QByteArray name() const;
    void setName(const QByteArray &newValue);

    Syntax syntax() const;
    void setSyntax(Syntax newValue);

    int defaultPort() const;
    void setDefaultPort(int newValue);

    Flags flags() const;
    void setFlags(Flags newValue);

    static void registerScheme(const QWebEngineUrlScheme &scheme);
    static QWebEngineUrlScheme schemeByName(const QByteArray &name);

private:
    friend QtWebEngineCore::WebEngineContext;
    static void lockSchemes();
    QWebEngineUrlScheme(QWebEngineUrlSchemePrivate *d);
    QSharedDataPointer<QWebEngineUrlSchemePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWebEngineUrlScheme::Flags)

QT_END_NAMESPACE

#endif // !QWEBENGINEURLSCHEME_H
