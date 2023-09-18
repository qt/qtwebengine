// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEVIEW_P_H
#define QQUICKWEBENGINEVIEW_P_H

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

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtWebEngineCore/qwebenginequotarequest.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>
#include <QtGui/qcolor.h>
#include <QtQml/qqmlregistration.h>
#include <QtQuick/qquickitem.h>

QT_BEGIN_NAMESPACE

class QQmlWebChannel;
class QQmlComponent;
class QQuickContextMenuBuilder;
class QQuickWebEngineAction;
class QQuickWebEngineAuthenticationDialogRequest;
class QQuickWebEngineClientCertificateSelection;
class QQuickWebEngineColorDialogRequest;
class QQuickWebEngineFileDialogRequest;
class QQuickWebEngineJavaScriptDialogRequest;
class QQuickWebEngineNewWindowRequest;
class QQuickWebEngineProfile;
class QQuickWebEngineSettings;
class QQuickWebEngineTooltipRequest;
class QQuickWebEngineViewPrivate;
class QWebEngineCertificateError;
class QWebEngineContextMenuRequest;
class QWebEngineFileSystemAccessRequest;
class QWebEngineFindTextResult;
class QWebEngineFullScreenRequest;
class QWebEngineHistory;
class QWebEngineLoadingInfo;
class QWebEngineNavigationRequest;
class QWebEngineNewWindowRequest;
class QWebEngineRegisterProtocolHandlerRequest;
class QQuickWebEngineScriptCollection;
class QQuickWebEngineTouchSelectionMenuRequest;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineView : public QQuickItem {
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged FINAL)
    Q_PROPERTY(QUrl icon READ icon NOTIFY iconChanged FINAL)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged FINAL)
    Q_PROPERTY(int loadProgress READ loadProgress NOTIFY loadProgressChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY canGoBackChanged FINAL)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY canGoForwardChanged FINAL)
    Q_PROPERTY(bool isFullScreen READ isFullScreen NOTIFY isFullScreenChanged REVISION(1,1) FINAL)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomFactorChanged REVISION(1,1) FINAL)
    Q_PROPERTY(QQuickWebEngineProfile *profile READ profile WRITE setProfile NOTIFY profileChanged FINAL REVISION(1,1))
    Q_PROPERTY(QQuickWebEngineSettings *settings READ settings REVISION(1,1) CONSTANT FINAL)
    Q_PROPERTY(QWebEngineHistory *history READ history CONSTANT FINAL REVISION(1,1))
#if QT_CONFIG(webengine_webchannel)
    Q_PROPERTY(QQmlWebChannel *webChannel READ webChannel WRITE setWebChannel NOTIFY webChannelChanged REVISION(1,1) FINAL)
#endif
    Q_PROPERTY(QQuickWebEngineScriptCollection *userScripts READ userScripts FINAL REVISION(1,1))
    Q_PROPERTY(bool activeFocusOnPress READ activeFocusOnPress WRITE setActiveFocusOnPress NOTIFY activeFocusOnPressChanged REVISION(1,2) FINAL)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged REVISION(1,2) FINAL)
    Q_PROPERTY(QSizeF contentsSize READ contentsSize NOTIFY contentsSizeChanged FINAL REVISION(1,3))
    Q_PROPERTY(QPointF scrollPosition READ scrollPosition NOTIFY scrollPositionChanged FINAL REVISION(1,3))
    Q_PROPERTY(bool audioMuted READ isAudioMuted WRITE setAudioMuted NOTIFY audioMutedChanged FINAL REVISION(1,3))
    Q_PROPERTY(bool recentlyAudible READ recentlyAudible NOTIFY recentlyAudibleChanged FINAL REVISION(1,3))
    Q_PROPERTY(uint webChannelWorld READ webChannelWorld WRITE setWebChannelWorld NOTIFY webChannelWorldChanged REVISION(1,3) FINAL)

    Q_PROPERTY(QQuickWebEngineView *inspectedView READ inspectedView WRITE setInspectedView NOTIFY inspectedViewChanged REVISION(1,7) FINAL)
    Q_PROPERTY(QQuickWebEngineView *devToolsView READ devToolsView WRITE setDevToolsView NOTIFY devToolsViewChanged REVISION(1,7) FINAL)
    Q_PROPERTY(QString devToolsId READ devToolsId CONSTANT REVISION(6,6) FINAL)

    Q_PROPERTY(LifecycleState lifecycleState READ lifecycleState WRITE setLifecycleState NOTIFY lifecycleStateChanged REVISION(1,10) FINAL)
    Q_PROPERTY(LifecycleState recommendedState READ recommendedState NOTIFY recommendedStateChanged REVISION(1,10) FINAL)

    Q_PROPERTY(qint64 renderProcessPid READ renderProcessPid NOTIFY renderProcessPidChanged FINAL REVISION(1,11))
    Q_PROPERTY(QQmlComponent *touchHandleDelegate READ touchHandleDelegate WRITE
                       setTouchHandleDelegate NOTIFY touchHandleDelegateChanged REVISION(0) FINAL)
    QML_NAMED_ELEMENT(WebEngineView)
    QML_ADDED_IN_VERSION(1, 0)
    QML_EXTRA_VERSION(2, 0)

