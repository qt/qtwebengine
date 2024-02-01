// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPAGE_P_H
#define QWEBENGINEPAGE_P_H

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

#include "qwebenginepage.h"

#include "qwebenginescriptcollection.h"
#include "web_contents_adapter_client.h"

#include <QtCore/qcompilerdetection.h>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtGui/QColor>

namespace QtWebEngineCore {
class AutofillPopupController;
class RenderWidgetHostViewQtDelegate;
class RenderWidgetHostViewQtDelegateClient;
class RenderWidgetHostViewQtDelegateItem;
class TouchHandleDrawableDelegate;
class TouchSelectionMenuController;
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE
class QPrinter;
class QWebEngineFindTextResult;
class QWebEngineHistory;
class QWebEnginePage;
class QWebEngineProfile;
class QWebEngineSettings;

class PageView
{
public:
    virtual void contextMenuRequested(QWebEngineContextMenuRequest *request) = 0;
    virtual QStringList chooseFiles(QWebEnginePage::FileSelectionMode mode,
                                    const QStringList &oldFiles,
                                    const QStringList &acceptedMimeTypes) = 0;
    virtual void
    showColorDialog(QSharedPointer<QtWebEngineCore::ColorChooserController> controller) = 0;
    virtual bool showAuthorizationDialog(const QString &title, const QString &message) = 0;
    virtual void javaScriptAlert(const QUrl &url, const QString &msg) = 0;
    virtual bool javaScriptConfirm(const QUrl &url, const QString &msg) = 0;
    virtual bool javaScriptPrompt(const QUrl &url, const QString &msg, const QString &defaultValue,
                                  QString *result) = 0;
    virtual void setToolTip(const QString &toolTipText) = 0;
    virtual QtWebEngineCore::RenderWidgetHostViewQtDelegate *CreateRenderWidgetHostViewQtDelegate(
            QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) = 0;
    virtual QtWebEngineCore::RenderWidgetHostViewQtDelegate *CreateRenderWidgetHostViewQtDelegateForPopup(
            QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) = 0;
    virtual QWebEngineContextMenuRequest *lastContextMenuRequest() const = 0;
    virtual QWebEnginePage *createPageForWindow(QWebEnginePage::WebWindowType type) = 0;
    virtual bool isEnabled() const = 0;
    virtual bool isVisible() const = 0;
    virtual QRect viewportRect() const = 0;
    virtual void focusContainer() = 0;
    virtual void unhandledKeyEvent(QKeyEvent *event) = 0;
    virtual bool passOnFocus(bool reverse) = 0;
    virtual QObject *accessibilityParentObject() = 0;
    virtual void didPrintPage(QPrinter *&printer, QSharedPointer<QByteArray> result) = 0;
    virtual void didPrintPageToPdf(const QString &filePath, bool success) = 0;
    virtual void printRequested() = 0;
    virtual void showAutofillPopup(QtWebEngineCore::AutofillPopupController *controller,
                                   const QRect &bounds, bool autoselectFirstSuggestion) = 0;
    virtual void hideAutofillPopup() = 0;
    virtual QtWebEngineCore::TouchHandleDrawableDelegate *
    createTouchHandleDelegate(const QMap<int, QImage> &) = 0;
    virtual void showTouchSelectionMenu(QtWebEngineCore::TouchSelectionMenuController *,
                                        const QRect &) = 0;
    virtual void hideTouchSelectionMenu() = 0;
};

class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEnginePagePrivate : public QtWebEngineCore::WebContentsAdapterClient
{
public:
    Q_DECLARE_PUBLIC(QWebEnginePage)
    QWebEnginePage *q_ptr;

    QWebEnginePagePrivate(QWebEngineProfile *profile = nullptr);
    ~QWebEnginePagePrivate();

    QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override;
    QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegateForPopup(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override;
    void initializationFinished() override;
    void lifecycleStateChanged(LifecycleState state) override;
    void recommendedStateChanged(LifecycleState state) override;
    void visibleChanged(bool visible) override;
    void titleChanged(const QString &) override;
    void urlChanged() override;
    void iconChanged(const QUrl &) override;
    void loadProgressChanged(int progress) override;
    void didUpdateTargetURL(const QUrl &) override;
    void selectionChanged() override;
    void zoomUpdateIsNeeded() override;
    void recentlyAudibleChanged(bool recentlyAudible) override;
    void renderProcessPidChanged(qint64 pid) override;
    QRectF viewportRect() const override;
    QColor backgroundColor() const override;
    void loadStarted(QWebEngineLoadingInfo info) override;
    void loadCommitted() override { }
    void loadFinished(QWebEngineLoadingInfo info) override;
    void focusContainer() override;
    void unhandledKeyEvent(QKeyEvent *event) override;
    QSharedPointer<QtWebEngineCore::WebContentsAdapter>
    adoptNewWindow(QSharedPointer<QtWebEngineCore::WebContentsAdapter> newWebContents,
                   WindowOpenDisposition disposition, bool userGesture,
                   const QRect &initialGeometry, const QUrl &targetUrl) override;
    bool isBeingAdopted() override;
    void close() override;
    void windowCloseRejected() override;
    void contextMenuRequested(QWebEngineContextMenuRequest *request) override;
    void navigationRequested(int navigationType, const QUrl &url, bool &accepted, bool isMainFrame) override;
    void requestFullScreenMode(const QUrl &origin, bool fullscreen) override;
    bool isFullScreenMode() const override;
    void javascriptDialog(QSharedPointer<QtWebEngineCore::JavaScriptDialogController>) override;
    void runFileChooser(QSharedPointer<QtWebEngineCore::FilePickerController>) override;
    void showColorDialog(QSharedPointer<QtWebEngineCore::ColorChooserController>) override;
    void didRunJavaScript(quint64 requestId, const QVariant &result) override;
    void didFetchDocumentMarkup(quint64 requestId, const QString &result) override;
    void didFetchDocumentInnerText(quint64 requestId, const QString &result) override;
    void didPrintPage(quint64 requestId, QSharedPointer<QByteArray> result) override;
    void didPrintPageToPdf(const QString &filePath, bool success) override;
    bool passOnFocus(bool reverse) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                  int lineNumber, const QString &sourceID) override;
    void authenticationRequired(
            QSharedPointer<QtWebEngineCore::AuthenticationDialogController>) override;
    void releaseProfile() override;
    void runMediaAccessPermissionRequest(const QUrl &securityOrigin, MediaRequestFlags requestFlags) override;
    void runFeaturePermissionRequest(QtWebEngineCore::ProfileAdapter::PermissionType permission, const QUrl &securityOrigin) override;
    void runMouseLockPermissionRequest(const QUrl &securityOrigin) override;
    void runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest) override;
    void runFileSystemAccessRequest(QWebEngineFileSystemAccessRequest) override;
    QObject *accessibilityParentObject() override;
    QWebEngineSettings *webEngineSettings() const override;
    void allowCertificateError(const QWebEngineCertificateError &error) override;
    void selectClientCert(
            const QSharedPointer<QtWebEngineCore::ClientCertSelectController> &controller) override;
    void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode) override;
    void requestGeometryChange(const QRect &geometry, const QRect &frameGeometry) override;
    void updateScrollPosition(const QPointF &position) override;
    void updateContentsSize(const QSizeF &size) override;
    void updateNavigationActions() override;
    void updateEditActions() override;
    QObject *dragSource() const override;
    bool isEnabled() const override;
    void setToolTip(const QString &toolTipText) override;
    void printRequested() override;
    QtWebEngineCore::TouchHandleDrawableDelegate *
    createTouchHandleDelegate(const QMap<int, QImage> &) override;
    void showTouchSelectionMenu(QtWebEngineCore::TouchSelectionMenuController *, const QRect &,
                                const QSize &) override;
    void hideTouchSelectionMenu() override;
    const QObject *holdingQObject() const override;
    ClientType clientType() override { return QtWebEngineCore::WebContentsAdapterClient::WidgetsClient; }
    void findTextFinished(const QWebEngineFindTextResult &result) override;
    void showAutofillPopup(QtWebEngineCore::AutofillPopupController *controller,
                           const QRect &bounds, bool autoselectFirstSuggestion) override;
    void hideAutofillPopup() override;

    QtWebEngineCore::ProfileAdapter *profileAdapter() override;
    QtWebEngineCore::WebContentsAdapter *webContentsAdapter() override;

    void updateAction(QWebEnginePage::WebAction) const;
    void _q_webActionTriggered(bool checked);

    void createNewWindow(WindowOpenDisposition disposition, bool userGesture, const QUrl &targetUrl);
    bool adoptWebContents(QtWebEngineCore::WebContentsAdapter *webContents);
    QtWebEngineCore::WebContentsAdapter *webContents() { return adapter.data(); }
    void recreateFromSerializedHistory(QDataStream &input);

    void setFullScreenMode(bool);
    void ensureInitialized() const;

    static QString actionText(int action);

    QSharedPointer<QtWebEngineCore::WebContentsAdapter> adapter;
    QWebEngineHistory *history;
    QWebEngineProfile *profile;
    QWebEngineSettings *settings;
    PageView *view;
    QUrl url;
    bool isLoading;
    QWebEngineScriptCollection scriptCollection;
    bool m_isBeingAdopted;
    QColor m_backgroundColor;
    bool fullscreenMode;
    QWebChannel *webChannel;
    unsigned int webChannelWorldId;
    QUrl iconUrl;
    QPointer<QWebEnginePage> inspectedPage;
    QPointer<QWebEnginePage> devToolsPage;
    bool defaultAudioMuted;
    qreal defaultZoomFactor;
    QTimer wasShownTimer;
    QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *delegateItem = nullptr;
#if QT_CONFIG(webengine_printing_and_pdf)
    QPrinter *currentPrinter = nullptr;
#endif

    mutable QMap<quint64, std::function<void(const QVariant &)>> m_variantCallbacks;
    mutable QMap<quint64, std::function<void(const QString &)>> m_stringCallbacks;
    QMap<quint64, std::function<void(const QByteArray &)>> m_pdfResultCallbacks;
    mutable QAction *actions[QWebEnginePage::WebActionCount];
};

QT_END_NAMESPACE

#endif // QWEBENGINEPAGE_P_H
