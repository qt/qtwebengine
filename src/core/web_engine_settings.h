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
#ifndef WEB_ENGINE_SETTINGS_H
#define WEB_ENGINE_SETTINGS_H

#include "qtwebenginecoreglobal.h"

#include <QExplicitlySharedDataPointer>
#include <QScopedPointer>
#include <QHash>
#include <QUrl>

class BatchTimer;
class WebContentsAdapter;
class WebEngineSettings;
struct WebPreferences;

class QWEBENGINE_EXPORT WebEngineSettingsDelegate {
public:
    virtual ~WebEngineSettingsDelegate() {}
    virtual void apply() = 0;
    // Needs to be a valid pointer, the last available fallback (ex: global settings) should return itself.
    virtual WebEngineSettings *fallbackSettings() const = 0;
};

class QWEBENGINE_EXPORT WebEngineSettings {
public:
    // Attributes. Names match the ones from the public widgets API.
    enum Attribute {
        UnsupportedInCoreSettings = -1,
        AutoLoadImages,
        JavascriptEnabled,
        JavaEnabled,
        PluginsEnabled,
        JavascriptCanOpenWindows,
        JavascriptCanAccessClipboard,
        LinksIncludedInFocusChain,
        PrintElementBackgrounds,
        LocalStorageEnabled,
        LocalContentCanAccessRemoteUrls,
        DnsPrefetchEnabled,
        XSSAuditingEnabled,
        AcceleratedCompositingEnabled,
        SpatialNavigationEnabled,
        LocalContentCanAccessFileUrls,
        SiteSpecificQuirksEnabled,
        JavascriptCanCloseWindows,
        WebGLEnabled,
        HyperlinkAuditingEnabled,
        ScrollAnimatorEnabled,
        CaretBrowsingEnabled,
    };

    // Must match the values from the public API in qwebenginesettings.h.
    enum FontFamily {
        StandardFont,
        FixedFont,
        SerifFont,
        SansSerifFont,
        CursiveFont,
        FantasyFont
    };

    // Must match the values from the public API in qwebenginesettings.h.
    enum FontSize {
        MinimumFontSize,
        MinimumLogicalFontSize,
        DefaultFontSize,
        DefaultFixedFontSize
    };

    static void enableOffTheRecord(bool on);
    static bool useOffTheRecordBrowserContextForNewWebContents();

    WebEngineSettings(WebEngineSettingsDelegate*);
    WebEngineSettings(WebEngineSettingsDelegate *,WebContentsAdapter *);
    virtual ~WebEngineSettings();

    void overrideWebPreferences(WebPreferences *prefs);

    void setAttribute(Attribute, bool on);
    bool testAttribute(Attribute) const;
    void resetAttribute(Attribute);

    void setFontFamily(FontFamily, const QString &);
    QString fontFamily(FontFamily);
    void resetFontFamily(FontFamily);

    void setFontSize(FontSize type, int size);
    int fontSize(FontSize type) const;
    void resetFontSize(FontSize type);

    void setDefaultTextEncoding(const QString &encoding);
    QString defaultTextEncoding() const;

    bool isOffTheRecord() const;

    void initDefaults();
    void scheduleApply();

private:
    void doApply();
    void applySettingsToWebPreferences(WebPreferences *);

    static bool s_useOffTheRecordBrowserContextForNewWebContents;

    WebContentsAdapter* m_adapter;
    WebEngineSettingsDelegate* m_delegate;
    QHash<Attribute, bool> m_attributes;
    QHash<FontFamily, QString> m_fontFamilies;
    QHash<FontSize, int> m_fontSizes;
    QString m_defaultEncoding;
    QScopedPointer<WebPreferences> webPreferences;
    QScopedPointer<BatchTimer> m_batchTimer;

    friend class BatchTimer;
};

#endif // WEB_ENGINE_SETTINGS_H
