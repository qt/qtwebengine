/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
