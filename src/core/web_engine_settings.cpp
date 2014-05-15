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

#include "web_engine_settings.h"
#include "web_contents_adapter.h"
#include "type_conversion.h"

#include "webkit/common/webpreferences.h"

#include <QFont>
#include <QTimer>

static const int batchTimerTimeout = 100;

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

#include "web_engine_settings.moc"

bool WebEngineSettings::s_useOffTheRecordBrowserContextForNewWebContents = false;

void WebEngineSettings::enableOffTheRecord(bool on)
{
    s_useOffTheRecordBrowserContextForNewWebContents = on;
}

bool WebEngineSettings::useOffTheRecordBrowserContextForNewWebContents()
{
    return s_useOffTheRecordBrowserContextForNewWebContents;
}

WebEngineSettings::WebEngineSettings(WebEngineSettingsDelegate *delegate)
    : m_adapter(0)
    , m_delegate(delegate)
    , m_batchTimer(new BatchTimer(this))
{
    Q_ASSERT(delegate);
}

WebEngineSettings::WebEngineSettings(WebEngineSettingsDelegate *delegate, WebContentsAdapter *adapter)
    : m_adapter(adapter)
    , m_delegate(delegate)
    , m_batchTimer(new BatchTimer(this))
{
    Q_ASSERT(delegate);
}

WebEngineSettings::~WebEngineSettings()
{
}

void WebEngineSettings::overrideWebPreferences(WebPreferences *prefs)
{
    // Apply our settings on top of those.
    applySettingsToWebPreferences(prefs);
    // Store the current webPreferences in use if this is the first time we get here
    // as the host process already overides some of the default WebPreferences values
    // before we get here (e.g. number_of_cpu_cores).
    if (webPreferences.isNull())
        webPreferences.reset(new WebPreferences(*prefs));
}

void WebEngineSettings::setAttribute(WebEngineSettings::Attribute attr, bool on)
{
    m_attributes.insert(attr, on);
    m_delegate->apply();
}

bool WebEngineSettings::testAttribute(WebEngineSettings::Attribute attr) const
{
    WebEngineSettings *fallback = m_delegate->fallbackSettings();
    Q_ASSERT(fallback);
    if (this == fallback) {
        Q_ASSERT(m_attributes.contains(attr));
        return m_attributes.value(attr);
    }
    return m_attributes.value(attr, fallback->testAttribute(attr));
}

void WebEngineSettings::resetAttribute(WebEngineSettings::Attribute attr)
{
    if (this == m_delegate->fallbackSettings())
        return;
    m_attributes.remove(attr);
    m_delegate->apply();
}

void WebEngineSettings::setFontFamily(WebEngineSettings::FontFamily which, const QString &family)
{
    m_fontFamilies.insert(which, family);
    m_delegate->apply();
}

QString WebEngineSettings::fontFamily(WebEngineSettings::FontFamily which)
{
    WebEngineSettings *fallback = m_delegate->fallbackSettings();
    Q_ASSERT(fallback);
    if (this == fallback) {
        Q_ASSERT(m_fontFamilies.contains(which));
        return m_fontFamilies.value(which);
    }
    return m_fontFamilies.value(which, fallback->fontFamily(which));
}

void WebEngineSettings::resetFontFamily(WebEngineSettings::FontFamily which)
{
    if (this == m_delegate->fallbackSettings())
        return;
    m_fontFamilies.remove(which);
    m_delegate->apply();
}

void WebEngineSettings::setFontSize(WebEngineSettings::FontSize type, int size)
{
    m_fontSizes.insert(type, size);
    m_delegate->apply();
}

int WebEngineSettings::fontSize(WebEngineSettings::FontSize type) const
{
    WebEngineSettings *fallback = m_delegate->fallbackSettings();
    Q_ASSERT(fallback);
    if (this == fallback) {
        Q_ASSERT(m_fontSizes.contains(type));
        return m_fontSizes.value(type);
    }
    return m_fontSizes.value(type, fallback->fontSize(type));
}

void WebEngineSettings::resetFontSize(WebEngineSettings::FontSize type)
{
    if (this == m_delegate->fallbackSettings())
        return;
    m_fontSizes.remove(type);
    m_delegate->apply();
}

void WebEngineSettings::setDefaultTextEncoding(const QString &encoding)
{
    m_defaultEncoding = encoding;
    m_delegate->apply();
}

QString WebEngineSettings::defaultTextEncoding() const
{
    WebEngineSettings *fallback = m_delegate->fallbackSettings();
    Q_ASSERT(fallback);
    if (this == fallback)
        return m_defaultEncoding;
    return m_defaultEncoding.isEmpty()? fallback->defaultTextEncoding() : m_defaultEncoding;
}

bool WebEngineSettings::isOffTheRecord() const
{
    return m_adapter ? m_adapter->isOffTheRecord() : true;
}

