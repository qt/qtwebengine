// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINESETTINGS_P_H
#define QQUICKWEBENGINESETTINGS_P_H

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

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>
#include <QtQml/qqmlregistration.h>
#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>

QT_BEGIN_NAMESPACE

class QWebEngineSettings;
class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool autoLoadImages READ autoLoadImages WRITE setAutoLoadImages NOTIFY autoLoadImagesChanged FINAL)
    Q_PROPERTY(bool javascriptEnabled READ javascriptEnabled WRITE setJavascriptEnabled NOTIFY javascriptEnabledChanged FINAL)
    Q_PROPERTY(bool javascriptCanOpenWindows READ javascriptCanOpenWindows WRITE setJavascriptCanOpenWindows NOTIFY javascriptCanOpenWindowsChanged FINAL)
    Q_PROPERTY(bool javascriptCanAccessClipboard READ javascriptCanAccessClipboard WRITE setJavascriptCanAccessClipboard NOTIFY javascriptCanAccessClipboardChanged FINAL)
    Q_PROPERTY(bool linksIncludedInFocusChain READ linksIncludedInFocusChain WRITE setLinksIncludedInFocusChain NOTIFY linksIncludedInFocusChainChanged FINAL)
    Q_PROPERTY(bool localStorageEnabled READ localStorageEnabled WRITE setLocalStorageEnabled NOTIFY localStorageEnabledChanged FINAL)
    Q_PROPERTY(bool localContentCanAccessRemoteUrls READ localContentCanAccessRemoteUrls WRITE setLocalContentCanAccessRemoteUrls NOTIFY localContentCanAccessRemoteUrlsChanged FINAL)
    Q_PROPERTY(bool spatialNavigationEnabled READ spatialNavigationEnabled WRITE setSpatialNavigationEnabled NOTIFY spatialNavigationEnabledChanged FINAL)
    Q_PROPERTY(bool localContentCanAccessFileUrls READ localContentCanAccessFileUrls WRITE setLocalContentCanAccessFileUrls NOTIFY localContentCanAccessFileUrlsChanged FINAL)
    Q_PROPERTY(bool hyperlinkAuditingEnabled READ hyperlinkAuditingEnabled WRITE setHyperlinkAuditingEnabled NOTIFY hyperlinkAuditingEnabledChanged FINAL)
    Q_PROPERTY(bool errorPageEnabled READ errorPageEnabled WRITE setErrorPageEnabled NOTIFY errorPageEnabledChanged FINAL)
    Q_PROPERTY(bool pluginsEnabled READ pluginsEnabled WRITE setPluginsEnabled NOTIFY pluginsEnabledChanged FINAL)
    Q_PROPERTY(bool fullScreenSupportEnabled READ fullScreenSupportEnabled WRITE setFullScreenSupportEnabled NOTIFY fullScreenSupportEnabledChanged REVISION(1,1) FINAL)
    Q_PROPERTY(QString defaultTextEncoding READ defaultTextEncoding WRITE setDefaultTextEncoding NOTIFY defaultTextEncodingChanged FINAL)
    Q_PROPERTY(bool screenCaptureEnabled READ screenCaptureEnabled WRITE setScreenCaptureEnabled NOTIFY screenCaptureEnabledChanged REVISION(1,2) FINAL)
    Q_PROPERTY(bool webGLEnabled READ webGLEnabled WRITE setWebGLEnabled NOTIFY webGLEnabledChanged REVISION(1,2) FINAL)
    Q_PROPERTY(bool accelerated2dCanvasEnabled READ accelerated2dCanvasEnabled WRITE setAccelerated2dCanvasEnabled NOTIFY accelerated2dCanvasEnabledChanged REVISION(1,2) FINAL)
    Q_PROPERTY(bool autoLoadIconsForPage READ autoLoadIconsForPage WRITE setAutoLoadIconsForPage NOTIFY autoLoadIconsForPageChanged REVISION(1,2) FINAL)
    Q_PROPERTY(bool touchIconsEnabled READ touchIconsEnabled WRITE setTouchIconsEnabled NOTIFY touchIconsEnabledChanged REVISION(1,2) FINAL)
    Q_PROPERTY(bool focusOnNavigationEnabled READ focusOnNavigationEnabled WRITE setFocusOnNavigationEnabled NOTIFY focusOnNavigationEnabledChanged REVISION(1,3) FINAL)
    Q_PROPERTY(bool printElementBackgrounds READ printElementBackgrounds WRITE setPrintElementBackgrounds NOTIFY printElementBackgroundsChanged REVISION(1,3) FINAL)
    Q_PROPERTY(bool allowRunningInsecureContent READ allowRunningInsecureContent WRITE setAllowRunningInsecureContent NOTIFY allowRunningInsecureContentChanged REVISION(1,3) FINAL)
    Q_PROPERTY(bool allowGeolocationOnInsecureOrigins READ allowGeolocationOnInsecureOrigins WRITE setAllowGeolocationOnInsecureOrigins NOTIFY allowGeolocationOnInsecureOriginsChanged REVISION(1,4) FINAL)
    Q_PROPERTY(bool allowWindowActivationFromJavaScript READ allowWindowActivationFromJavaScript WRITE setAllowWindowActivationFromJavaScript NOTIFY allowWindowActivationFromJavaScriptChanged REVISION(1,5) FINAL)
    Q_PROPERTY(bool showScrollBars READ showScrollBars WRITE setShowScrollBars NOTIFY showScrollBarsChanged REVISION(1,5) FINAL)
    Q_PROPERTY(UnknownUrlSchemePolicy unknownUrlSchemePolicy READ unknownUrlSchemePolicy WRITE setUnknownUrlSchemePolicy NOTIFY unknownUrlSchemePolicyChanged REVISION(1,6) FINAL)
    Q_PROPERTY(bool playbackRequiresUserGesture READ playbackRequiresUserGesture WRITE setPlaybackRequiresUserGesture NOTIFY playbackRequiresUserGestureChanged REVISION(1,6) FINAL)
    Q_PROPERTY(bool webRTCPublicInterfacesOnly READ webRTCPublicInterfacesOnly WRITE setWebRTCPublicInterfacesOnly NOTIFY webRTCPublicInterfacesOnlyChanged REVISION(1,6) FINAL)
    Q_PROPERTY(bool javascriptCanPaste READ javascriptCanPaste WRITE setJavascriptCanPaste NOTIFY javascriptCanPasteChanged REVISION(1,6) FINAL)
    Q_PROPERTY(bool dnsPrefetchEnabled READ dnsPrefetchEnabled WRITE setDnsPrefetchEnabled NOTIFY dnsPrefetchEnabledChanged REVISION(1,7) FINAL)
    Q_PROPERTY(bool pdfViewerEnabled READ pdfViewerEnabled WRITE setPdfViewerEnabled NOTIFY pdfViewerEnabledChanged REVISION(1,8) FINAL)
    Q_PROPERTY(bool navigateOnDropEnabled READ navigateOnDropEnabled WRITE setNavigateOnDropEnabled NOTIFY navigateOnDropEnabledChanged REVISION(6,4) FINAL)
    Q_PROPERTY(bool readingFromCanvasEnabled READ readingFromCanvasEnabled WRITE setReadingFromCanvasEnabled NOTIFY readingFromCanvasEnabledChanged REVISION(6,6) FINAL)
    QML_NAMED_ELEMENT(WebEngineSettings)
    QML_ADDED_IN_VERSION(1, 1)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")
