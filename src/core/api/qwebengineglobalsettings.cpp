// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineglobalsettings.h"
#include "qwebengineglobalsettings_p.h"

#ifdef signals
#undef signals
#endif

#include "content/public/browser/network_service_instance.h"
#include "services/network/network_service.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineGlobalSettings
    \brief The QWebEngineGlobalSettings class configures global properties of the web engine.
    \since 6.6
    \inmodule QtWebEngineCore

    The QWebEngineGlobalSettings class is a singleton that configures global properties
    of the web engine.

    Invoke configureDnsOverHttps() to configure DNS-over-HTTPS capabilities.

    \sa QWebEngineGlobalSettings::configureDnsOverHttps()
*/

QWebEngineGlobalSettings::QWebEngineGlobalSettings(QObject *p)
    : QObject(p), d_ptr(new QWebEngineGlobalSettingsPrivate)
{
}

QWebEngineGlobalSettings::~QWebEngineGlobalSettings() { }

/*!
    \fn QWebEngineGlobalSettings *QWebEngineGlobalSettings::GetInstance()

    Gets the global instance of QWebEngineGlobalSettings.
*/
QWebEngineGlobalSettings *QWebEngineGlobalSettings::GetInstance()
{
    static QWebEngineGlobalSettings settings;
    return &settings;
}

/*!
    \enum QWebEngineGlobalSettings::DnsMode

    This enum sets the DNS-over-HTTPS mode:

    \value WithFallback Enable DNS-over-HTTPS with fallbacks. If a host
    can't be resolved, try the insecure DNS client of Chromium. If that fails as
    well, try the system DNS host resolution, which can be secure or insecure.
    \value Secure Enable DNS-over-HTTPS and only allow the secure Chromium
    DNS client to resolve hosts.
*/

/*!
    \fn QWebEngineGlobalSettings::configureDnsOverHttps(const DnsMode dnsMode,
                                                        const QString &dnsOverHttpsTemplates)

    Configures the Chromium stub host resolver, thus allowing DNS-over-HTTPS functionality.

    Set \a dnsMode to QWebEngineGlobalSettings::DnsMode::WithFallback to enable secure DNS
    host resolution with a fallback to insecure DNS host resolution and a final fallback to
    the system DNS resolution, which can be secure or insecure. Set it to
    QWebEngineGlobalSettings::DnsMode::Secure to only allow secure DNS host resolution via
    the Chromium DNS client.

    Independently of \a {dnsMode}, \a dnsOverHttpsTemplates has to be set to one or multiple
    valid \l{https://datatracker.ietf.org/doc/html/rfc6570}{URI templates} separated by
    whitespace characters. One example URI template is https://dns.google/dns-query{?dns}.
*/
void QWebEngineGlobalSettings::configureDnsOverHttps(const DnsMode dnsMode,
                                                     const QString &dnsOverHttpsTemplates)
{
    Q_D(QWebEngineGlobalSettings);

    d->dnsMode = dnsMode;
    d->dnsOverHttpsTemplates = dnsOverHttpsTemplates.toStdString();
    d->isDnsOverHttpsUserConfigured = true;

    // Make sure that DoH settings are in effect immediately if the network service already exists,
    // thus allowing to change DoH configuration at any point
    network::mojom::NetworkService *networkService = content::GetNetworkService();
    if (networkService) {
        networkService->ConfigureStubHostResolver(
                d->insecureDnsClientEnabled, net::SecureDnsMode(d->dnsMode),
                *net::DnsOverHttpsConfig::FromString(d->dnsOverHttpsTemplates),
                d->additionalInsecureDnsTypesEnabled);
    }
}

QT_END_NAMESPACE
