/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "web_engine_settings.h"
#include "web_contents_adapter.h"
#include "type_conversion.h"

#include "content/public/common/web_preferences.h"

#include <QFont>
#include <QTimer>
#include <QTouchDevice>

namespace QtWebEngineCore {

static const int batchTimerTimeout = 0;

class BatchTimer : public QTimer {
    Q_OBJECT
public:
    BatchTimer(WebEngineSettings *settings)
        : m_settings(settings)
    {
        setSingleShot(true);
        setInterval(batchTimerTimeout);
        connect(this, SIGNAL(timeout()), SLOT(onTimeout()));
    }

private Q_SLOTS:
    void onTimeout()
    {
        m_settings->doApply();
    }

private:
    WebEngineSettings *m_settings;
};

static inline bool isTouchScreenAvailable() {
    static bool initialized = false;
    static bool touchScreenAvailable = false;
    if (!initialized) {
        Q_FOREACH (const QTouchDevice *d, QTouchDevice::devices()) {
            if (d->type() == QTouchDevice::TouchScreen) {
                touchScreenAvailable = true;
                break;
            }
        }
        initialized = true;
    }
    return touchScreenAvailable;
}


WebEngineSettings::WebEngineSettings(WebEngineSettings *_parentSettings)
    : m_adapter(0)
    , m_batchTimer(new BatchTimer(this))
    , parentSettings(_parentSettings)
{
    if (parentSettings)
        parentSettings->childSettings.insert(this);
}

WebEngineSettings::~WebEngineSettings()
{
    if (parentSettings)
        parentSettings->childSettings.remove(this);
    // In QML the profile and its settings may be garbage collected before the page and its settings.
    Q_FOREACH (WebEngineSettings *settings, childSettings) {
        settings->parentSettings = 0;
    }
}

void WebEngineSettings::overrideWebPreferences(content::WebPreferences *prefs)
{
    // Apply our settings on top of those.
    applySettingsToWebPreferences(prefs);
    // Store the current webPreferences in use if this is the first time we get here
    // as the host process already overides some of the default WebPreferences values
    // before we get here (e.g. number_of_cpu_cores).
    if (webPreferences.isNull())
        webPreferences.reset(new content::WebPreferences(*prefs));
}

void WebEngineSettings::setAttribute(WebEngineSettings::Attribute attr, bool on)
{
    m_attributes.insert(attr, on);
    scheduleApplyRecursively();
}

bool WebEngineSettings::testAttribute(WebEngineSettings::Attribute attr) const
{
    if (!parentSettings) {
        Q_ASSERT(m_attributes.contains(attr));
        return m_attributes.value(attr);
    }
    return m_attributes.value(attr, parentSettings->testAttribute(attr));
}

void WebEngineSettings::resetAttribute(WebEngineSettings::Attribute attr)
{
    if (!parentSettings) // FIXME: Set initial defaults.
        return;
    m_attributes.remove(attr);
    scheduleApplyRecursively();
}

void WebEngineSettings::setFontFamily(WebEngineSettings::FontFamily which, const QString &family)
{
    m_fontFamilies.insert(which, family);
    scheduleApplyRecursively();
}

QString WebEngineSettings::fontFamily(WebEngineSettings::FontFamily which)
{
    if (!parentSettings) {
        Q_ASSERT(m_fontFamilies.contains(which));
        return m_fontFamilies.value(which);
    }
    return m_fontFamilies.value(which, parentSettings->fontFamily(which));
}

void WebEngineSettings::resetFontFamily(WebEngineSettings::FontFamily which)
{
    if (!parentSettings) // FIXME: Set initial defaults.
        return;
    m_fontFamilies.remove(which);
    scheduleApplyRecursively();
}

void WebEngineSettings::setFontSize(WebEngineSettings::FontSize type, int size)
{
    m_fontSizes.insert(type, size);
    scheduleApplyRecursively();
}

int WebEngineSettings::fontSize(WebEngineSettings::FontSize type) const
{
    if (!parentSettings) {
        Q_ASSERT(m_fontSizes.contains(type));
        return m_fontSizes.value(type);
    }
    return m_fontSizes.value(type, parentSettings->fontSize(type));
}

void WebEngineSettings::resetFontSize(WebEngineSettings::FontSize type)
{
    if (!parentSettings) // FIXME: Set initial defaults.
        return;
    m_fontSizes.remove(type);
    scheduleApplyRecursively();
}

void WebEngineSettings::setDefaultTextEncoding(const QString &encoding)
{
    m_defaultEncoding = encoding;
    scheduleApplyRecursively();
}

QString WebEngineSettings::defaultTextEncoding() const
{
    if (!parentSettings)
        return m_defaultEncoding;
    return m_defaultEncoding.isEmpty()? parentSettings->defaultTextEncoding() : m_defaultEncoding;
}

void WebEngineSettings::initDefaults(bool offTheRecord)
{
    // Initialize the default settings.
    m_attributes.insert(AutoLoadImages, true);
    m_attributes.insert(JavascriptEnabled, true);
    m_attributes.insert(JavascriptCanOpenWindows, true);
    m_attributes.insert(JavascriptCanAccessClipboard, false);
    m_attributes.insert(LinksIncludedInFocusChain, true);
    m_attributes.insert(LocalStorageEnabled, !offTheRecord);
    m_attributes.insert(LocalContentCanAccessRemoteUrls, false);
    m_attributes.insert(XSSAuditingEnabled, false);
    m_attributes.insert(SpatialNavigationEnabled, false);
    m_attributes.insert(LocalContentCanAccessFileUrls, true);
    m_attributes.insert(HyperlinkAuditingEnabled, false);
    m_attributes.insert(ScrollAnimatorEnabled, false);
    m_attributes.insert(ErrorPageEnabled, true);

    // Default fonts
    QFont defaultFont;
    defaultFont.setStyleHint(QFont::Serif);
    m_fontFamilies.insert(StandardFont, defaultFont.defaultFamily());
    m_fontFamilies.insert(SerifFont, defaultFont.defaultFamily());

    defaultFont.setStyleHint(QFont::Fantasy);
    m_fontFamilies.insert(FantasyFont, defaultFont.defaultFamily());

    defaultFont.setStyleHint(QFont::Cursive);
    m_fontFamilies.insert(CursiveFont, defaultFont.defaultFamily());

    defaultFont.setStyleHint(QFont::SansSerif);
    m_fontFamilies.insert(SansSerifFont, defaultFont.defaultFamily());

    defaultFont.setStyleHint(QFont::Monospace);
    m_fontFamilies.insert(FixedFont, defaultFont.defaultFamily());

    m_fontSizes.insert(MinimumFontSize, 0);
    m_fontSizes.insert(MinimumLogicalFontSize, 6);
    m_fontSizes.insert(DefaultFixedFontSize, 13);
    m_fontSizes.insert(DefaultFontSize, 16);

    m_defaultEncoding = QStringLiteral("ISO-8859-1");
}

void WebEngineSettings::scheduleApply()
{
    if (!m_batchTimer->isActive())
        m_batchTimer->start();
}

void WebEngineSettings::doApply()
{
    if (webPreferences.isNull())
        return;
    // Override with our settings when applicable
    applySettingsToWebPreferences(webPreferences.data());

    Q_ASSERT(m_adapter);
    m_adapter->updateWebPreferences(*webPreferences.data());
}

void WebEngineSettings::applySettingsToWebPreferences(content::WebPreferences *prefs)
{
    // Override for now
    prefs->java_enabled = false;
    prefs->touch_enabled = isTouchScreenAvailable();

    // Attributes mapping.
    prefs->loads_images_automatically = testAttribute(AutoLoadImages);
    prefs->javascript_enabled = testAttribute(JavascriptEnabled);
    prefs->javascript_can_open_windows_automatically = testAttribute(JavascriptCanOpenWindows);
    prefs->javascript_can_access_clipboard = testAttribute(JavascriptCanAccessClipboard);
    prefs->tabs_to_links = testAttribute(LinksIncludedInFocusChain);
    prefs->local_storage_enabled = testAttribute(LocalStorageEnabled);
    prefs->allow_universal_access_from_file_urls = testAttribute(LocalContentCanAccessRemoteUrls);
    prefs->xss_auditor_enabled = testAttribute(XSSAuditingEnabled);
    prefs->spatial_navigation_enabled = testAttribute(SpatialNavigationEnabled);
    prefs->allow_file_access_from_file_urls = testAttribute(LocalContentCanAccessFileUrls);
    prefs->hyperlink_auditing_enabled = testAttribute(HyperlinkAuditingEnabled);
    prefs->enable_scroll_animator = testAttribute(ScrollAnimatorEnabled);
    prefs->enable_error_page = testAttribute(ErrorPageEnabled);

    // Fonts settings.
    prefs->standard_font_family_map[content::kCommonScript] = toString16(fontFamily(StandardFont));
    prefs->fixed_font_family_map[content::kCommonScript] = toString16(fontFamily(FixedFont));
    prefs->serif_font_family_map[content::kCommonScript] = toString16(fontFamily(SerifFont));
    prefs->sans_serif_font_family_map[content::kCommonScript] = toString16(fontFamily(SansSerifFont));
    prefs->cursive_font_family_map[content::kCommonScript] = toString16(fontFamily(CursiveFont));
    prefs->fantasy_font_family_map[content::kCommonScript] = toString16(fontFamily(FantasyFont));
    // FIXME: add pictograph?
    //    prefs.pictograph_font_family_map[content::kCommonScript] = toString16(fontFamily());
    prefs->default_font_size = fontSize(DefaultFontSize);
    prefs->default_fixed_font_size = fontSize(DefaultFixedFontSize);
    prefs->minimum_font_size = fontSize(MinimumFontSize);
    prefs->minimum_logical_font_size = fontSize(MinimumLogicalFontSize);
    prefs->default_encoding = defaultTextEncoding().toStdString();
}

void WebEngineSettings::scheduleApplyRecursively()
{
    scheduleApply();
    Q_FOREACH (WebEngineSettings *settings, childSettings) {
        settings->scheduleApply();
    }
}

void WebEngineSettings::setParentSettings(WebEngineSettings *_parentSettings)
{
    if (parentSettings)
        parentSettings->childSettings.remove(this);
    parentSettings = _parentSettings;
    if (parentSettings)
        parentSettings->childSettings.insert(this);
}

} // namespace QtWebEngineCore

#include "web_engine_settings.moc"
