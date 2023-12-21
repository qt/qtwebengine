// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineclienthints.h"

#include "profile_adapter.h"

#include <QJsonObject>

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineClientHints
    \brief The QWebEngineClientHints class provides an object to customize User-Agent Client Hints used by a profile.

    \since 6.8

    \inmodule QtWebEngineCore

    QWebEngineClientHints allows configuration of exposing browser and platform information via
    User-Agent response and request headers, and a JavaScript API.

    The information accessed via this API is split into two groups: low entropy and high entropy hints.
    Low entropy hints (\l{QWebEngineClientHints::platform}{platform} and \l{QWebEngineClientHints::mobile}{mobile})
    are those that do not give away much information; the API makes these accessible with every request and they can not
    be disabled by QWebEngineClientHints::setAllClientHintsEnabled.

    All the others are high entropy hints; they have the potential to give away more information, therefore they can be
    disabled by QWebEngineClientHints::setAllClientHintsEnabled.

    Each profile object has its own QWebEngineClientHints object, which configures the
    Client Hint settings for that browsing context. If a Client Hint is not configured for a web engine
    profile, its default value is deduced from the system.

    \sa QWebEngineProfile::clientHints(), QQuickWebEngineProfile::clientHints()
*/

QWebEngineClientHints::QWebEngineClientHints(QtWebEngineCore::ProfileAdapter *profileAdapter)
    : m_profileAdapter(profileAdapter)
{
}

QWebEngineClientHints::~QWebEngineClientHints()
{
}

/*!
    \property QWebEngineClientHints::arch
    The value of the \c{Sec-CH-UA-Arch} HTTP header and \c{architecture} member of NavigatorUAData in JavaScript.
*/
QString QWebEngineClientHints::arch() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAArchitecture).toString();
}

/*!
    \property QWebEngineClientHints::platform
    The value of the \c{Sec-CH-UA-Platform} HTTP header and \c{platform} member of NavigatorUAData in JavaScript.

    Can not be disabled.
*/
QString QWebEngineClientHints::platform() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAPlatform).toString();
}

/*!
    \property QWebEngineClientHints::model
    The value of the \c{Sec-CH-UA-Model} HTTP header and \c{model} member of NavigatorUAData in JavaScript.
*/
QString QWebEngineClientHints::model() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAModel).toString();
}

/*!
    \property QWebEngineClientHints::mobile
    The value of the \c{Sec-CH-UA-Mobile} HTTP header and \c{mobile} member of NavigatorUAData in JavaScript.

    Can not be disabled.
*/
bool QWebEngineClientHints::isMobile() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAMobile).toBool();
}

/*!
    \property QWebEngineClientHints::fullVersion
    The value of the \c{Sec-CH-UA-Full-Version} HTTP header and \c{uaFullVersion} member of NavigatorUAData in JavaScript.
*/
QString QWebEngineClientHints::fullVersion() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAFullVersion).toString();
}

/*!
    \property QWebEngineClientHints::platformVersion
    The value of the \c{Sec-CH-UA-Platform-Version} HTTP header and \c{platformVersion} member of NavigatorUAData in JavaScript.
*/
QString QWebEngineClientHints::platformVersion() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAPlatformVersion).toString();
}

/*!
    \property QWebEngineClientHints::bitness
    The value of the \c{Sec-CH-UA-Bitness} HTTP header and \c{bitness} member of NavigatorUAData in JavaScript.
*/
QString QWebEngineClientHints::bitness() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UABitness).toString();
}

/*!
    \property QWebEngineClientHints::fullVersionList
    The value of the \c{Sec-CH-UA-Full-Version-List} HTTP header and \c{fullVersionList} member of NavigatorUAData in JavaScript.

    It holds brand name and version number pairs in a QHash. The provided values will be automatically extended by the currently used version
    of Chromium and a semi-random brand.
*/
QHash<QString,QString> QWebEngineClientHints::fullVersionList() const
{
    QHash<QString, QString> ret;
    QJsonObject fullVersionList = m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAFullVersionList).toJsonObject();
    for (const QString &key : fullVersionList.keys())
        ret.insert(key, fullVersionList.value(key).toString());
    return ret;
}

/*!
    \property QWebEngineClientHints::wow64
    The value of the \c{Sec-CH-UA-Wow64} HTTP header and \c{wow64} member of NavigatorUAData in JavaScript.
*/
bool QWebEngineClientHints::isWow64() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAWOW64).toBool();
}

void QWebEngineClientHints::setArch(const QString &arch)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAArchitecture, QVariant(arch));
}

void QWebEngineClientHints::setPlatform(const QString &platform)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAPlatform, QVariant(platform));
}

void QWebEngineClientHints::setModel(const QString &model)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAModel, QVariant(model));
}

void QWebEngineClientHints::setIsMobile(const bool mobile)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAMobile, QVariant(mobile));
}

void QWebEngineClientHints::setFullVersion(const QString &fullVerson)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAFullVersion, QVariant(fullVerson));
}

void QWebEngineClientHints::setPlatformVersion(const QString &platformVersion)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAPlatformVersion, QVariant(platformVersion));
}

void QWebEngineClientHints::setBitness(const QString &bitness)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UABitness, QVariant(bitness));
}

void QWebEngineClientHints::setFullVersionList(const QHash<QString,QString> &fullVersionList)
{
    QJsonObject jsonObject;
    for (auto i = fullVersionList.cbegin(), end = fullVersionList.cend(); i != end; ++i)
        jsonObject.insert(i.key(), QJsonValue(i.value()));
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAFullVersionList, QVariant(jsonObject));
}

void QWebEngineClientHints::setIsWow64(const bool wow64)
{
    m_profileAdapter->setClientHint(QtWebEngineCore::ProfileAdapter::UAWOW64, QVariant(wow64));
}

/*!
    \property QWebEngineClientHints::isAllClientHintsEnabled
    This property controls whether the Client Hints HTTP headers are sent by WebEngine or not.

    Enabled by default.
*/
bool QWebEngineClientHints::isAllClientHintsEnabled()
{
    return m_profileAdapter->clientHintsEnabled();
}

void QWebEngineClientHints::setAllClientHintsEnabled(bool enabled)
{
    m_profileAdapter->setClientHintsEnabled(enabled);
}

/*!
    Resets all Client Hints settings to their default values.
*/
void QWebEngineClientHints::resetAll()
{
    m_profileAdapter->resetClientHints();
}

QT_END_NAMESPACE
