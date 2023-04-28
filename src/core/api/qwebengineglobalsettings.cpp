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

ASSERT_ENUMS_MATCH(net::SecureDnsMode::kSecure, QWebEngineGlobalSettings::DnsMode::SecureOnly)
ASSERT_ENUMS_MATCH(net::SecureDnsMode::kAutomatic,
                   QWebEngineGlobalSettings::DnsMode::SecureWithFallback)
ASSERT_ENUMS_MATCH(net::SecureDnsMode::kOff, QWebEngineGlobalSettings::DnsMode::SystemOnly)

/*!
    \class QWebEngineGlobalSettings
    \brief The QWebEngineGlobalSettings class configures global properties of the web engine.
    \since 6.6
    \inmodule QtWebEngineCore

    The QWebEngineGlobalSettings class is a singleton that configures global properties
    of the web engine.

    Invoke setDnsMode() and setDnsServerTemplates() to configure DNS-over-HTTPS.

    \sa QWebEngineGlobalSettings::setDnsMode(), QWebEngineGlobalSettings::setDnsServerTemplates()
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
QWebEngineGlobalSettings *QWebEngineGlobalSettings::instance()
{
    static QWebEngineGlobalSettings settings;
    return &settings;
}

/*!
    \enum QWebEngineGlobalSettings::DnsMode

    This enum sets the DNS-over-HTTPS mode:

    \value SystemOnly This is the default. Use the system DNS host resolution.
    \value SecureWithFallback Enable DNS-over-HTTPS (DoH). DoH servers have to be
    provided through QWebEngineGlobalSettings::setDnsServerTemplates(). If a host can't be resolved
    via the provided servers, the system DNS host resolution is used.
    \value SecureOnly Enable DNS-over-HTTPS and only allow hosts to be resolved this way.
    DoH servers have to be provided through QWebEngineGlobalSettings::setDnsServerTemplates().
    If the DNS-over-HTTPS resolution fails, there is no fallback and DNS host resolution
    fails completely.
*/

/*!
    \fn void QWebEngineGlobalSettings::setDnsMode(DnsMode dnsMode, const QStringList
    &dnsServerTemplates)

    Set \a dnsMode to DnsMode::SystemOnly to use the system DNS resolution.

    Set \a dnsMode to DnsMode::SecureOnly to only allow DNS-over-HTTPS host resolution using servers
    from \a dnsServerTemplates.

    Set \a dnsMode to DnsMode::SecureWithFallback to enable DNS-over-HTTPS host resolution using
    servers from \a dnsServerTemplates,with a fallback to the system DNS.

    A list \a dnsServerTemplates is a list of \l{https://datatracker.ietf.org/d7oc/html/rfc6570}{URI
    templates}. One example URI template is https://dns.google/dns-query{?dns}.

    This function returns \c false if the \a dnsServerTemplates list is empty or contains URI
    templates that cannot be parsed for DnsMode::SecureOnly or DnsMode::SecureWithFallback.
    Otherwise, it returns \c true meaning the DNS mode change is triggered.
*/
bool QWebEngineGlobalSettings::setDnsMode(DnsMode dnsMode, const QStringList &dnsServerTemplates)
{
    Q_D(QWebEngineGlobalSettings);
    if (dnsMode != DnsMode::SystemOnly) {
        const QString servers = dnsServerTemplates.join(QChar::Space);
        const std::string templates = servers.toStdString();
        absl::optional<net::DnsOverHttpsConfig> dnsOverHttpsConfig =
                net::DnsOverHttpsConfig::FromString(templates);
        if (!dnsOverHttpsConfig.has_value())
            return false;
        d->dnsOverHttpsTemplates = templates;
    }
    d->dnsMode = dnsMode;
    d->configureStubHostResolver();
    return true;
}

/*!
    \internal
*/
void QWebEngineGlobalSettingsPrivate::configureStubHostResolver()
{
    if (content::GetNetworkServiceAvailability()
        != content::NetworkServiceAvailability::NOT_CREATED) {
        network::mojom::NetworkService *networkService = content::GetNetworkService();
        if (networkService) {
            qDebug() << "doh set to" << dnsOverHttpsTemplates << " -- "
                     << (dnsMode == QWebEngineGlobalSettings::DnsMode::SecureOnly ? "SecureOnly"
                                 : dnsMode == QWebEngineGlobalSettings::DnsMode::SystemOnly
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
