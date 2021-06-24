/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef WEB_CONTENTS_ADAPTER_CLIENT_H
#define WEB_CONTENTS_ADAPTER_CLIENT_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include "profile_adapter.h"

#include <QFlags>
#include <QRect>
#include <QString>
#include <QStringList>
#include <QUrl>

QT_FORWARD_DECLARE_CLASS(QKeyEvent)
QT_FORWARD_DECLARE_CLASS(QVariant)
QT_FORWARD_DECLARE_CLASS(QWebEngineFindTextResult)
QT_FORWARD_DECLARE_CLASS(QWebEngineLoadingInfo)
QT_FORWARD_DECLARE_CLASS(QWebEngineQuotaRequest)
QT_FORWARD_DECLARE_CLASS(QWebEngineRegisterProtocolHandlerRequest)
QT_FORWARD_DECLARE_CLASS(QWebEngineUrlRequestInfo)
QT_FORWARD_DECLARE_CLASS(QWebEngineUrlRequestInterceptor)
QT_FORWARD_DECLARE_CLASS(QWebEngineContextMenuRequest)
QT_FORWARD_DECLARE_CLASS(QWebEngineCertificateError)
QT_FORWARD_DECLARE_CLASS(QWebEngineSettings)

namespace content {
struct DropData;
}

namespace QtWebEngineCore {

class CertificateErrorController;
class ClientCertSelectController;
class AuthenticationDialogController;
class ColorChooserController;
class FilePickerController;
class JavaScriptDialogController;
class RenderWidgetHostViewQt;
class RenderWidgetHostViewQtDelegate;
class RenderWidgetHostViewQtDelegateClient;
class TouchHandleDrawableClient;
class TouchSelectionMenuController;
class WebContentsAdapter;
class WebContentsDelegateQt;
class WebEngineSettings;

class Q_WEBENGINECORE_PRIVATE_EXPORT WebContentsAdapterClient {
public:
    // This must match window_open_disposition_list.h.
    enum WindowOpenDisposition {
        UnknownDisposition = 0,
        CurrentTabDisposition = 1,
        SingletonTabDisposition = 2,
        NewForegroundTabDisposition = 3,
        NewBackgroundTabDisposition = 4,
        NewPopupDisposition = 5,
        NewWindowDisposition = 6,
        SaveToDiskDisposition = 7,
        OffTheRecordDisposition = 8,
        IgnoreActionDisposition = 9,
    };

    // Must match the values in javascript_dialog_type.h.
    enum JavascriptDialogType {
        AlertDialog,
        ConfirmDialog,
        PromptDialog,
        UnloadDialog,
        // Leave room for potential new specs
        InternalAuthorizationDialog = 0x10,
    };

    enum NavigationType {
        LinkNavigation,
        TypedNavigation,
        FormSubmittedNavigation,
        BackForwardNavigation,
        ReloadNavigation,
        OtherNavigation,
        RedirectNavigation,
    };

    enum JavaScriptConsoleMessageLevel {
        Info = 0,
        Warning,
        Error
    };

    enum RenderProcessTerminationStatus {
        NormalTerminationStatus = 0,
        AbnormalTerminationStatus,
        CrashedTerminationStatus,
        KilledTerminationStatus
    };

    enum ClientType {
        QmlClient,
        WidgetsClient
    };

    enum MediaRequestFlag {
        MediaNone = 0,
        MediaAudioCapture = 0x01,
        MediaVideoCapture = 0x02,
        MediaDesktopAudioCapture = 0x04,
        MediaDesktopVideoCapture = 0x08
    };
    Q_DECLARE_FLAGS(MediaRequestFlags, MediaRequestFlag)

    enum class LifecycleState {
        Active,
        Frozen,
        Discarded,
    };

    enum class LoadingState {
        Unloaded,
        Loading,
        Loaded,
    };

