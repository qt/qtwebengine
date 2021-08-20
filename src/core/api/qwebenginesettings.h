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
