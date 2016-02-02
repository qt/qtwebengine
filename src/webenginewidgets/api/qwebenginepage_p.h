/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

#include "qwebenginecallback_p.h"
#include "qwebenginescriptcollection.h"
#include "web_contents_adapter_client.h"
#include <QtCore/qcompilerdetection.h>

namespace QtWebEngineCore {
class RenderWidgetHostViewQtDelegate;
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE
class QWebEngineHistory;
class QWebEnginePage;
class QWebEngineProfile;
class QWebEngineSettings;
class QWebEngineView;

class QWebEnginePagePrivate : public QtWebEngineCore::WebContentsAdapterClient
{
public:
    Q_DECLARE_PUBLIC(QWebEnginePage)
    QWebEnginePage *q_ptr;

    QWebEnginePagePrivate(QWebEngineProfile *profile = 0);
    ~QWebEnginePagePrivate();

    virtual QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) Q_DECL_OVERRIDE;
    virtual QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegateForPopup(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) Q_DECL_OVERRIDE { return CreateRenderWidgetHostViewQtDelegate(client); }
    virtual void titleChanged(const QString&) Q_DECL_OVERRIDE;
    virtual void urlChanged(const QUrl&) Q_DECL_OVERRIDE;
    virtual void iconChanged(const QUrl&) Q_DECL_OVERRIDE;
    virtual void loadProgressChanged(int progress) Q_DECL_OVERRIDE;
    virtual void didUpdateTargetURL(const QUrl&) Q_DECL_OVERRIDE;
    virtual void selectionChanged() Q_DECL_OVERRIDE;
    virtual QRectF viewportRect() const Q_DECL_OVERRIDE;
    virtual qreal dpiScale() const Q_DECL_OVERRIDE;
    virtual QColor backgroundColor() const Q_DECL_OVERRIDE;
    virtual void loadStarted(const QUrl &provisionalUrl, bool isErrorPage = false) Q_DECL_OVERRIDE;
    virtual void loadCommitted() Q_DECL_OVERRIDE;
    virtual void loadVisuallyCommitted() Q_DECL_OVERRIDE { }
    virtual void loadFinished(bool success, const QUrl &url, bool isErrorPage = false, int errorCode = 0, const QString &errorDescription = QString()) Q_DECL_OVERRIDE;
    virtual void focusContainer() Q_DECL_OVERRIDE;
    virtual void unhandledKeyEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void adoptNewWindow(QtWebEngineCore::WebContentsAdapter *newWebContents, WindowOpenDisposition disposition, bool userGesture, const QRect &initialGeometry) Q_DECL_OVERRIDE;
    virtual bool isBeingAdopted() Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;
    virtual void windowCloseRejected() Q_DECL_OVERRIDE;
    virtual bool contextMenuRequested(const QtWebEngineCore::WebEngineContextMenuData &data) Q_DECL_OVERRIDE;
    virtual void navigationRequested(int navigationType, const QUrl &url, int &navigationRequestAction, bool isMainFrame) Q_DECL_OVERRIDE;
    virtual void requestFullScreenMode(const QUrl &origin, bool fullscreen) Q_DECL_OVERRIDE;
    virtual bool isFullScreenMode() const Q_DECL_OVERRIDE;
    virtual void javascriptDialog(QSharedPointer<QtWebEngineCore::JavaScriptDialogController>) Q_DECL_OVERRIDE;
    virtual void runFileChooser(QtWebEngineCore::FilePickerController *controller) Q_DECL_OVERRIDE;
    virtual void didRunJavaScript(quint64 requestId, const QVariant& result) Q_DECL_OVERRIDE;
    virtual void didFetchDocumentMarkup(quint64 requestId, const QString& result) Q_DECL_OVERRIDE;
    virtual void didFetchDocumentInnerText(quint64 requestId, const QString& result) Q_DECL_OVERRIDE;
    virtual void didFindText(quint64 requestId, int matchCount) Q_DECL_OVERRIDE;
    virtual void passOnFocus(bool reverse) Q_DECL_OVERRIDE;
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) Q_DECL_OVERRIDE;
    virtual void authenticationRequired(QSharedPointer<QtWebEngineCore::AuthenticationDialogController>) Q_DECL_OVERRIDE;
    virtual void runMediaAccessPermissionRequest(const QUrl &securityOrigin, MediaRequestFlags requestFlags) Q_DECL_OVERRIDE;
    virtual void runGeolocationPermissionRequest(const QUrl &securityOrigin) Q_DECL_OVERRIDE;
    virtual void runMouseLockPermissionRequest(const QUrl &securityOrigin) Q_DECL_OVERRIDE;
#ifndef QT_NO_ACCESSIBILITY
    virtual QObject *accessibilityParentObject() Q_DECL_OVERRIDE;
#endif // QT_NO_ACCESSIBILITY
    virtual QtWebEngineCore::WebEngineSettings *webEngineSettings() const Q_DECL_OVERRIDE;
    virtual void allowCertificateError(const QSharedPointer<CertificateErrorController> &controller) Q_DECL_OVERRIDE;
    virtual void showValidationMessage(const QRect &anchor, const QString &mainText, const QString &subText) Q_DECL_OVERRIDE;
    virtual void hideValidationMessage() Q_DECL_OVERRIDE;
    virtual void moveValidationMessage(const QRect &anchor) Q_DECL_OVERRIDE;
    virtual void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus,
                                     int exitCode) Q_DECL_OVERRIDE;
    virtual void requestGeometryChange(const QRect &geometry) Q_DECL_OVERRIDE;

    virtual QtWebEngineCore::BrowserContextAdapter *browserContextAdapter() Q_DECL_OVERRIDE;

    void updateAction(QWebEnginePage::WebAction) const;
    void updateNavigationActions();
    void _q_webActionTriggered(bool checked);

    void wasShown();
    void wasHidden();

    QtWebEngineCore::WebContentsAdapter *webContents() { return adapter.data(); }
    void recreateFromSerializedHistory(QDataStream &input);

    void setFullScreenMode(bool);

    QExplicitlySharedDataPointer<QtWebEngineCore::WebContentsAdapter> adapter;
    QWebEngineHistory *history;
    QWebEngineProfile *profile;
    QWebEngineSettings *settings;
    QWebEngineView *view;
    QUrl explicitUrl;
    QtWebEngineCore::WebEngineContextMenuData m_menuData;
    bool isLoading;
    QWebEngineScriptCollection scriptCollection;
    bool m_isBeingAdopted;
    QColor m_backgroundColor;
    bool fullscreenMode;
    QWebChannel *webChannel;

    mutable QtWebEngineCore::CallbackDirectory m_callbacks;
    mutable QAction *actions[QWebEnginePage::WebActionCount];
};

QT_END_NAMESPACE

#endif // QWEBENGINEPAGE_P_H
