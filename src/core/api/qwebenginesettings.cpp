// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginesettings.h"
#include "web_engine_settings.h"

QT_BEGIN_NAMESPACE

using QtWebEngineCore::WebEngineSettings;

QWebEngineSettings::QWebEngineSettings(QWebEngineSettings *parentSettings)
    : d_ptr(new WebEngineSettings(parentSettings ? parentSettings->d_ptr.data() : nullptr))
{
    d_ptr->scheduleApplyRecursively();
}

QWebEngineSettings::~QWebEngineSettings() { }

/*
    Returns the settings for a web engine page that belongs to the default
    profile. All web pages not specifically created with another profile belong
    to the default profile.

QWebEngineSettings *QWebEngineSettings::defaultSettings()
{
    return QWebEngineProfile::defaultProfile()->settings();
}
*/

void QWebEngineSettings::setFontFamily(QWebEngineSettings::FontFamily which, const QString &family)
{
    d_ptr->setFontFamily(which, family);
}

QString QWebEngineSettings::fontFamily(QWebEngineSettings::FontFamily which) const
{
    return d_ptr->fontFamily(which);
}

void QWebEngineSettings::resetFontFamily(QWebEngineSettings::FontFamily which)
{
    d_ptr->resetFontFamily(which);
}

void QWebEngineSettings::setFontSize(QWebEngineSettings::FontSize type, int size)
{
    d_ptr->setFontSize(type, size);
}

int QWebEngineSettings::fontSize(QWebEngineSettings::FontSize type) const
{
    return d_ptr->fontSize(type);
}

void QWebEngineSettings::resetFontSize(QWebEngineSettings::FontSize type)
{
    d_ptr->resetFontSize(type);
}

QString QWebEngineSettings::defaultTextEncoding() const
{
    return d_ptr->defaultTextEncoding();
}

QWebEngineSettings::UnknownUrlSchemePolicy QWebEngineSettings::unknownUrlSchemePolicy() const
{
    return d_ptr->unknownUrlSchemePolicy();
}

void QWebEngineSettings::resetUnknownUrlSchemePolicy()
{
    d_ptr->setUnknownUrlSchemePolicy(QWebEngineSettings::InheritedUnknownUrlSchemePolicy);
}

void QWebEngineSettings::setAttribute(QWebEngineSettings::WebAttribute attr, bool on)
{
    d_ptr->setAttribute(attr, on);
}

bool QWebEngineSettings::testAttribute(QWebEngineSettings::WebAttribute attr) const
{
    return d_ptr->testAttribute(attr);
}

void QWebEngineSettings::resetAttribute(QWebEngineSettings::WebAttribute attr)
{
    d_ptr->resetAttribute(attr);
}

void QWebEngineSettings::setDefaultTextEncoding(const QString &encoding)
{
    d_ptr->setDefaultTextEncoding(encoding);
}

void QWebEngineSettings::setUnknownUrlSchemePolicy(
        QWebEngineSettings::UnknownUrlSchemePolicy policy)
{
    d_ptr->setUnknownUrlSchemePolicy(policy);
}

void QWebEngineSettings::setParentSettings(QWebEngineSettings *parentSettings)
{
    d_ptr->setParentSettings(parentSettings->d_ptr.data());
}

QT_END_NAMESPACE