public:
    enum UnknownUrlSchemePolicy {
        DisallowUnknownUrlSchemes = 1,
        AllowUnknownUrlSchemesFromUserInteraction,
        AllowAllUnknownUrlSchemes
    };

    Q_ENUM(UnknownUrlSchemePolicy)

    ~QQuickWebEngineSettings();

    bool autoLoadImages() const;
    bool javascriptEnabled() const;
    bool javascriptCanOpenWindows() const;
    bool javascriptCanAccessClipboard() const;
    bool linksIncludedInFocusChain() const;
    bool localStorageEnabled() const;
    bool localContentCanAccessRemoteUrls() const;
    bool spatialNavigationEnabled() const;
    bool localContentCanAccessFileUrls() const;
    bool hyperlinkAuditingEnabled() const;
    bool errorPageEnabled() const;
    bool pluginsEnabled() const;
    bool fullScreenSupportEnabled() const;
    QString defaultTextEncoding() const;
    bool screenCaptureEnabled() const;
    bool webGLEnabled() const;
    bool accelerated2dCanvasEnabled() const;
    bool autoLoadIconsForPage() const;
    bool touchIconsEnabled() const;
    bool focusOnNavigationEnabled() const;
    bool printElementBackgrounds() const;
    bool allowRunningInsecureContent() const;
    bool allowGeolocationOnInsecureOrigins() const;
    bool allowWindowActivationFromJavaScript() const;
    bool showScrollBars() const;
    UnknownUrlSchemePolicy unknownUrlSchemePolicy() const;
    bool playbackRequiresUserGesture() const;
    bool webRTCPublicInterfacesOnly() const;
    bool javascriptCanPaste() const;
    bool dnsPrefetchEnabled() const;
    bool pdfViewerEnabled() const;
    bool navigateOnDropEnabled() const;
    bool readingFromCanvasEnabled() const;

    void setAutoLoadImages(bool on);
    void setJavascriptEnabled(bool on);
    void setJavascriptCanOpenWindows(bool on);
    void setJavascriptCanAccessClipboard(bool on);
    void setLinksIncludedInFocusChain(bool on);
    void setLocalStorageEnabled(bool on);
    void setLocalContentCanAccessRemoteUrls(bool on);
    void setSpatialNavigationEnabled(bool on);
    void setLocalContentCanAccessFileUrls(bool on);
    void setHyperlinkAuditingEnabled(bool on);
    void setErrorPageEnabled(bool on);
    void setPluginsEnabled(bool on);
    void setFullScreenSupportEnabled(bool on);
    void setDefaultTextEncoding(QString encoding);
    void setScreenCaptureEnabled(bool on);
    void setWebGLEnabled(bool on);
    void setAccelerated2dCanvasEnabled(bool on);
    void setAutoLoadIconsForPage(bool on);
    void setTouchIconsEnabled(bool on);
    void setFocusOnNavigationEnabled(bool on);
    void setPrintElementBackgrounds(bool on);
    void setAllowRunningInsecureContent(bool on);
    void setAllowGeolocationOnInsecureOrigins(bool on);
    void setAllowWindowActivationFromJavaScript(bool on);
    void setShowScrollBars(bool on);
    void setUnknownUrlSchemePolicy(UnknownUrlSchemePolicy policy);
    void setPlaybackRequiresUserGesture(bool on);
    void setWebRTCPublicInterfacesOnly(bool on);
    void setJavascriptCanPaste(bool on);
    void setDnsPrefetchEnabled(bool on);
    void setPdfViewerEnabled(bool on);
    void setNavigateOnDropEnabled(bool on);
    void setReadingFromCanvasEnabled(bool on);

