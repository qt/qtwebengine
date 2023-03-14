// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEGLOBALSETTINGS_P_H
#define QWEBENGINEGLOBALSETTINGS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtwebenginecoreglobal_p.h"
#include "qwebengineglobalsettings.h"
#include <string>

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineGlobalSettingsPrivate
{
public:
    QWebEngineGlobalSettingsPrivate()
        : dnsMode(QWebEngineGlobalSettings::DnsMode::WithFallback)
        , dnsOverHttpsTemplates("")
        , insecureDnsClientEnabled(true)
        , additionalInsecureDnsTypesEnabled(true)
        , isDnsOverHttpsUserConfigured(false){};

    QWebEngineGlobalSettings::DnsMode dnsMode;
    std::string dnsOverHttpsTemplates;
    bool insecureDnsClientEnabled;
    bool additionalInsecureDnsTypesEnabled;
    bool isDnsOverHttpsUserConfigured;
};

QT_END_NAMESPACE

#endif // QWEBENGINEGLOBALSETTINGS_P_H
