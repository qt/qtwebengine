// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINEVIEW_P_P_H
#define QQUICKWEBENGINEVIEW_P_P_H

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

#include "qquickwebenginetouchhandle_p.h"
#include "qquickwebengineview_p.h"
#include "render_view_context_menu_qt.h"
#include "touch_handle_drawable_client.h"
#include "ui_delegates_manager.h"
#include "web_contents_adapter_client.h"

#include <QtCore/qcompilerdetection.h>
#include <QtCore/qpointer.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qstring.h>

namespace QtWebEngineCore {
class RenderWidgetHostViewQtDelegateItem;
class TouchSelectionMenuController;
class UIDelegatesManager;
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQuickWebEngineFaviconProvider;
class QQuickWebEngineScriptCollection;
class QQuickWebEngineSettings;
class QQuickWebEngineView;
class QWebEngineContextMenuRequest;
class QWebEngineFindTextResult;
class QWebEngineHistory;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineViewPrivate : public QtWebEngineCore::WebContentsAdapterClient
{
public:
    Q_DECLARE_PUBLIC(QQuickWebEngineView)
    QQuickWebEngineView *q_ptr;
    QQuickWebEngineViewPrivate();
    ~QQuickWebEngineViewPrivate();
    void releaseProfile() override;
    void initializeProfile();
    QtWebEngineCore::UIDelegatesManager *ui();

    QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override;
    QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegateForPopup(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override;
    void initializationFinished() override;
    void lifecycleStateChanged(LifecycleState state) override;
    void recommendedStateChanged(LifecycleState state) override;
    void visibleChanged(bool visible) override;
    void titleChanged(const QString&) override;
    void urlChanged() override;
    void iconChanged(const QUrl&) override;
    void loadProgressChanged(int progress) override;
    void didUpdateTargetURL(const QUrl&) override;
    void selectionChanged() override;
    void zoomUpdateIsNeeded() override;
    void recentlyAudibleChanged(bool recentlyAudible) override;
    void renderProcessPidChanged(qint64 pid) override;
    QRectF viewportRect() const override;
    QColor backgroundColor() const override;
    void loadStarted(QWebEngineLoadingInfo info) override;
    void loadCommitted() override;
    void loadFinished(QWebEngineLoadingInfo info) override;
    void focusContainer() override;
    void unhandledKeyEvent(QKeyEvent *event) override;
    QSharedPointer<QtWebEngineCore::WebContentsAdapter>
    adoptNewWindow(QSharedPointer<QtWebEngineCore::WebContentsAdapter> newWebContents,
                   WindowOpenDisposition disposition, bool userGesture, const QRect &,
                   const QUrl &targetUrl) override;
    bool isBeingAdopted() override;
    void close() override;
    void windowCloseRejected() override;
    void requestFullScreenMode(const QUrl &origin, bool fullscreen) override;
    bool isFullScreenMode() const override;
    void contextMenuRequested(QWebEngineContextMenuRequest *request) override;
    void navigationRequested(int navigationType, const QUrl &url, bool &accepted, bool isMainFrame) override;
    void javascriptDialog(QSharedPointer<QtWebEngineCore::JavaScriptDialogController>) override;
    void runFileChooser(QSharedPointer<QtWebEngineCore::FilePickerController>) override;
    void showColorDialog(QSharedPointer<QtWebEngineCore::ColorChooserController>) override;
    void didRunJavaScript(quint64, const QVariant&) override;
    void didFetchDocumentMarkup(quint64, const QString&) override { }
    void didFetchDocumentInnerText(quint64, const QString&) override { }
    void didPrintPage(quint64 requestId, QSharedPointer<QByteArray>) override;
    void didPrintPageToPdf(const QString &filePath, bool success) override;
    bool passOnFocus(bool reverse) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) override;
    void authenticationRequired(QSharedPointer<QtWebEngineCore::AuthenticationDialogController>) override;
    void runMediaAccessPermissionRequest(const QUrl &securityOrigin, MediaRequestFlags requestFlags) override;
    void runMouseLockPermissionRequest(const QUrl &securityOrigin) override;
    void runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest) override;
    void runFileSystemAccessRequest(QWebEngineFileSystemAccessRequest) override;
    QObject *accessibilityParentObject() override;
    QWebEngineSettings *webEngineSettings() const override;
    void allowCertificateError(const QWebEngineCertificateError &error) override;
    void selectClientCert(const QSharedPointer<QtWebEngineCore::ClientCertSelectController>
                                  &selectController) override;
    void runFeaturePermissionRequest(QtWebEngineCore::ProfileAdapter::PermissionType permission, const QUrl &securityOrigin) override;
    void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode) override;
    void requestGeometryChange(const QRect &geometry, const QRect &frameGeometry) override;
    void updateScrollPosition(const QPointF &position) override;
    void updateContentsSize(const QSizeF &size) override;
    void updateNavigationActions() override;
    void updateEditActions() override;
    QObject *dragSource() const override;
    bool isEnabled() const override;
    void setToolTip(const QString &toolTipText) override;
    QtWebEngineCore::TouchHandleDrawableDelegate *createTouchHandleDelegate(const QMap<int, QImage> &images) override;
    void showTouchSelectionMenu(QtWebEngineCore::TouchSelectionMenuController *, const QRect &, const QSize &) override;
    void hideTouchSelectionMenu() override;
    const QObject *holdingQObject() const override;
    ClientType clientType() override { return QtWebEngineCore::WebContentsAdapterClient::QmlClient; }

    QtWebEngineCore::ProfileAdapter *profileAdapter() override;
    QtWebEngineCore::WebContentsAdapter *webContentsAdapter() override;
    void printRequested() override;
    void findTextFinished(const QWebEngineFindTextResult &result) override;
    void showAutofillPopup(QtWebEngineCore::AutofillPopupController *controller,
                           const QRect &bounds, bool autoselectFirstSuggestion) override;
    void hideAutofillPopup() override;

    void updateAction(QQuickWebEngineView::WebAction) const;
    bool adoptWebContents(QtWebEngineCore::WebContentsAdapter *webContents);
    void setProfile(QQuickWebEngineProfile *profile);
    void updateAdapter();
    void ensureContentsAdapter();
    void setFullScreenMode(bool);

    static void bindViewAndDelegateItem(QQuickWebEngineViewPrivate *viewPrivate, QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *delegateItem);
    void delegateItemChanged(QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *oldDelegateItem,
                             QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *newDelegateItem);

    QQuickWebEngineProfile *m_profile;
    QSharedPointer<QtWebEngineCore::WebContentsAdapter> adapter;
    QScopedPointer<QWebEngineHistory> m_history;
    QScopedPointer<QQuickWebEngineSettings> m_settings;
    QQmlComponent *contextMenuExtraItems;
    QUrl m_url;
    QString m_html;
    QUrl iconUrl;
    int loadProgress;
    bool m_fullscreenMode;
    bool isLoading;
    bool m_activeFocusOnPress;
    QMap<quint64, QJSValue> m_callbacks;
    QQmlWebChannel *m_webChannel;
    QPointer<QQuickWebEngineView> inspectedView;
    QPointer<QQuickWebEngineView> devToolsView;
    uint m_webChannelWorld;
    bool m_defaultAudioMuted;
    bool m_isBeingAdopted;
    mutable QQuickWebEngineAction *actions[QQuickWebEngineView::WebActionCount];
    QtWebEngineCore::RenderWidgetHostViewQtDelegateItem *delegateItem = nullptr;

    bool profileInitialized() const;
    QQuickWebEngineScriptCollection *getUserScripts();

private:
    QScopedPointer<QtWebEngineCore::UIDelegatesManager> m_uIDelegatesManager;
    QColor m_backgroundColor;
    qreal m_zoomFactor;
    bool m_profileInitialized;
    QWebEngineContextMenuRequest *m_contextMenuRequest;
    QScopedPointer<QQuickWebEngineScriptCollection> m_scriptCollection;
    QPointer<QQuickWebEngineFaviconProvider> m_faviconProvider;
    QQmlComponent *m_touchHandleDelegate;
};

class QQuickContextMenuBuilder : public QtWebEngineCore::RenderViewContextMenuQt
{
public:
    QQuickContextMenuBuilder(QWebEngineContextMenuRequest *data, QQuickWebEngineView *view,
                             QObject *menu);
    void appendExtraItems(QQmlEngine *engine);

private:
    virtual bool hasInspector() override;
    virtual bool isFullScreenMode() override;

    virtual void addMenuItem(ContextMenuItem menuItem) override;
    virtual bool isMenuItemEnabled(ContextMenuItem menuItem) override;

    QQuickWebEngineView *m_view;
    QObject *m_menu;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEVIEW_P_P_H
