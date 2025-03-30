// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPAGE_H
#define QWEBENGINEPAGE_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtWebEngineCore/qwebengineclientcertificateselection.h>
#include <QtWebEngineCore/qwebenginedownloadrequest.h>
#include <QtWebEngineCore/qwebenginequotarequest.h>
#include <QtWebEngineCore/qwebengineframe.h>
#include <QtWebEngineCore/qwebenginepermission.h>

#include <QtCore/qanystringview.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtGui/qpagelayout.h>
#include <QtGui/qpageranges.h>
#include <QtGui/qtgui-config.h>

#include <functional>
#include <optional>

QT_BEGIN_NAMESPACE

class QAction;
class QAuthenticator;
class QContextMenuBuilder;
class QRect;
class QVariant;
class QWebChannel;
class QWebEngineCertificateError;
class QWebEngineDesktopMediaRequest;
class QWebEngineFileSystemAccessRequest;
class QWebEngineFindTextResult;
class QWebEngineFullScreenRequest;
class QWebEngineHistory;
class QWebEngineHttpRequest;
class QWebEngineLoadingInfo;
class QWebEngineNavigationRequest;
class QWebEngineNewWindowRequest;
class QWebEnginePagePrivate;
class QWebEngineProfile;
class QWebEngineRegisterProtocolHandlerRequest;
class QWebEngineScriptCollection;
class QWebEngineSettings;
class QWebEngineUrlRequestInterceptor;
class QWebEngineWebAuthUxRequest;