public:
    QQuickWebEngineView(QQuickItem *parent = nullptr);
    ~QQuickWebEngineView();

    QUrl url() const;
    void setUrl(const QUrl&);
    QUrl icon() const;
    bool isLoading() const;
    int loadProgress() const;
    QString title() const;
    bool canGoBack() const;
    bool canGoForward() const;
    bool isFullScreen() const;
    qreal zoomFactor() const;
    void setZoomFactor(qreal arg);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);
    QSizeF contentsSize() const;
    QPointF scrollPosition() const;

#if QT_DEPRECATED_SINCE(6, 2)
QT_WARNING_PUSH QT_WARNING_DISABLE_DEPRECATED
    enum QT_DEPRECATED NavigationRequestAction {
        AcceptRequest,
        // Make room in the valid range of the enum so
        // we can expose extra actions.
        IgnoreRequest = 0xFF
    };
    Q_ENUM(NavigationRequestAction)

    enum QT_DEPRECATED NavigationType {
        LinkClickedNavigation,
        TypedNavigation,
        FormSubmittedNavigation,
        BackForwardNavigation,
        ReloadNavigation,
        OtherNavigation,
        RedirectNavigation,
    };
    Q_ENUM(NavigationType)

    enum QT_DEPRECATED LoadStatus {
        LoadStartedStatus,
        LoadStoppedStatus,
        LoadSucceededStatus,
        LoadFailedStatus
    };
    Q_ENUM(LoadStatus)

    enum QT_DEPRECATED ErrorDomain {
         NoErrorDomain,
         InternalErrorDomain,
         ConnectionErrorDomain,
         CertificateErrorDomain,
         HttpErrorDomain,
         FtpErrorDomain,
         DnsErrorDomain
    };
    Q_ENUM(ErrorDomain)

    enum QT_DEPRECATED NewViewDestination {
        NewViewInWindow,
        NewViewInTab,
        NewViewInDialog,
        NewViewInBackgroundTab
    };
    Q_ENUM(NewViewDestination)
