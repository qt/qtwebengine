// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "web_engine_settings.h"

#include "web_contents_adapter.h"
#include "web_engine_context.h"
#include "type_conversion.h"

#include "base/command_line.h"
#include "chrome/common/chrome_switches.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "media/base/media_switches.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/blink/public/common/peerconnection/webrtc_ip_handling_policy.h"
#include "third_party/blink/public/common/renderer_preferences/renderer_preferences.h"
#include "third_party/blink/public/common/web_preferences/web_preferences.h"
#include "ui/base/ui_base_switches.h"
#include "ui/base/pointer/pointer_device.h"
#include "ui/events/event_switches.h"
#include "ui/native_theme/native_theme.h"

#include <QFont>
#include <QTimer>

namespace QtWebEngineCore {

QHash<QWebEngineSettings::WebAttribute, bool> WebEngineSettings::s_defaultAttributes;
QHash<QWebEngineSettings::FontFamily, QString> WebEngineSettings::s_defaultFontFamilies;
QHash<QWebEngineSettings::FontSize, int> WebEngineSettings::s_defaultFontSizes;

static const int batchTimerTimeout = 0;

static inline bool isTouchEventsAPIEnabled() {
    static bool initialized = false;
    static bool touchEventsAPIEnabled = false;
    if (!initialized) {
        base::CommandLine *parsedCommandLine = base::CommandLine::ForCurrentProcess();

        // By default the Touch Events API support (presence of 'ontouchstart' in 'window' object)
        // will be determined based on the availability of touch screen devices.
        const std::string touchEventsSwitchValue =
            parsedCommandLine->HasSwitch(switches::kTouchEventFeatureDetection) ?
                parsedCommandLine->GetSwitchValueASCII(switches::kTouchEventFeatureDetection) :
                switches::kTouchEventFeatureDetectionAuto;

        if (touchEventsSwitchValue == switches::kTouchEventFeatureDetectionEnabled)
            touchEventsAPIEnabled = true;
        else if (touchEventsSwitchValue == switches::kTouchEventFeatureDetectionAuto)
            touchEventsAPIEnabled = (ui::GetTouchScreensAvailability() == ui::TouchScreensAvailability::ENABLED);

        initialized = true;
    }
    return touchEventsAPIEnabled;
}

WebEngineSettings::WebEngineSettings(WebEngineSettings *_parentSettings)
    : m_adapter(nullptr)
    , parentSettings(_parentSettings)
    , m_unknownUrlSchemePolicy(QWebEngineSettings::InheritedUnknownUrlSchemePolicy)
{
    if (parentSettings)
        parentSettings->childSettings.insert(this);
    else
        initDefaults();
    m_batchTimer.setSingleShot(true);
    m_batchTimer.setInterval(batchTimerTimeout);
    QObject::connect(&m_batchTimer, &QTimer::timeout, [this]() {
        doApply();
    });
}

WebEngineSettings::~WebEngineSettings()
{
    if (parentSettings)
        parentSettings->childSettings.remove(this);
    // In QML the profile and its settings may be garbage collected before the page and its settings.
    for (WebEngineSettings *settings : std::as_const(childSettings))
        settings->parentSettings = nullptr;
}

void WebEngineSettings::overrideWebPreferences(content::WebContents *webContents, blink::web_pref::WebPreferences *prefs)
{
    // Apply our settings on top of those.
    applySettingsToWebPreferences(prefs);
    // Store the current webPreferences in use if this is the first time we get here
    // as the host process already overides some of the default WebPreferences values
    // before we get here (e.g. number_of_cpu_cores).
    if (webPreferences.isNull())
        webPreferences.reset(new blink::web_pref::WebPreferences(*prefs));

    if (webContents
            && applySettingsToRendererPreferences(webContents->GetMutableRendererPrefs())) {
        webContents->SyncRendererPrefs();
    }
}

void WebEngineSettings::setAttribute(QWebEngineSettings::WebAttribute attr, bool on)
{
    m_attributes.insert(attr, on);
    scheduleApplyRecursively();
}

bool WebEngineSettings::testAttribute(QWebEngineSettings::WebAttribute attr) const
{
    auto it = m_attributes.constFind(attr);
    if (it != m_attributes.constEnd())
        return *it;

    if (parentSettings)
        return parentSettings->testAttribute(attr);

    Q_ASSERT(s_defaultAttributes.contains(attr));
    return s_defaultAttributes.value(attr);
}

bool WebEngineSettings::isAttributeExplicitlySet(QWebEngineSettings::WebAttribute attr) const
{
    if (m_attributes.contains(attr))
        return true;

    if (parentSettings)
        return parentSettings->isAttributeExplicitlySet(attr);

    return false;
}

void WebEngineSettings::resetAttribute(QWebEngineSettings::WebAttribute attr)
{
    m_attributes.remove(attr);
    scheduleApplyRecursively();
}

void WebEngineSettings::setFontFamily(QWebEngineSettings::FontFamily which, const QString &family)
{
    m_fontFamilies.insert(which, family);
    scheduleApplyRecursively();
}

QString WebEngineSettings::fontFamily(QWebEngineSettings::FontFamily which)
{
    if (!parentSettings) {
        Q_ASSERT(s_defaultFontFamilies.contains(which));
        return m_fontFamilies.value(which, s_defaultFontFamilies.value(which));
    }
    return m_fontFamilies.value(which, parentSettings->fontFamily(which));
}

void WebEngineSettings::resetFontFamily(QWebEngineSettings::FontFamily which)
{
    m_fontFamilies.remove(which);
    scheduleApplyRecursively();
}

void WebEngineSettings::setFontSize(QWebEngineSettings::FontSize type, int size)
{
    m_fontSizes.insert(type, size);
    scheduleApplyRecursively();
}

int WebEngineSettings::fontSize(QWebEngineSettings::FontSize type) const
{
    if (!parentSettings) {
        Q_ASSERT(s_defaultFontSizes.contains(type));
        return m_fontSizes.value(type, s_defaultFontSizes.value(type));
    }
    return m_fontSizes.value(type, parentSettings->fontSize(type));
}

void WebEngineSettings::resetFontSize(QWebEngineSettings::FontSize type)
{
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

void WebEngineSettings::setUnknownUrlSchemePolicy(QWebEngineSettings::UnknownUrlSchemePolicy policy)
{
    m_unknownUrlSchemePolicy = policy;
}

QWebEngineSettings::UnknownUrlSchemePolicy WebEngineSettings::unknownUrlSchemePolicy() const
{
    // value InheritedUnknownUrlSchemePolicy means it is taken from parent, if possible. If there
    // is no parent, then AllowUnknownUrlSchemesFromUserInteraction (the default behavior) is used.
    if (m_unknownUrlSchemePolicy != QWebEngineSettings::InheritedUnknownUrlSchemePolicy)
        return m_unknownUrlSchemePolicy;
    if (parentSettings)
        return parentSettings->unknownUrlSchemePolicy();
    return QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction;
}

void WebEngineSettings::initDefaults()
{
    if (s_defaultAttributes.isEmpty()) {
        // Initialize the default settings.
        s_defaultAttributes.insert(QWebEngineSettings::AutoLoadImages, true);
        s_defaultAttributes.insert(QWebEngineSettings::JavascriptEnabled, true);
        s_defaultAttributes.insert(QWebEngineSettings::JavascriptCanOpenWindows, true);
        s_defaultAttributes.insert(QWebEngineSettings::JavascriptCanAccessClipboard, false);
        s_defaultAttributes.insert(QWebEngineSettings::LinksIncludedInFocusChain, true);
        s_defaultAttributes.insert(QWebEngineSettings::LocalStorageEnabled, true);
        s_defaultAttributes.insert(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
        s_defaultAttributes.insert(QWebEngineSettings::XSSAuditingEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::SpatialNavigationEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
        s_defaultAttributes.insert(QWebEngineSettings::HyperlinkAuditingEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::ErrorPageEnabled, true);
        s_defaultAttributes.insert(QWebEngineSettings::PluginsEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::FullScreenSupportEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::ScreenCaptureEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::ShowScrollBars, true);
        // The following defaults matches logic in render_view_host_impl.cc
        // But first we must ensure the WebContext has been initialized
        QtWebEngineCore::WebEngineContext::current();
        base::CommandLine* commandLine = base::CommandLine::ForCurrentProcess();
        bool smoothScrolling = commandLine->HasSwitch(switches::kEnableSmoothScrolling);
        bool webGL =
                !commandLine->HasSwitch(switches::kDisable3DAPIs) &&
                !commandLine->HasSwitch(switches::kDisableWebGL);
        bool accelerated2dCanvas =
                !commandLine->HasSwitch(switches::kDisableAccelerated2dCanvas);
        bool allowRunningInsecureContent = commandLine->HasSwitch(switches::kAllowRunningInsecureContent);
        s_defaultAttributes.insert(QWebEngineSettings::ScrollAnimatorEnabled, smoothScrolling);
        s_defaultAttributes.insert(QWebEngineSettings::WebGLEnabled, webGL);
        s_defaultAttributes.insert(QWebEngineSettings::Accelerated2dCanvasEnabled,
                                   accelerated2dCanvas);
        s_defaultAttributes.insert(QWebEngineSettings::AutoLoadIconsForPage, true);
        s_defaultAttributes.insert(QWebEngineSettings::TouchIconsEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::FocusOnNavigationEnabled, false);
        s_defaultAttributes.insert(QWebEngineSettings::PrintElementBackgrounds, true);
        s_defaultAttributes.insert(QWebEngineSettings::AllowRunningInsecureContent,
                                   allowRunningInsecureContent);
        s_defaultAttributes.insert(QWebEngineSettings::AllowGeolocationOnInsecureOrigins, false);
        s_defaultAttributes.insert(QWebEngineSettings::AllowWindowActivationFromJavaScript, false);
        bool playbackRequiresUserGesture = false;
        if (commandLine->HasSwitch(switches::kAutoplayPolicy))
            playbackRequiresUserGesture = (commandLine->GetSwitchValueASCII(switches::kAutoplayPolicy) != switches::autoplay::kNoUserGestureRequiredPolicy);
        s_defaultAttributes.insert(QWebEngineSettings::PlaybackRequiresUserGesture,
                                   playbackRequiresUserGesture);
        s_defaultAttributes.insert(QWebEngineSettings::WebRTCPublicInterfacesOnly, false);
        s_defaultAttributes.insert(QWebEngineSettings::JavascriptCanPaste, false);
        s_defaultAttributes.insert(QWebEngineSettings::DnsPrefetchEnabled, false);
#if QT_CONFIG(webengine_extensions) && QT_CONFIG(webengine_printing_and_pdf)
        s_defaultAttributes.insert(QWebEngineSettings::PdfViewerEnabled, true);
#else
        s_defaultAttributes.insert(QWebEngineSettings::PdfViewerEnabled, false);
#endif
        s_defaultAttributes.insert(QWebEngineSettings::NavigateOnDropEnabled, true);
        bool noReadingFromCanvas =
                commandLine->HasSwitch(switches::kDisableReadingFromCanvas);
        s_defaultAttributes.insert(QWebEngineSettings::ReadingFromCanvasEnabled, !noReadingFromCanvas);
    }

    if (s_defaultFontFamilies.isEmpty()) {
        // Default fonts
        QFont defaultFont;
        defaultFont.setStyleHint(QFont::Serif);
        s_defaultFontFamilies.insert(QWebEngineSettings::StandardFont, defaultFont.defaultFamily());
        s_defaultFontFamilies.insert(QWebEngineSettings::SerifFont, defaultFont.defaultFamily());
        s_defaultFontFamilies.insert(QWebEngineSettings::PictographFont,
                                     defaultFont.defaultFamily());

        defaultFont.setStyleHint(QFont::Fantasy);
        s_defaultFontFamilies.insert(QWebEngineSettings::FantasyFont, defaultFont.defaultFamily());

        defaultFont.setStyleHint(QFont::Cursive);
        s_defaultFontFamilies.insert(QWebEngineSettings::CursiveFont, defaultFont.defaultFamily());

        defaultFont.setStyleHint(QFont::SansSerif);
        s_defaultFontFamilies.insert(QWebEngineSettings::SansSerifFont,
                                     defaultFont.defaultFamily());

        defaultFont.setStyleHint(QFont::Monospace);
        s_defaultFontFamilies.insert(QWebEngineSettings::FixedFont, defaultFont.defaultFamily());
    }

    if (s_defaultFontSizes.isEmpty()) {
        s_defaultFontSizes.insert(QWebEngineSettings::MinimumFontSize, 0);
        s_defaultFontSizes.insert(QWebEngineSettings::MinimumLogicalFontSize, 6);
        s_defaultFontSizes.insert(QWebEngineSettings::DefaultFixedFontSize, 13);
        s_defaultFontSizes.insert(QWebEngineSettings::DefaultFontSize, 16);
    }

    m_defaultEncoding = QStringLiteral("ISO-8859-1");
    m_unknownUrlSchemePolicy = QWebEngineSettings::InheritedUnknownUrlSchemePolicy;
}

void WebEngineSettings::scheduleApply()
{
    if (!m_batchTimer.isActive())
        m_batchTimer.start();
}

void WebEngineSettings::doApply()
{
    if (webPreferences.isNull())
        return;

    m_batchTimer.stop();
    // Override with our settings when applicable
    applySettingsToWebPreferences(webPreferences.data());
    Q_ASSERT(m_adapter);
    m_adapter->updateWebPreferences(*webPreferences.data());

    if (applySettingsToRendererPreferences(m_adapter->webContents()->GetMutableRendererPrefs()))
        m_adapter->webContents()->SyncRendererPrefs();
}

void WebEngineSettings::applySettingsToWebPreferences(blink::web_pref::WebPreferences *prefs)
{
    // Not supported
    prefs->picture_in_picture_enabled = false;

    // Override for now
    prefs->touch_event_feature_detection_enabled = isTouchEventsAPIEnabled();
#if !QT_CONFIG(webengine_embedded_build)
    prefs->available_hover_types = (int)blink::mojom::HoverType::kHoverHoverType;
    prefs->primary_hover_type = blink::mojom::HoverType::kHoverHoverType;
#endif
    if (prefs->viewport_enabled) {
        // We need to enable the viewport options together as it doesn't really work
        // to enable them separately. With viewport-enabled we match Android defaults.
        prefs->viewport_meta_enabled = true;
        prefs->shrinks_viewport_contents_to_fit = true;
        prefs->main_frame_resizes_are_orientation_changes = true;
    }

    // Attributes mapping.
    prefs->loads_images_automatically = testAttribute(QWebEngineSettings::AutoLoadImages);
    prefs->javascript_enabled = testAttribute(QWebEngineSettings::JavascriptEnabled);
    prefs->javascript_can_access_clipboard =
            testAttribute(QWebEngineSettings::JavascriptCanAccessClipboard);
    prefs->tabs_to_links = testAttribute(QWebEngineSettings::LinksIncludedInFocusChain);
    prefs->local_storage_enabled = testAttribute(QWebEngineSettings::LocalStorageEnabled);
    prefs->databases_enabled = testAttribute(QWebEngineSettings::LocalStorageEnabled);
    prefs->allow_remote_access_from_local_urls =
            testAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls);
    prefs->spatial_navigation_enabled = testAttribute(QWebEngineSettings::SpatialNavigationEnabled);
    prefs->allow_file_access_from_file_urls =
            testAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls);
    prefs->hyperlink_auditing_enabled = testAttribute(QWebEngineSettings::HyperlinkAuditingEnabled);
    prefs->enable_scroll_animator = testAttribute(QWebEngineSettings::ScrollAnimatorEnabled);
    prefs->enable_error_page = testAttribute(QWebEngineSettings::ErrorPageEnabled);
    prefs->plugins_enabled = testAttribute(QWebEngineSettings::PluginsEnabled);
    prefs->fullscreen_supported = testAttribute(QWebEngineSettings::FullScreenSupportEnabled);
    prefs->accelerated_2d_canvas_enabled =
            testAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled);
    prefs->webgl1_enabled = prefs->webgl2_enabled = testAttribute(QWebEngineSettings::WebGLEnabled);
    prefs->should_print_backgrounds = testAttribute(QWebEngineSettings::PrintElementBackgrounds);
    prefs->allow_running_insecure_content =
            testAttribute(QWebEngineSettings::AllowRunningInsecureContent);
    prefs->allow_geolocation_on_insecure_origins =
            testAttribute(QWebEngineSettings::AllowGeolocationOnInsecureOrigins);
    prefs->hide_scrollbars = !testAttribute(QWebEngineSettings::ShowScrollBars);
    if (isAttributeExplicitlySet(QWebEngineSettings::PlaybackRequiresUserGesture)) {
        prefs->autoplay_policy = testAttribute(QWebEngineSettings::PlaybackRequiresUserGesture)
                               ? blink::mojom::AutoplayPolicy::kUserGestureRequired
                               : blink::mojom::AutoplayPolicy::kNoUserGestureRequired;
    }
    prefs->dom_paste_enabled = testAttribute(QWebEngineSettings::JavascriptCanPaste);
    prefs->dns_prefetching_enabled = testAttribute(QWebEngineSettings::DnsPrefetchEnabled);
    prefs->navigate_on_drag_drop = testAttribute(QWebEngineSettings::NavigateOnDropEnabled);
    prefs->disable_reading_from_canvas = !testAttribute(QWebEngineSettings::ReadingFromCanvasEnabled);

    // Fonts settings.
    prefs->standard_font_family_map[blink::web_pref::kCommonScript] =
            toString16(fontFamily(QWebEngineSettings::StandardFont));
    prefs->fixed_font_family_map[blink::web_pref::kCommonScript] =
            toString16(fontFamily(QWebEngineSettings::FixedFont));
    prefs->serif_font_family_map[blink::web_pref::kCommonScript] =
            toString16(fontFamily(QWebEngineSettings::SerifFont));
    prefs->sans_serif_font_family_map[blink::web_pref::kCommonScript] =
            toString16(fontFamily(QWebEngineSettings::SansSerifFont));
    prefs->cursive_font_family_map[blink::web_pref::kCommonScript] =
            toString16(fontFamily(QWebEngineSettings::CursiveFont));
    prefs->fantasy_font_family_map[blink::web_pref::kCommonScript] =
            toString16(fontFamily(QWebEngineSettings::FantasyFont));
    prefs->default_font_size = fontSize(QWebEngineSettings::DefaultFontSize);
    prefs->default_fixed_font_size = fontSize(QWebEngineSettings::DefaultFixedFontSize);
    prefs->minimum_font_size = fontSize(QWebEngineSettings::MinimumFontSize);
    prefs->minimum_logical_font_size = fontSize(QWebEngineSettings::MinimumLogicalFontSize);
    prefs->default_encoding = defaultTextEncoding().toStdString();

    // Set the theme colors. Based on chrome_content_browser_client.cc:
    const ui::NativeTheme *webTheme = ui::NativeTheme::GetInstanceForWeb();
    if (webTheme) {
        switch (webTheme->GetPreferredColorScheme()) {
          case ui::NativeTheme::PreferredColorScheme::kDark:
            prefs->preferred_color_scheme = blink::mojom::PreferredColorScheme::kDark;
            break;
          case ui::NativeTheme::PreferredColorScheme::kLight:
            prefs->preferred_color_scheme = blink::mojom::PreferredColorScheme::kLight;
            break;
        }
    }

    // Apply native CaptionStyle parameters.
    absl::optional<ui::CaptionStyle> style;
    if (base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kForceCaptionStyle)) {
        style = ui::CaptionStyle::FromSpec(
                    base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kForceCaptionStyle));
    }

    // Apply system caption style.
    if (!style && webTheme)
        style = webTheme->GetSystemCaptionStyle();

    if (style) {
        prefs->text_track_background_color = style->background_color;
        prefs->text_track_text_color = style->text_color;
        prefs->text_track_text_size = style->text_size;
        prefs->text_track_text_shadow = style->text_shadow;
        prefs->text_track_font_family = style->font_family;
        prefs->text_track_font_variant = style->font_variant;
        prefs->text_track_window_color = style->window_color;
        prefs->text_track_window_radius = style->window_radius;
    }
}

