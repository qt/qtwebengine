// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef WEB_ENGINE_SETTINGS_H
#define WEB_ENGINE_SETTINGS_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtWebEngineCore/qwebenginesettings.h>
#include <QScopedPointer>
#include <QHash>
#include <QUrl>
#include <QSet>
#include <QTimer>

namespace content {
class WebContents;
}

namespace blink {
struct RendererPreferences;
namespace web_pref {
struct WebPreferences;
}
}
namespace QtWebEngineCore {

class WebContentsAdapter;

class WebEngineSettings {
public:
    static WebEngineSettings* get(QWebEngineSettings *settings) { return settings->d_ptr.data(); }

    explicit WebEngineSettings(WebEngineSettings *parentSettings = nullptr);
    ~WebEngineSettings();

    void setParentSettings(WebEngineSettings *parentSettings);

    void overrideWebPreferences(content::WebContents *webContents, blink::web_pref::WebPreferences *prefs);

    void setAttribute(QWebEngineSettings::WebAttribute, bool on);
    bool testAttribute(QWebEngineSettings::WebAttribute) const;
    void resetAttribute(QWebEngineSettings::WebAttribute);
    bool isAttributeExplicitlySet(QWebEngineSettings::WebAttribute) const;

    void setFontFamily(QWebEngineSettings::FontFamily, const QString &);
    QString fontFamily(QWebEngineSettings::FontFamily);
    void resetFontFamily(QWebEngineSettings::FontFamily);

    void setFontSize(QWebEngineSettings::FontSize type, int size);
    int fontSize(QWebEngineSettings::FontSize type) const;
    void resetFontSize(QWebEngineSettings::FontSize type);

    void setDefaultTextEncoding(const QString &encoding);
    QString defaultTextEncoding() const;

    void setUnknownUrlSchemePolicy(QWebEngineSettings::UnknownUrlSchemePolicy policy);
    QWebEngineSettings::UnknownUrlSchemePolicy unknownUrlSchemePolicy() const;

    void scheduleApply();

    void scheduleApplyRecursively();

    bool getJavaScriptCanOpenWindowsAutomatically();

private:
    void initDefaults();
    void doApply();
    void applySettingsToWebPreferences(blink::web_pref::WebPreferences *);
    bool applySettingsToRendererPreferences(blink::RendererPreferences *);
    void setWebContentsAdapter(WebContentsAdapter *adapter) { m_adapter = adapter; }

    WebContentsAdapter* m_adapter;
    QHash<QWebEngineSettings::WebAttribute, bool> m_attributes;
    QHash<QWebEngineSettings::FontFamily, QString> m_fontFamilies;
    QHash<QWebEngineSettings::FontSize, int> m_fontSizes;
    QString m_defaultEncoding;
    QScopedPointer<blink::web_pref::WebPreferences> webPreferences;
    QTimer m_batchTimer;

    WebEngineSettings *parentSettings;
    QSet<WebEngineSettings *> childSettings;

    static QHash<QWebEngineSettings::WebAttribute, bool> s_defaultAttributes;
    static QHash<QWebEngineSettings::FontFamily, QString> s_defaultFontFamilies;
    static QHash<QWebEngineSettings::FontSize, int> s_defaultFontSizes;
    QWebEngineSettings::UnknownUrlSchemePolicy m_unknownUrlSchemePolicy;

    friend class WebContentsAdapter;
};

} // namespace QtWebEngineCore

#endif // WEB_ENGINE_SETTINGS_H