class Q_WEBENGINECORE_EXPORT QWebEnginePage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(bool hasSelection READ hasSelection)
    Q_PROPERTY(QUrl requestedUrl READ requestedUrl)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomFactorChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QUrl iconUrl READ iconUrl NOTIFY iconUrlChanged)
    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QSizeF contentsSize READ contentsSize NOTIFY contentsSizeChanged)
    Q_PROPERTY(QPointF scrollPosition READ scrollPosition NOTIFY scrollPositionChanged)
    Q_PROPERTY(bool audioMuted READ isAudioMuted WRITE setAudioMuted NOTIFY audioMutedChanged)
    Q_PROPERTY(bool recentlyAudible READ recentlyAudible NOTIFY recentlyAudibleChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(LifecycleState lifecycleState READ lifecycleState WRITE setLifecycleState NOTIFY lifecycleStateChanged)
    Q_PROPERTY(LifecycleState recommendedState READ recommendedState NOTIFY recommendedStateChanged)
    Q_PROPERTY(qint64 renderProcessPid READ renderProcessPid NOTIFY renderProcessPidChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged FINAL)

public:
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
        OpenLinkInNewBackgroundTab,
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

    enum FindFlag {
        FindBackward = 1,
        FindCaseSensitively = 2,
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    enum WebWindowType {
        WebBrowserWindow,
        WebBrowserTab,
        WebDialog,
        WebBrowserBackgroundTab
    };
    Q_ENUM(WebWindowType)

#if QT_DEPRECATED_SINCE(6, 8)
    enum PermissionPolicy {
        PermissionUnknown Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::State::Ask instead"),
        PermissionGrantedByUser Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::State::Granted instead"),
        PermissionDeniedByUser Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::State::Denied instead")
    };
    Q_ENUM(PermissionPolicy)
#endif

    // must match WebContentsAdapterClient::NavigationType
    enum NavigationType {
        NavigationTypeLinkClicked,
        NavigationTypeTyped,
        NavigationTypeFormSubmitted,
        NavigationTypeBackForward,
        NavigationTypeReload,
        NavigationTypeOther,
        NavigationTypeRedirect,
    };
    Q_ENUM(NavigationType)

#if QT_DEPRECATED_SINCE(6, 8)
    enum Feature {
        Notifications Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::Notifications instead") = 0,
        Geolocation Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::Geolocation instead") = 1,
        MediaAudioCapture Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::MediaAudioCapture instead") = 2,
        MediaVideoCapture Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::MediaVideoCapture instead"),
        MediaAudioVideoCapture Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::MediaAudioVideoCapture instead"),
        MouseLock Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::MouseLock instead"),
        DesktopVideoCapture Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::DesktopVideoCapture instead"),
        DesktopAudioVideoCapture Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::DesktopAudioVideoCapture instead"),
        ClipboardReadWrite Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::ClipboardReadWrite instead"),
        LocalFontsAccess Q_DECL_ENUMERATOR_DEPRECATED_X(
            "Use QWebEnginePermission::PermissionType::LocalFontsAccess instead"),
    };
    Q_ENUM(Feature)
#endif

    // Ex-QWebFrame enum

    enum FileSelectionMode {
        FileSelectOpen,
        FileSelectOpenMultiple,
        FileSelectUploadFolder,
        FileSelectSave
    };
    Q_ENUM(FileSelectionMode)

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

    // must match WebContentsAdapterClient::LifecycleState
    enum class LifecycleState {
        Active,
        Frozen,
        Discarded,
    };
    Q_ENUM(LifecycleState)

    explicit QWebEnginePage(QObject *parent = nullptr);
    QWebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);
    ~QWebEnginePage();
    QWebEngineHistory *history() const;

    bool hasSelection() const;
    QString selectedText() const;

    QWebEngineProfile *profile() const;

#if QT_CONFIG(action)
    QAction *action(WebAction action) const;
#endif
    virtual void triggerAction(WebAction action, bool checked = false);

    void replaceMisspelledWord(const QString &replacement);

    bool event(QEvent*) override;

    void findText(const QString &subString, FindFlags options = {}, const std::function<void(const QWebEngineFindTextResult &)> &resultCallback = std::function<void(const QWebEngineFindTextResult &)>());

#if QT_DEPRECATED_SINCE(6, 8)
    QT_DEPRECATED_VERSION_X_6_8(
        "Setting permissions through QWebEnginePage has been deprecated. Please use QWebEnginePermission instead.")
    void setFeaturePermission(const QUrl &securityOrigin, Feature feature, PermissionPolicy policy);
#endif

    bool isLoading() const;

    void load(const QUrl &url);
    void load(const QWebEngineHttpRequest &request);
    void download(const QUrl &url, const QString &filename = QString());
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());

    void toHtml(const std::function<void(const QString &)> &resultCallback) const;
    void toPlainText(const std::function<void(const QString &)> &resultCallback) const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QUrl requestedUrl() const;
    QUrl iconUrl() const;
    QIcon icon() const;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    QPointF scrollPosition() const;
    QSizeF contentsSize() const;

    void runJavaScript(const QString &scriptSource, const std::function<void(const QVariant &)> &resultCallback);
    void runJavaScript(const QString &scriptSource, quint32 worldId = 0, const std::function<void(const QVariant &)> &resultCallback = {});
    QWebEngineScriptCollection &scripts();
    QWebEngineSettings *settings() const;

    QWebChannel *webChannel() const;
    void setWebChannel(QWebChannel *, quint32 worldId = 0);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    void save(const QString &filePath, QWebEngineDownloadRequest::SavePageFormat format
                                                = QWebEngineDownloadRequest::MimeHtmlSaveFormat) const;

    bool isAudioMuted() const;
    void setAudioMuted(bool muted);
    bool recentlyAudible() const;
    qint64 renderProcessPid() const;

    void printToPdf(const QString &filePath,
                    const QPageLayout &layout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF()),
                    const QPageRanges &ranges = {});
    void printToPdf(const std::function<void(const QByteArray&)> &resultCallback,
                    const QPageLayout &layout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF()),
                    const QPageRanges &ranges = {});

    void setInspectedPage(QWebEnginePage *page);
    QWebEnginePage *inspectedPage() const;
    void setDevToolsPage(QWebEnginePage *page);
    QWebEnginePage *devToolsPage() const;
    QString devToolsId() const;

    void setUrlRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);

    LifecycleState lifecycleState() const;
    void setLifecycleState(LifecycleState state);

    LifecycleState recommendedState() const;

    bool isVisible() const;
    void setVisible(bool visible);

    QWebEngineFrame mainFrame();
    std::optional<QWebEngineFrame> findFrameByName(QAnyStringView name);

    void acceptAsNewWindow(QWebEngineNewWindowRequest &request);

Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool ok);
    void loadingChanged(const QWebEngineLoadingInfo &loadingInfo);

    void linkHovered(const QString &url);
    void selectionChanged();
    void geometryChangeRequested(const QRect &geom);
    void windowCloseRequested();

