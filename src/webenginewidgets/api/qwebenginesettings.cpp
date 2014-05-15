/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginesettings.h"
#include "qwebenginesettings_p.h"

#include <QDebug>


static WebEngineSettings::Attribute toWebEngineAttribute(QWebEngineSettings::WebAttribute attribute) {
    switch(attribute) {
    case QWebEngineSettings::AutoLoadImages:
        return WebEngineSettings::AutoLoadImages;
    case QWebEngineSettings::JavascriptEnabled:
        return WebEngineSettings::JavascriptEnabled;
    case QWebEngineSettings::JavaEnabled:
        return WebEngineSettings::JavaEnabled;
    case QWebEngineSettings::PluginsEnabled:
        return WebEngineSettings::PluginsEnabled;
    case QWebEngineSettings::JavascriptCanOpenWindows:
        return WebEngineSettings::JavascriptCanOpenWindows;
    case QWebEngineSettings::JavascriptCanAccessClipboard:
        return WebEngineSettings::JavascriptCanAccessClipboard;
    case QWebEngineSettings::LinksIncludedInFocusChain:
        return WebEngineSettings::LinksIncludedInFocusChain;
    case QWebEngineSettings::PrintElementBackgrounds:
        return WebEngineSettings::PrintElementBackgrounds;
    case QWebEngineSettings::LocalStorageEnabled:
        return WebEngineSettings::LocalStorageEnabled;
    case QWebEngineSettings::LocalContentCanAccessRemoteUrls:
        return WebEngineSettings::LocalContentCanAccessRemoteUrls;
    case QWebEngineSettings::DnsPrefetchEnabled:
        return WebEngineSettings::DnsPrefetchEnabled;
    case QWebEngineSettings::XSSAuditingEnabled:
        return WebEngineSettings::XSSAuditingEnabled;
    case QWebEngineSettings::AcceleratedCompositingEnabled:
        return WebEngineSettings::AcceleratedCompositingEnabled;
    case QWebEngineSettings::SpatialNavigationEnabled:
        return WebEngineSettings::SpatialNavigationEnabled;
    case QWebEngineSettings::LocalContentCanAccessFileUrls:
        return WebEngineSettings::LocalContentCanAccessFileUrls;
    case QWebEngineSettings::SiteSpecificQuirksEnabled:
        return WebEngineSettings::SiteSpecificQuirksEnabled;
    case QWebEngineSettings::JavascriptCanCloseWindows:
        return WebEngineSettings::JavascriptCanCloseWindows;
    case QWebEngineSettings::WebGLEnabled:
        return WebEngineSettings::WebGLEnabled;
    case QWebEngineSettings::HyperlinkAuditingEnabled:
        return WebEngineSettings::HyperlinkAuditingEnabled;
    case QWebEngineSettings::ScrollAnimatorEnabled:
        return WebEngineSettings::ScrollAnimatorEnabled;
    case QWebEngineSettings::CaretBrowsingEnabled:
        return WebEngineSettings::CaretBrowsingEnabled;
    default:
        return WebEngineSettings::UnsupportedInCoreSettings;
    }
}

Q_GLOBAL_STATIC(QList<QWebEngineSettingsPrivate*>, allSettings);

QWebEngineSettingsPrivate::QWebEngineSettingsPrivate()
    : coreSettings(new WebEngineSettings(this))
{
}

QWebEngineSettingsPrivate::QWebEngineSettingsPrivate(WebContentsAdapter *adapter)
    : coreSettings(new WebEngineSettings(this, adapter))
{
}

void QWebEngineSettingsPrivate::apply()
{
    coreSettings->scheduleApply();
    QWebEngineSettingsPrivate *globals = QWebEngineSettings::globalSettings()->d;
    if (this == globals) {
        Q_FOREACH (QWebEngineSettingsPrivate *settings, *allSettings)
            settings->coreSettings->scheduleApply();
    }
}

void QWebEngineSettingsPrivate::initDefaults()
{
    coreSettings->initDefaults();
}

WebEngineSettings *QWebEngineSettingsPrivate::fallbackSettings() const {
    return QWebEngineSettings::globalSettings()->d->coreSettings.data();
}

