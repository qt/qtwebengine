/*
    Copyright (C) 2015 The Qt Company Ltd.
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBENGINESETTINGS_H
#define QWEBENGINESETTINGS_H

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstring.h>

namespace QtWebEngineCore {
class WebEngineSettings;
}

QT_BEGIN_NAMESPACE

class QIcon;
class QPixmap;
class QUrl;

class QWEBENGINEWIDGETS_EXPORT QWebEngineSettings {
public:
    enum FontFamily {
        StandardFont,
        FixedFont,
        SerifFont,
        SansSerifFont,
        CursiveFont,
        FantasyFont
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
        FullScreenSupportEnabled
    };

    enum FontSize {
        MinimumFontSize,
        MinimumLogicalFontSize,
        DefaultFontSize,
        DefaultFixedFontSize
    };

#if QT_DEPRECATED_SINCE(5, 5)
    static QWebEngineSettings *globalSettings();
#endif
    static QWebEngineSettings *defaultSettings();

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

private:
    Q_DISABLE_COPY(QWebEngineSettings)
    typedef ::QtWebEngineCore::WebEngineSettings QWebEngineSettingsPrivate;
    QWebEngineSettingsPrivate* d_func() { return d_ptr.data(); }
    const QWebEngineSettingsPrivate* d_func() const { return d_ptr.data(); }
    QScopedPointer<QWebEngineSettingsPrivate> d_ptr;
    friend class QWebEnginePagePrivate;
    friend class QWebEngineProfilePrivate;

    ~QWebEngineSettings();
    explicit QWebEngineSettings(QWebEngineSettings *parentSettings = 0);
};

QT_END_NAMESPACE

#endif // QWEBENGINESETTINGS_H