bool WebEngineSettings::applySettingsToRendererPreferences(blink::RendererPreferences *prefs)
{
    bool changed = false;
#if QT_CONFIG(webengine_webrtc)
    if (!base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kForceWebRtcIPHandlingPolicy)) {
        std::string webrtc_ip_handling_policy =
                testAttribute(QWebEngineSettings::WebRTCPublicInterfacesOnly)
                ? blink::kWebRTCIPHandlingDefaultPublicInterfaceOnly
                : blink::kWebRTCIPHandlingDefault;
        if (prefs->webrtc_ip_handling_policy != webrtc_ip_handling_policy) {
            prefs->webrtc_ip_handling_policy = webrtc_ip_handling_policy;
            changed = true;
        }
    }
#endif
    bool canNavigateOnDrop = testAttribute(QWebEngineSettings::NavigateOnDropEnabled);
    if (canNavigateOnDrop != prefs->can_accept_load_drops) {
        prefs->can_accept_load_drops = canNavigateOnDrop;
        changed = true;
    }
    return changed;
}

void WebEngineSettings::scheduleApplyRecursively()
{
    scheduleApply();
    for (WebEngineSettings *settings : std::as_const(childSettings)) {
        settings->scheduleApply();
    }
}

bool WebEngineSettings::getJavaScriptCanOpenWindowsAutomatically()
{
    return testAttribute(QWebEngineSettings::JavascriptCanOpenWindows);
}

void WebEngineSettings::setParentSettings(WebEngineSettings *_parentSettings)
{
    if (parentSettings)
        parentSettings->childSettings.remove(this);
    parentSettings = _parentSettings;
    if (parentSettings)
        parentSettings->childSettings.insert(this);
    scheduleApplyRecursively();
}

} // namespace QtWebEngineCore
