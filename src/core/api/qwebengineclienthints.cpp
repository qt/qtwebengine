// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineclienthints.h"

#include "profile_adapter.h"

#include <QJsonObject>

QT_BEGIN_NAMESPACE

QWebEngineClientHints::QWebEngineClientHints(QtWebEngineCore::ProfileAdapter *profileAdapter)
    : m_profileAdapter(profileAdapter)
{
}

QWebEngineClientHints::~QWebEngineClientHints()
{
}

QString QWebEngineClientHints::arch() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAArchitecture).toString();
}

QString QWebEngineClientHints::platform() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAPlatform).toString();
}

QString QWebEngineClientHints::model() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAModel).toString();
}

bool QWebEngineClientHints::isMobile() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAMobile).toBool();
}

QString QWebEngineClientHints::fullVersion() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAFullVersion).toString();
}

QString QWebEngineClientHints::platformVersion() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAPlatformVersion).toString();
}

QString QWebEngineClientHints::bitness() const
{
    return m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UABitness).toString();
}

QHash<QString,QString> QWebEngineClientHints::fullVersionList() const
{
    QHash<QString, QString> ret;
    QJsonObject fullVersionList = m_profileAdapter->clientHint(QtWebEngineCore::ProfileAdapter::UAFullVersionList).toJsonObject();
    for (const QString &key : fullVersionList.keys())
        ret.insert(key, fullVersionList.value(key).toString());
    return ret;
}

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

bool QWebEngineClientHints::isAllClientHintsEnabled()
{
    return m_profileAdapter->clientHintsEnabled();
}

void QWebEngineClientHints::setAllClientHintsEnabled(bool enabled)
{
    m_profileAdapter->setClientHintsEnabled(enabled);
}

void QWebEngineClientHints::resetAll()
{
    m_profileAdapter->resetClientHints();
}

QT_END_NAMESPACE