    virtual ~WebContentsAdapterClient() { }

    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client) = 0;
    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegateForPopup(RenderWidgetHostViewQtDelegateClient *client) = 0;
    virtual void initializationFinished() = 0;
    virtual void lifecycleStateChanged(LifecycleState) = 0;
    virtual void recommendedStateChanged(LifecycleState) = 0;
    virtual void visibleChanged(bool) = 0;
    virtual void titleChanged(const QString&) = 0;
    virtual void urlChanged() = 0;
    virtual void iconChanged(const QUrl&) = 0;
    virtual void loadProgressChanged(int progress) = 0;
    virtual void didUpdateTargetURL(const QUrl&) = 0;
    virtual void selectionChanged() = 0;
    virtual void recentlyAudibleChanged(bool recentlyAudible) = 0;
    virtual void renderProcessPidChanged(qint64 pid) = 0;
    virtual QRectF viewportRect() const = 0;
    virtual QColor backgroundColor() const = 0;
    virtual void loadStarted(QWebEngineLoadingInfo info) = 0;
    virtual void loadCommitted() = 0;
    virtual void loadFinished(QWebEngineLoadingInfo info) = 0;
    virtual void focusContainer() = 0;
    virtual void unhandledKeyEvent(QKeyEvent *event) = 0;
    virtual QSharedPointer<WebContentsAdapter>
    adoptNewWindow(QSharedPointer<WebContentsAdapter> newWebContents,
                   WindowOpenDisposition disposition, bool userGesture,
                   const QRect &initialGeometry, const QUrl &targetUrl) = 0;
    virtual bool isBeingAdopted() = 0;
    virtual void close() = 0;
    virtual void windowCloseRejected() = 0;
    virtual void contextMenuRequested(QWebEngineContextMenuRequest *request) = 0;
    virtual void navigationRequested(int navigationType, const QUrl &url, bool &accepted, bool isMainFrame) = 0;
    virtual void requestFullScreenMode(const QUrl &origin, bool fullscreen) = 0;
    virtual bool isFullScreenMode() const = 0;
    virtual void javascriptDialog(QSharedPointer<JavaScriptDialogController>) = 0;
    virtual void runFileChooser(QSharedPointer<FilePickerController>) = 0;
    virtual void showColorDialog(QSharedPointer<ColorChooserController>) = 0;
    virtual void didRunJavaScript(quint64 requestId, const QVariant& result) = 0;
    virtual void didFetchDocumentMarkup(quint64 requestId, const QString& result) = 0;
    virtual void didFetchDocumentInnerText(quint64 requestId, const QString& result) = 0;
    virtual void didPrintPage(quint64 requestId, QSharedPointer<QByteArray>) = 0;
    virtual void didPrintPageToPdf(const QString &filePath, bool success) = 0;
    virtual bool passOnFocus(bool reverse) = 0;
    // returns the last QObject (QWidget/QQuickItem) based object in the accessibility
    // hierarchy before going into the BrowserAccessibility tree
    virtual QObject *accessibilityParentObject() = 0;
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) = 0;
    virtual void authenticationRequired(QSharedPointer<AuthenticationDialogController>) = 0;
    virtual void runFeaturePermissionRequest(ProfileAdapter::PermissionType, const QUrl &securityOrigin) = 0;
    virtual void runMediaAccessPermissionRequest(const QUrl &securityOrigin, MediaRequestFlags requestFlags) = 0;
    virtual void runMouseLockPermissionRequest(const QUrl &securityOrigin) = 0;
    virtual void runQuotaRequest(QWebEngineQuotaRequest) = 0;
    virtual void runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest) = 0;
    virtual QWebEngineSettings *webEngineSettings() const = 0;
    RenderProcessTerminationStatus renderProcessExitStatus(int);
    virtual void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode) = 0;
    virtual void requestGeometryChange(const QRect &geometry, const QRect &frameGeometry) = 0;
    virtual void allowCertificateError(const QWebEngineCertificateError &error) = 0;
    virtual void selectClientCert(const QSharedPointer<ClientCertSelectController> &selectController) = 0;
    virtual void updateScrollPosition(const QPointF &position) = 0;
    virtual void updateContentsSize(const QSizeF &size) = 0;
    virtual void updateNavigationActions() = 0;
    virtual void updateEditActions() = 0;
    virtual QObject *dragSource() const = 0;
    virtual bool isEnabled() const = 0;
    virtual const QObject *holdingQObject() const = 0;
    virtual void setToolTip(const QString& toolTipText) = 0;
    virtual ClientType clientType() = 0;
    virtual void printRequested() = 0;
    virtual TouchHandleDrawableClient *createTouchHandle(const QMap<int, QImage> &images) = 0;
    virtual void showTouchSelectionMenu(TouchSelectionMenuController *menuController, const QRect &bounds, const QSize &handleSize) = 0;
    virtual void hideTouchSelectionMenu() = 0;
    virtual void findTextFinished(const QWebEngineFindTextResult &result) = 0;

    virtual ProfileAdapter *profileAdapter() = 0;
    virtual WebContentsAdapter* webContentsAdapter() = 0;
    virtual void releaseProfile() = 0;
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_ADAPTER_CLIENT_H