signals:
    void autoLoadImagesChanged();
    void javascriptEnabledChanged();
    void javascriptCanOpenWindowsChanged();
    void javascriptCanAccessClipboardChanged();
    void linksIncludedInFocusChainChanged();
    void localStorageEnabledChanged();
    void localContentCanAccessRemoteUrlsChanged();
    void spatialNavigationEnabledChanged();
    void localContentCanAccessFileUrlsChanged();
    void hyperlinkAuditingEnabledChanged();
    void errorPageEnabledChanged();
    void pluginsEnabledChanged();
    Q_REVISION(1,1) void fullScreenSupportEnabledChanged();
    void defaultTextEncodingChanged();
    Q_REVISION(1,2) void screenCaptureEnabledChanged();
    Q_REVISION(1,2) void webGLEnabledChanged();
    Q_REVISION(1,2) void accelerated2dCanvasEnabledChanged();
    Q_REVISION(1,2) void autoLoadIconsForPageChanged();
    Q_REVISION(1,2) void touchIconsEnabledChanged();
    Q_REVISION(1,3) void focusOnNavigationEnabledChanged();
    Q_REVISION(1,3) void printElementBackgroundsChanged();
    Q_REVISION(1,3) void allowRunningInsecureContentChanged();
    Q_REVISION(1,4) void allowGeolocationOnInsecureOriginsChanged();
    Q_REVISION(1,5) void allowWindowActivationFromJavaScriptChanged();
    Q_REVISION(1,5) void showScrollBarsChanged();
    Q_REVISION(1,6) void unknownUrlSchemePolicyChanged();
    Q_REVISION(1,6) void playbackRequiresUserGestureChanged();
    Q_REVISION(1,6) void webRTCPublicInterfacesOnlyChanged();
    Q_REVISION(1,6) void javascriptCanPasteChanged();
    Q_REVISION(1,7) void dnsPrefetchEnabledChanged();
    Q_REVISION(1,8) void pdfViewerEnabledChanged();
    Q_REVISION(6,4) void navigateOnDropEnabledChanged();
    Q_REVISION(6,6) void readingFromCanvasEnabledChanged();

private:
    explicit QQuickWebEngineSettings(QQuickWebEngineSettings *parentSettings = nullptr);
    Q_DISABLE_COPY(QQuickWebEngineSettings)
    friend class QQuickWebEngineProfilePrivate;
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickWebEngineView;
    void setParentSettings(QQuickWebEngineSettings *parentSettings);

    QScopedPointer<QWebEngineSettings> d_ptr;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINESETTINGS_P_H
