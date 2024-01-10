// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINESETTINGS_H
#define QWEBENGINESETTINGS_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstring.h>

namespace QtWebEngineCore {
class WebEngineSettings;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineSettings
{
public:
    enum FontFamily {
        StandardFont,
        FixedFont,
        SerifFont,
        SansSerifFont,
        CursiveFont,
        FantasyFont,
        PictographFont
    };

    enum WebAttribute {
        AutoLoadImages,
        JavascriptEnabled,
        JavascriptCanOpenWindows,
        JavascriptCanAccessClipboard,
        LinksIncludedInFocusChain,
        LocalStorageEnabled,
        LocalContentCanAccessRemoteUrls,
        XSSAuditingEnabled,
        SpatialNavigationEnabled,
        LocalContentCanAccessFileUrls,
        HyperlinkAuditingEnabled,
        ScrollAnimatorEnabled,
        ErrorPageEnabled,
        PluginsEnabled,
        FullScreenSupportEnabled,
        ScreenCaptureEnabled,
        WebGLEnabled,
        Accelerated2dCanvasEnabled,
        AutoLoadIconsForPage,
        TouchIconsEnabled,
        FocusOnNavigationEnabled,
        PrintElementBackgrounds,
        AllowRunningInsecureContent,
        AllowGeolocationOnInsecureOrigins,
        AllowWindowActivationFromJavaScript,
        ShowScrollBars,
        PlaybackRequiresUserGesture,
        WebRTCPublicInterfacesOnly,
        JavascriptCanPaste,
        DnsPrefetchEnabled,
        PdfViewerEnabled,
        NavigateOnDropEnabled,
        ReadingFromCanvasEnabled,
    };

    enum FontSize {
        MinimumFontSize,
        MinimumLogicalFontSize,
        DefaultFontSize,
        DefaultFixedFontSize
    };

    enum UnknownUrlSchemePolicy {
        InheritedUnknownUrlSchemePolicy = 0, // TODO: hide
        DisallowUnknownUrlSchemes = 1,
        AllowUnknownUrlSchemesFromUserInteraction,
        AllowAllUnknownUrlSchemes
    };

public:
    ~QWebEngineSettings();
    void setFontFamily(FontFamily which, const QString &family);
    QString fontFamily(FontFamily which) const;
    void resetFontFamily(FontFamily which);

    void setFontSize(FontSize type, int size);
    int fontSize(FontSize type) const;
    void resetFontSize(FontSize type);

    void setAttribute(WebAttribute attr, bool on);
    bool testAttribute(WebAttribute attr) const;
    void resetAttribute(WebAttribute attr);

    void setDefaultTextEncoding(const QString &encoding);
    QString defaultTextEncoding() const;

    UnknownUrlSchemePolicy unknownUrlSchemePolicy() const;
    void setUnknownUrlSchemePolicy(UnknownUrlSchemePolicy policy);
    void resetUnknownUrlSchemePolicy();

private:
    explicit QWebEngineSettings(QWebEngineSettings *parentSettings = nullptr);
    void setParentSettings(QWebEngineSettings *parentSettings);
    Q_DISABLE_COPY(QWebEngineSettings)
    typedef ::QtWebEngineCore::WebEngineSettings QWebEngineSettingsPrivate;
    QScopedPointer<QWebEngineSettingsPrivate> d_ptr;
    friend class QWebEnginePagePrivate;
    friend class QWebEngineProfilePrivate;
    friend class QQuickWebEngineSettings;
    friend class QtWebEngineCore::WebEngineSettings;
};

QT_END_NAMESPACE

#endif // QWEBENGINESETTINGS_H
