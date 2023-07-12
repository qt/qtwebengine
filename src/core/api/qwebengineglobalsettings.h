// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEGLOBALSETTINGS_H
#define QWEBENGINEGLOBALSETTINGS_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

namespace QtWebEngineCore {
class SystemNetworkContextManager;
}

QT_BEGIN_NAMESPACE

class QWebEngineGlobalSettingsPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineGlobalSettings : public QObject
{
    Q_OBJECT
public:
    static QWebEngineGlobalSettings *instance();

    // Mapping net::SecureDnsMode
    enum class DnsMode { WithFallback = 1, Secure = 2 };

    void configureDnsOverHttps(DnsMode dnsMode, const QString &dnsOverHttpsTemplates);

private:
    QWebEngineGlobalSettings(QObject *p = nullptr);
    ~QWebEngineGlobalSettings() override;

    friend class QtWebEngineCore::SystemNetworkContextManager;
    Q_DECLARE_PRIVATE(QWebEngineGlobalSettings)
    // can't re-use base d_ptr: need to maintain compat with last Qt LTS
    QScopedPointer<QWebEngineGlobalSettingsPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEGLOBALSETTINGS_H
