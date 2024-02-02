// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineglobalsettings.h"
#include "qwebengineglobalsettings_p.h"
#include <QDebug>

#ifdef signals
#undef signals
#endif

#include "content/browser/network_service_instance_impl.h"
#include "content/public/browser/network_service_instance.h"
#include "services/network/network_service.h"

QT_BEGIN_NAMESPACE

ASSERT_ENUMS_MATCH(net::SecureDnsMode::kSecure, QWebEngineGlobalSettings::SecureDnsMode::SecureOnly)
ASSERT_ENUMS_MATCH(net::SecureDnsMode::kAutomatic,
                   QWebEngineGlobalSettings::SecureDnsMode::SecureWithFallback)
ASSERT_ENUMS_MATCH(net::SecureDnsMode::kOff, QWebEngineGlobalSettings::SecureDnsMode::SystemOnly)

/*!
    \namespace QWebEngineGlobalSettings
    \brief The QWebEngineGlobalSettings namespace holds global settings of the web engine.
    \since 6.6
    \inmodule QtWebEngineCore

    The QWebEngineGlobalSettings namespace holds global properties of the web engine.

    Invoke setDnsMode() to configure DNS-over-HTTPS.

    \sa QWebEngineGlobalSettings::setDnsMode()
*/

/*!
    \enum QWebEngineGlobalSettings::SecureDnsMode

    This enum sets the DNS-over-HTTPS mode used by the DnsMode structure:

    \value SystemOnly This is the default. Use the system DNS host resolution.
    \value SecureWithFallback Enable DNS-over-HTTPS (DoH). DoH servers have to be
    provided through \l {QWebEngineGlobalSettings::DnsMode::serverTemplates}{serverTemplates} in
    the DnsMode structure. If a host cannot be resolved via the provided servers,
    the system DNS host resolution is used.
    \value SecureOnly Enable DNS-over-HTTPS and only allow hosts to be resolved
    this way. DoH servers have to be provided through
    \l {QWebEngineGlobalSettings::DnsMode::serverTemplates}{serverTemplates} in the DnsMode
    structure. If the DNS-over-HTTPS resolution fails, there is no fallback and the DNS host
    resolution fails completely.
*/

/*!
    \class QWebEngineGlobalSettings::DnsMode
    \brief The DnsMode struct provides means to specify the DNS host resolution mode.
    \since 6.6
    \inmodule QtWebEngineCore

    The QWebEngineGlobalSettings::DnsMode structure describes the DNS mode and
    the associated DNS server template used for the DNS host resolution.
*/

/*!
    \variable QWebEngineGlobalSettings::DnsMode::secureMode
    \brief The DNS mode used for the host resolution.

    Set \a secureMode to SecureDnsMode::SecureOnly to only allow DNS-over-HTTPS host resolution
    using servers from \a serverTemplates.

    Set \a secureMode to SecureDnsMode::SecureWithFallback to enable DNS-over-HTTPS host resolution
    using servers from \a serverTemplates, with a fallback to the system DNS.

    \sa QWebEngineGlobalSettings::SecureDnsMode
*/

/*!
    \variable QWebEngineGlobalSettings::DnsMode::serverTemplates
    \brief A list of server URI templates used for secure DNS-over-HTTPS host resolution.

    The \c serverTemplates structure member lists
    \l{https://datatracker.ietf.org/d7oc/html/rfc6570}{URI templates}.
    An example of a URI template is https://dns.google/dns-query{?dns}.
*/

/*!
    \fn void QWebEngineGlobalSettings::setDnsMode(DnsMode dnsMode)

    Sets \a dnsMode for DNS-over-HTTPS host resolution.

    This function returns \c false if the \l {QWebEngineGlobalSettings::DnsMode::serverTemplates}
    {serverTemplates} list in the \l {QWebEngineGlobalSettings::DnsMode}{DnsMode} structure is empty
    or contains URI templates that cannot be parsed for SecureDnsMode::SecureOnly or
    SecureDnsMode::SecureWithFallback. Otherwise, it returns \c true meaning that the DNS mode
    change is triggered.
*/

bool QWebEngineGlobalSettings::setDnsMode(DnsMode dnsMode)
{
    QWebEngineGlobalSettingsPrivate *d = QWebEngineGlobalSettingsPrivate::instance();
    if (dnsMode.secureMode != SecureDnsMode::SystemOnly) {
        const QString servers = dnsMode.serverTemplates.join(QChar::Space);
        const std::string templates = servers.toStdString();
        absl::optional<net::DnsOverHttpsConfig> dnsOverHttpsConfig =
                net::DnsOverHttpsConfig::FromString(templates);
        if (!dnsOverHttpsConfig.has_value())
            return false;
        d->dnsOverHttpsTemplates = templates;
    }
    d->dnsMode = dnsMode.secureMode;
    d->configureStubHostResolver();
    return true;
}

/*!
    \internal
*/
QWebEngineGlobalSettingsPrivate *QWebEngineGlobalSettingsPrivate::instance()
{
    static QWebEngineGlobalSettingsPrivate settings;
    return &settings;
}

void QWebEngineGlobalSettingsPrivate::configureStubHostResolver()
{
    if (content::GetNetworkServiceAvailability()
        != content::NetworkServiceAvailability::NOT_CREATED) {
        network::mojom::NetworkService *networkService = content::GetNetworkService();
        if (networkService) {
            qDebug() << "doh set to" << dnsOverHttpsTemplates << " -- "
                     << (dnsMode == QWebEngineGlobalSettings::SecureDnsMode::SecureOnly
                                 ? "SecureOnly"
                                 : dnsMode == QWebEngineGlobalSettings::SecureDnsMode::SystemOnly
                                 ? "SystemOnly"
                                 : "SecureWithFallback");
            absl::optional<net::DnsOverHttpsConfig> dohConfig = dnsOverHttpsTemplates.empty()
                    ? net::DnsOverHttpsConfig()
                    : net::DnsOverHttpsConfig::FromString(dnsOverHttpsTemplates);
            networkService->ConfigureStubHostResolver(insecureDnsClientEnabled,
                                                      net::SecureDnsMode(dnsMode), *dohConfig,
                                                      additionalInsecureDnsTypesEnabled);
        }
    }
}

QT_END_NAMESPACE
