// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEGLOBALSETTINGS_H
#define QWEBENGINEGLOBALSETTINGS_H

#if 0
#pragma qt_class(QWebEngineGlobalSettings)
#endif

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

namespace QWebEngineGlobalSettings {
// Mapping net::SecureDnsMode
enum class SecureDnsMode : quint8 { SystemOnly = 0, SecureWithFallback = 1, SecureOnly = 2 };
struct DnsMode
{
    SecureDnsMode secureMode = SecureDnsMode::SystemOnly;
    QStringList serverTemplates;
};
Q_WEBENGINECORE_EXPORT bool setDnsMode(DnsMode dnsMode);
}

QT_END_NAMESPACE

#endif // QWEBENGINEGLOBALSETTINGS_H