QT_WARNING_POP
#endif

    enum Feature {
        MediaAudioCapture,
        MediaVideoCapture,
        MediaAudioVideoCapture,
        Geolocation,
        DesktopVideoCapture,
        DesktopAudioVideoCapture,
        Notifications,
    };
    Q_ENUM(Feature)

    enum WebAction {
        NoWebAction = - 1,
        Back,
        Forward,
        Stop,
        Reload,

        Cut,
        Copy,
        Paste,

        Undo,
        Redo,
        SelectAll,
        ReloadAndBypassCache,

        PasteAndMatchStyle,

        OpenLinkInThisWindow,
        OpenLinkInNewWindow,
        OpenLinkInNewTab,
        CopyLinkToClipboard,
        DownloadLinkToDisk,

        CopyImageToClipboard,
        CopyImageUrlToClipboard,
        DownloadImageToDisk,

        CopyMediaUrlToClipboard,
        ToggleMediaControls,
        ToggleMediaLoop,
        ToggleMediaPlayPause,
        ToggleMediaMute,
        DownloadMediaToDisk,

        InspectElement,
        ExitFullScreen,
        RequestClose,
        Unselect,
        SavePage,
        OpenLinkInNewBackgroundTab, // Not supported in QML
        ViewSource,

        ToggleBold,
        ToggleItalic,
        ToggleUnderline,
        ToggleStrikethrough,

        AlignLeft,
        AlignCenter,
        AlignRight,
        AlignJustified,
        Indent,
        Outdent,

        InsertOrderedList,
        InsertUnorderedList,

        ChangeTextDirectionLTR,
        ChangeTextDirectionRTL,

        WebActionCount
    };
    Q_ENUM(WebAction)

    // must match WebContentsAdapterClient::JavaScriptConsoleMessageLevel
    enum JavaScriptConsoleMessageLevel {
        InfoMessageLevel = 0,
        WarningMessageLevel,
        ErrorMessageLevel
    };
    Q_ENUM(JavaScriptConsoleMessageLevel)

    // must match WebContentsAdapterClient::RenderProcessTerminationStatus
    enum RenderProcessTerminationStatus {
        NormalTerminationStatus = 0,
        AbnormalTerminationStatus,
        CrashedTerminationStatus,
        KilledTerminationStatus
    };
    Q_ENUM(RenderProcessTerminationStatus)

    enum FindFlag {
        FindBackward = 1,
        FindCaseSensitively = 2,
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)
    Q_FLAG(FindFlags)

    // must match QPageSize::PageSizeId
    enum PrintedPageSizeId {
        // Existing Qt sizes
        A4,
        B5,
        Letter,
        Legal,
        Executive,
        A0,
        A1,
        A2,
        A3,
        A5,
        A6,
        A7,
        A8,
        A9,
        B0,
        B1,
        B10,
        B2,
        B3,
        B4,
        B6,
        B7,
        B8,
        B9,
        C5E,
        Comm10E,
        DLE,
        Folio,
        Ledger,
        Tabloid,
        Custom,

        // New values derived from PPD standard
        A10,
        A3Extra,
        A4Extra,
        A4Plus,
        A4Small,
        A5Extra,
        B5Extra,

        JisB0,
        JisB1,
        JisB2,
        JisB3,
        JisB4,
        JisB5,
        JisB6,
        JisB7,
        JisB8,
        JisB9,
        JisB10,

        // AnsiA = Letter,
        // AnsiB = Ledger,
        AnsiC,
        AnsiD,
        AnsiE,
        LegalExtra,
        LetterExtra,
        LetterPlus,
        LetterSmall,
        TabloidExtra,

        ArchA,
        ArchB,
        ArchC,
        ArchD,
        ArchE,

        Imperial7x9,
        Imperial8x10,
        Imperial9x11,
        Imperial9x12,
        Imperial10x11,
        Imperial10x13,
        Imperial10x14,
        Imperial12x11,
        Imperial15x11,

        ExecutiveStandard,
        Note,
        Quarto,
        Statement,
        SuperA,
        SuperB,
        Postcard,
        DoublePostcard,
        Prc16K,
        Prc32K,
        Prc32KBig,

        FanFoldUS,
        FanFoldGerman,
        FanFoldGermanLegal,

        EnvelopeB4,
        EnvelopeB5,
        EnvelopeB6,
        EnvelopeC0,
        EnvelopeC1,
        EnvelopeC2,
        EnvelopeC3,
        EnvelopeC4,
        // EnvelopeC5 = C5E,
        EnvelopeC6,
        EnvelopeC65,
        EnvelopeC7,
        // EnvelopeDL = DLE,

        Envelope9,
        // Envelope10 = Comm10E,
        Envelope11,
        Envelope12,
        Envelope14,
        EnvelopeMonarch,
        EnvelopePersonal,

        EnvelopeChou3,
        EnvelopeChou4,
        EnvelopeInvite,
        EnvelopeItalian,
        EnvelopeKaku2,
        EnvelopeKaku3,
        EnvelopePrc1,
        EnvelopePrc2,
        EnvelopePrc3,
        EnvelopePrc4,
        EnvelopePrc5,
        EnvelopePrc6,
        EnvelopePrc7,
        EnvelopePrc8,
        EnvelopePrc9,
        EnvelopePrc10,
        EnvelopeYou4,

        // Last item, with commonly used synynoms from QPagedPrintEngine / QPrinter
        LastPageSize = EnvelopeYou4,
        NPageSize = LastPageSize,
        NPaperSize = LastPageSize,

        // Convenience overloads for naming consistency
        AnsiA = Letter,
        AnsiB = Ledger,
        EnvelopeC5 = C5E,
        EnvelopeDL = DLE,
        Envelope10 = Comm10E
    };
    Q_ENUM(PrintedPageSizeId)

    // must match QPageLayout::Orientation
    enum PrintedPageOrientation {
        Portrait,
        Landscape
    };
    Q_ENUM(PrintedPageOrientation)

    // must match WebContentsAdapterClient::LifecycleState
    enum class LifecycleState {
        Active,
        Frozen,
        Discarded,
    };
    Q_ENUM(LifecycleState)

    // QmlParserStatus
    void componentComplete() override;

    QQuickWebEngineProfile *profile();
    void setProfile(QQuickWebEngineProfile *);
    QQuickWebEngineScriptCollection *userScripts();

    QQuickWebEngineSettings *settings();
    QQmlWebChannel *webChannel();
    void setWebChannel(QQmlWebChannel *);
    QWebEngineHistory *history() const;
    uint webChannelWorld() const;
    void setWebChannelWorld(uint);
    Q_REVISION(1,8) Q_INVOKABLE QQuickWebEngineAction *action(WebAction action);

    Q_INVOKABLE void acceptAsNewWindow(QWebEngineNewWindowRequest *request);

    bool isAudioMuted() const;
    void setAudioMuted(bool muted);
    bool recentlyAudible() const;

    qint64 renderProcessPid() const;

    bool activeFocusOnPress() const;

    void setInspectedView(QQuickWebEngineView *);
    QQuickWebEngineView *inspectedView() const;
    void setDevToolsView(QQuickWebEngineView *);
    QQuickWebEngineView *devToolsView() const;
    QString devToolsId();

    LifecycleState lifecycleState() const;
    void setLifecycleState(LifecycleState state);

    LifecycleState recommendedState() const;

    QQmlComponent *touchHandleDelegate() const;
    void setTouchHandleDelegate(QQmlComponent *delegagte);