#if QT_DEPRECATED_SINCE(6, 8)
    QT_MOC_COMPAT QT_DEPRECATED_VERSION_X_6_8(
        "The signal has been deprecated; please use permissionRequested instead.")
    void featurePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    QT_MOC_COMPAT QT_DEPRECATED_VERSION_X_6_8(
        "The signal has been deprecated, and no longer functions.")
    void featurePermissionRequestCanceled(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
#endif // QT_DEPRECATED_SINCE(6, 8)

    void fullScreenRequested(QWebEngineFullScreenRequest fullScreenRequest);
    void permissionRequested(QWebEnginePermission permissionRequest);

#if QT_DEPRECATED_SINCE(6, 5)
    QT_DEPRECATED_VERSION_X_6_5("Requesting host quota is no longer supported.")
    void quotaRequested(QWebEngineQuotaRequest quotaRequest);
#endif
    void registerProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request);
    void fileSystemAccessRequested(QWebEngineFileSystemAccessRequest request);
    void selectClientCertificate(QWebEngineClientCertificateSelection clientCertSelection);
    void authenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator);
    void proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost);

    void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode);
    void desktopMediaRequested(const QWebEngineDesktopMediaRequest &request);
    void certificateError(const QWebEngineCertificateError &certificateError);
    void navigationRequested(QWebEngineNavigationRequest &request);
    void newWindowRequested(QWebEngineNewWindowRequest &request);

    // Ex-QWebFrame signals
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void iconUrlChanged(const QUrl &url);
    void iconChanged(const QIcon &icon);

    void zoomFactorChanged(qreal factor);
    void scrollPositionChanged(const QPointF &position);
    void contentsSizeChanged(const QSizeF &size);
    void audioMutedChanged(bool muted);
    void recentlyAudibleChanged(bool recentlyAudible);
    void renderProcessPidChanged(qint64 pid);

    void pdfPrintingFinished(const QString &filePath, bool success);
    void printRequested();
    void printRequestedByFrame(QWebEngineFrame frame);

    void visibleChanged(bool visible);

    void lifecycleStateChanged(LifecycleState state);
    void recommendedStateChanged(LifecycleState state);

    void findTextFinished(const QWebEngineFindTextResult &result);

    // TODO: fixme / rewrite bindPageToView
    void _q_aboutToDelete();

    void webAuthUxRequested(QWebEngineWebAuthUxRequest *request);

protected:
    virtual QWebEnginePage *createWindow(WebWindowType type);
    virtual QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles,
                                    const QStringList &acceptedMimeTypes);
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString &msg);
    virtual bool javaScriptConfirm(const QUrl &securityOrigin, const QString &msg);
    virtual bool javaScriptPrompt(const QUrl &securityOrigin, const QString &msg,
                                  const QString &defaultValue, QString *result);
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                          const QString &message, int lineNumber,
                                          const QString &sourceID);
    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

private:
    Q_DISABLE_COPY(QWebEnginePage)
    Q_DECLARE_PRIVATE(QWebEnginePage)
    QScopedPointer<QWebEnginePagePrivate> d_ptr;
#if QT_CONFIG(action)
    Q_PRIVATE_SLOT(d_func(), void _q_webActionTriggered(bool checked))
#endif

    friend class QContextMenuBuilder;
    friend class QWebEngineView;
    friend class QWebEngineViewPrivate;
#if QT_CONFIG(accessibility)
    friend class QWebEngineViewAccessible;
#endif // QT_CONFIG(accessibility)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWebEnginePage::FindFlags)

Q_WEBENGINECORE_EXPORT QDataStream &operator<<(QDataStream &stream,
                                               const QWebEngineHistory &history);
Q_WEBENGINECORE_EXPORT QDataStream &operator>>(QDataStream &stream, QWebEngineHistory &history);

QT_END_NAMESPACE

#endif // QWEBENGINEPAGE_H