QWebEngineSettings *QWebEngineSettings::globalSettings()
{
    static QWebEngineSettings* globalSettings = 0;
    if (!globalSettings)
        globalSettings = new QWebEngineSettings;
    return globalSettings;
}

Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::StandardFont) == static_cast<int>(QWebEngineSettings::StandardFont), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::FixedFont) == static_cast<int>(QWebEngineSettings::FixedFont), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::SerifFont) == static_cast<int>(QWebEngineSettings::SerifFont), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::SansSerifFont) == static_cast<int>(QWebEngineSettings::SansSerifFont), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::CursiveFont) == static_cast<int>(QWebEngineSettings::CursiveFont), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::FantasyFont) == static_cast<int>(QWebEngineSettings::FantasyFont), "The enum values must match");

void QWebEngineSettings::setFontFamily(QWebEngineSettings::FontFamily which, const QString &family)
{
    d->coreSettings->setFontFamily(static_cast<WebEngineSettings::FontFamily>(which), family);
}

QString QWebEngineSettings::fontFamily(QWebEngineSettings::FontFamily which) const
{
    return d->coreSettings->fontFamily(static_cast<WebEngineSettings::FontFamily>(which));
}

void QWebEngineSettings::resetFontFamily(QWebEngineSettings::FontFamily which)
{
    d->coreSettings->resetFontFamily(static_cast<WebEngineSettings::FontFamily>(which));
}

Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::DefaultFixedFontSize) == static_cast<int>(QWebEngineSettings::DefaultFixedFontSize), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::DefaultFontSize) == static_cast<int>(QWebEngineSettings::DefaultFontSize), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::MinimumFontSize) == static_cast<int>(QWebEngineSettings::MinimumFontSize), "The enum values must match");
Q_STATIC_ASSERT_X(static_cast<int>(WebEngineSettings::MinimumLogicalFontSize) == static_cast<int>(QWebEngineSettings::MinimumLogicalFontSize), "The enum values must match");

void QWebEngineSettings::setFontSize(QWebEngineSettings::FontSize type, int size)
{
    d->coreSettings->setFontSize(static_cast<WebEngineSettings::FontSize>(type), size);
}

int QWebEngineSettings::fontSize(QWebEngineSettings::FontSize type) const
{
    return d->coreSettings->fontSize(static_cast<WebEngineSettings::FontSize>(type));
}

void QWebEngineSettings::resetFontSize(QWebEngineSettings::FontSize type)
{
    d->coreSettings->resetFontSize(static_cast<WebEngineSettings::FontSize>(type));
}


QWebEngineSettings::QWebEngineSettings()
    : d(new QWebEngineSettingsPrivate)
{
    d->initDefaults();
}


QWebEngineSettings::QWebEngineSettings(QWebEngineSettingsPrivate *p)
    : d(p)
{
    Q_ASSERT(d);
    allSettings->append(d);
    d->coreSettings->scheduleApply();
}


QWebEngineSettings::~QWebEngineSettings()
{
    allSettings->removeAll(d);
    delete d;
}

void QWebEngineSettings::setDefaultTextEncoding(const QString &encoding)
{
    d->coreSettings->setDefaultTextEncoding(encoding);
}

QString QWebEngineSettings::defaultTextEncoding() const
{
    return d->coreSettings->defaultTextEncoding();
}

void QWebEngineSettings::setOffTheRecordEnabled(bool on)
{
    WebEngineSettings::enableOffTheRecord(on);
}

bool QWebEngineSettings::offTheRecordEnabled()
{
    return WebEngineSettings::useOffTheRecordBrowserContextForNewWebContents();
}

bool QWebEngineSettings::isOffTheRecord() const
{
    return d->coreSettings->isOffTheRecord();
}

void QWebEngineSettings::setAttribute(QWebEngineSettings::WebAttribute attr, bool on)
{
    WebEngineSettings::Attribute webEngineAttribute = toWebEngineAttribute(attr);
    if (webEngineAttribute != WebEngineSettings::UnsupportedInCoreSettings)
        return d->coreSettings->setAttribute(webEngineAttribute, on);
    qDebug() << Q_FUNC_INFO << "Missing support for:" << attr;
}

bool QWebEngineSettings::testAttribute(QWebEngineSettings::WebAttribute attr) const
{
    WebEngineSettings::Attribute webEngineAttribute = toWebEngineAttribute(attr);
    if (webEngineAttribute != WebEngineSettings::UnsupportedInCoreSettings)
        return d->coreSettings->testAttribute(webEngineAttribute);



    qDebug() << Q_FUNC_INFO << "Missing support for:" << attr;
    return false;
}

void QWebEngineSettings::resetAttribute(QWebEngineSettings::WebAttribute attr)
{
    setAttribute(attr, globalSettings()->testAttribute(attr));
}