public Q_SLOTS:
    void runJavaScript(const QString&, const QJSValue & = QJSValue());
    Q_REVISION(1,3) void runJavaScript(const QString&, quint32 worldId, const QJSValue & = QJSValue());
    void loadHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void goBack();
    void goForward();
    Q_REVISION(1,1) void goBackOrForward(int index);
    void reload();
    Q_REVISION(1,1) void reloadAndBypassCache();
    void stop();
    Q_REVISION(1,1) void findText(const QString &subString, FindFlags options = { }, const QJSValue &callback = QJSValue());
    Q_REVISION(1,1) void fullScreenCancelled();
    Q_REVISION(1,1) void grantFeaturePermission(const QUrl &securityOrigin, QQuickWebEngineView::Feature, bool granted);
    Q_REVISION(1,2) void setActiveFocusOnPress(bool arg);
    Q_REVISION(1,2) void triggerWebAction(WebAction action);
    Q_REVISION(1,3) void printToPdf(const QString &filePath, PrintedPageSizeId pageSizeId = PrintedPageSizeId::A4, PrintedPageOrientation orientation = PrintedPageOrientation::Portrait);
    Q_REVISION(1,3) void printToPdf(const QJSValue &callback, PrintedPageSizeId pageSizeId = PrintedPageSizeId::A4, PrintedPageOrientation orientation = PrintedPageOrientation::Portrait);
    Q_REVISION(1,4) void replaceMisspelledWord(const QString &replacement);
    Q_REVISION(6, 6) void save(const QString &filePath,
                               QWebEngineDownloadRequest::SavePageFormat format =
                                       QWebEngineDownloadRequest::MimeHtmlSaveFormat) const;

private Q_SLOTS:
    void lazyInitialize();