void WebEngineSettings::initDefaults()
{
    // Initialize the default settings.

    m_attributes.insert(AutoLoadImages, true);
    m_attributes.insert(JavascriptEnabled, true);
    m_attributes.insert(JavaEnabled, false);
    m_attributes.insert(PluginsEnabled, false);
    m_attributes.insert(JavascriptCanOpenWindows, false);
    m_attributes.insert(JavascriptCanAccessClipboard, false);
    m_attributes.insert(LinksIncludedInFocusChain, true);
    m_attributes.insert(PrintElementBackgrounds, true);
    m_attributes.insert(LocalStorageEnabled, false);
    m_attributes.insert(LocalContentCanAccessRemoteUrls, false);
    m_attributes.insert(DnsPrefetchEnabled, false);
    m_attributes.insert(XSSAuditingEnabled, false);
    m_attributes.insert(AcceleratedCompositingEnabled, true);
    m_attributes.insert(SpatialNavigationEnabled, false);
    m_attributes.insert(LocalContentCanAccessFileUrls, true);
    m_attributes.insert(SiteSpecificQuirksEnabled, true);
    m_attributes.insert(JavascriptCanCloseWindows, false);
    m_attributes.insert(WebGLEnabled, true);
    m_attributes.insert(HyperlinkAuditingEnabled, false);
    m_attributes.insert(ScrollAnimatorEnabled, false);
    m_attributes.insert(CaretBrowsingEnabled, false);

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
    // FIXME: batch sequential calls to apply?
    applySettingsToWebPreferences(webPreferences.data());

    if (m_adapter)
        m_adapter->updateWebPreferences(*webPreferences.data());
}

void WebEngineSettings::applySettingsToWebPreferences(WebPreferences *prefs)
{

    // Attributes mapping.
    prefs->loads_images_automatically = testAttribute(AutoLoadImages);
    prefs->javascript_enabled = testAttribute(JavascriptEnabled);
    prefs->java_enabled = testAttribute(JavaEnabled);
    prefs->plugins_enabled = testAttribute(PluginsEnabled);
    prefs->javascript_can_open_windows_automatically = testAttribute(JavascriptCanOpenWindows);
    prefs->javascript_can_access_clipboard = testAttribute(JavascriptCanAccessClipboard);
    prefs->tabs_to_links = testAttribute(LinksIncludedInFocusChain);
    prefs->should_print_backgrounds = testAttribute(PrintElementBackgrounds);
    prefs->local_storage_enabled = testAttribute(LocalStorageEnabled);
    prefs->allow_universal_access_from_file_urls = testAttribute(LocalContentCanAccessRemoteUrls);
    prefs->dns_prefetching_enabled = testAttribute(DnsPrefetchEnabled);
    prefs->xss_auditor_enabled = testAttribute(XSSAuditingEnabled);
    prefs->accelerated_compositing_enabled = testAttribute(AcceleratedCompositingEnabled);
    prefs->spatial_navigation_enabled = testAttribute(SpatialNavigationEnabled);
    prefs->allow_file_access_from_file_urls = testAttribute(LocalContentCanAccessFileUrls);
    prefs->site_specific_quirks_enabled = testAttribute(SiteSpecificQuirksEnabled);
    prefs->allow_scripts_to_close_windows = testAttribute(JavascriptCanCloseWindows);
    prefs->experimental_webgl_enabled = testAttribute(WebGLEnabled);
    prefs->hyperlink_auditing_enabled = testAttribute(HyperlinkAuditingEnabled);
    prefs->enable_scroll_animator = testAttribute(ScrollAnimatorEnabled);
    prefs->caret_browsing_enabled = testAttribute(CaretBrowsingEnabled);

    // Fonts settings.
    prefs->standard_font_family_map[webkit_glue::kCommonScript] = toString16(fontFamily(StandardFont));
    prefs->fixed_font_family_map[webkit_glue::kCommonScript] = toString16(fontFamily(FixedFont));
    prefs->serif_font_family_map[webkit_glue::kCommonScript] = toString16(fontFamily(SerifFont));
    prefs->sans_serif_font_family_map[webkit_glue::kCommonScript] = toString16(fontFamily(SansSerifFont));
    prefs->cursive_font_family_map[webkit_glue::kCommonScript] = toString16(fontFamily(CursiveFont));
    prefs->fantasy_font_family_map[webkit_glue::kCommonScript] = toString16(fontFamily(FantasyFont));
    // FIXME: add pictograph?
    //    prefs.pictograph_font_family_map[webkit_glue::kCommonScript] = toString16(fontFamily());
    prefs->default_font_size = fontSize(DefaultFontSize);
    prefs->default_fixed_font_size = fontSize(DefaultFixedFontSize);
    prefs->minimum_font_size = fontSize(MinimumFontSize);
    prefs->minimum_logical_font_size = fontSize(MinimumLogicalFontSize);
}