Q_SIGNALS:
    void titleChanged();
    void urlChanged();
    void iconChanged();
    void loadingChanged(const QWebEngineLoadingInfo &loadingInfo);
    void loadProgressChanged();
    void linkHovered(const QUrl &hoveredUrl);
    void navigationRequested(QWebEngineNavigationRequest *request);
    void javaScriptConsoleMessage(QQuickWebEngineView::JavaScriptConsoleMessageLevel level,
                                  const QString &message, int lineNumber, const QString &sourceID);
    Q_REVISION(1,1) void certificateError(const QWebEngineCertificateError &error);
    Q_REVISION(1,1) void fullScreenRequested(const QWebEngineFullScreenRequest &request);
    Q_REVISION(1,1) void isFullScreenChanged();
    Q_REVISION(1, 1)
    void featurePermissionRequested(const QUrl &securityOrigin,
                                    QQuickWebEngineView::Feature feature);
    Q_REVISION(1,1) void zoomFactorChanged(qreal arg);
    Q_REVISION(1,1) void profileChanged();
    Q_REVISION(1,1) void webChannelChanged();
    Q_REVISION(1,2) void activeFocusOnPressChanged(bool);
    Q_REVISION(1,2) void backgroundColorChanged();
    Q_REVISION(1, 2)
    void renderProcessTerminated(QQuickWebEngineView::RenderProcessTerminationStatus terminationStatus,
                            int exitCode);
    Q_REVISION(1,2) void windowCloseRequested();
    Q_REVISION(1,3) void contentsSizeChanged(const QSizeF& size);
    Q_REVISION(1,3) void scrollPositionChanged(const QPointF& position);
    Q_REVISION(1,3) void audioMutedChanged(bool muted);
    Q_REVISION(1,3) void recentlyAudibleChanged(bool recentlyAudible);
    Q_REVISION(1,3) void webChannelWorldChanged(uint);
    Q_REVISION(1,4) void contextMenuRequested(QWebEngineContextMenuRequest *request);
    Q_REVISION(1,4) void authenticationDialogRequested(QQuickWebEngineAuthenticationDialogRequest *request);
    Q_REVISION(1,4) void javaScriptDialogRequested(QQuickWebEngineJavaScriptDialogRequest *request);
    Q_REVISION(1,4) void colorDialogRequested(QQuickWebEngineColorDialogRequest *request);
    Q_REVISION(1,4) void fileDialogRequested(QQuickWebEngineFileDialogRequest *request);
    Q_REVISION(1,5) void pdfPrintingFinished(const QString &filePath, bool success);
#if QT_DEPRECATED_SINCE(6, 5)
    QT_DEPRECATED_VERSION_X_6_5("Requesting host quota is no longer supported.")
    Q_REVISION(1, 7) void quotaRequested(const QWebEngineQuotaRequest &request);
#endif
    Q_REVISION(1,7) void geometryChangeRequested(const QRect &geometry, const QRect &frameGeometry);
    Q_REVISION(1,7) void inspectedViewChanged();
    Q_REVISION(1,7) void devToolsViewChanged();
    Q_REVISION(1,7) void registerProtocolHandlerRequested(const QWebEngineRegisterProtocolHandlerRequest &request);
    Q_REVISION(1,8) void printRequested();
    Q_REVISION(1,9) void selectClientCertificate(QQuickWebEngineClientCertificateSelection *clientCertSelection);
    Q_REVISION(1,10) void tooltipRequested(QQuickWebEngineTooltipRequest *request);
    Q_REVISION(1, 10) void lifecycleStateChanged(QQuickWebEngineView::LifecycleState state);
    Q_REVISION(1, 10) void recommendedStateChanged(QQuickWebEngineView::LifecycleState state);
    Q_REVISION(1,10) void findTextFinished(const QWebEngineFindTextResult &result);
    Q_REVISION(1,11) void renderProcessPidChanged(qint64 pid);
    Q_REVISION(1,11) void canGoBackChanged();
    Q_REVISION(1,11) void canGoForwardChanged();
    Q_REVISION(1,12) void newWindowRequested(QQuickWebEngineNewWindowRequest *request);
    Q_REVISION(6,3) void touchSelectionMenuRequested(QQuickWebEngineTouchSelectionMenuRequest *request);
    Q_REVISION(6,4) void touchHandleDelegateChanged();
    Q_REVISION(6,4) void fileSystemAccessRequested(const QWebEngineFileSystemAccessRequest &request);

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void itemChange(ItemChange, const ItemChangeData &) override;
#if QT_CONFIG(draganddrop)
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
#endif // QT_CONFIG(draganddrop)

private:
    Q_DECLARE_PRIVATE(QQuickWebEngineView)
    QScopedPointer<QQuickWebEngineViewPrivate> d_ptr;

    friend class QQuickContextMenuBuilder;
    friend class FaviconImageResponse;
    friend class FaviconImageResponseRunnable;
#if QT_CONFIG(accessibility)
    friend class QQuickWebEngineViewAccessible;
#endif // QT_CONFIG(accessibility)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineView)

#endif // QQUICKWEBENGINEVIEW_P_H
