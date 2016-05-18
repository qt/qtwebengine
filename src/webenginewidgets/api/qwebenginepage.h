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

#ifndef QWEBENGINEPAGE_H
#define QWEBENGINEPAGE_H

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>
#include <QtWebEngineWidgets/qwebenginecertificateerror.h>
#include <QtWebEngineCore/qwebenginecallback.h>

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE
class QMenu;
class QWebChannel;
class QWebEngineFullScreenRequest;
class QWebEngineHistory;
class QWebEnginePage;
class QWebEnginePagePrivate;
class QWebEngineProfile;
class QWebEngineScriptCollection;
class QWebEngineSettings;

class QWEBENGINEWIDGETS_EXPORT QWebEnginePage : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(bool hasSelection READ hasSelection)

    // Ex-QWebFrame properties
    Q_PROPERTY(QUrl requestedUrl READ requestedUrl)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QUrl iconUrl READ iconUrl)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

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

        WebActionCount
    };

    enum FindFlag {
        FindBackward = 1,
        FindCaseSensitively = 2,
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    enum WebWindowType {
        WebBrowserWindow,
        WebBrowserTab,
        WebDialog
    };

    enum PermissionPolicy {
        PermissionUnknown,
        PermissionGrantedByUser,
        PermissionDeniedByUser
    };

    // must match WebContentsAdapterClient::NavigationType
    enum NavigationType {
        NavigationTypeLinkClicked,
        NavigationTypeTyped,
        NavigationTypeFormSubmitted,
        NavigationTypeBackForward,
        NavigationTypeReload,
        NavigationTypeOther
    };

    enum Feature {
#ifndef Q_QDOC
        Notifications = 0,
#endif
        Geolocation = 1,
        MediaAudioCapture = 2,
        MediaVideoCapture,
        MediaAudioVideoCapture,
        MouseLock
    };

    // Ex-QWebFrame enum

    enum FileSelectionMode {
        FileSelectOpen,
        FileSelectOpenMultiple,
    };

    // must match WebContentsAdapterClient::JavaScriptConsoleMessageLevel
    enum JavaScriptConsoleMessageLevel {
        InfoMessageLevel = 0,
        WarningMessageLevel,
        ErrorMessageLevel
    };

    // must match WebContentsAdapterClient::RenderProcessTerminationStatus
    enum RenderProcessTerminationStatus {
        NormalTerminationStatus = 0,
        AbnormalTerminationStatus,
        CrashedTerminationStatus,
        KilledTerminationStatus
    };

    explicit QWebEnginePage(QObject *parent = Q_NULLPTR);
    QWebEnginePage(QWebEngineProfile *profile, QObject *parent = Q_NULLPTR);
    ~QWebEnginePage();
    QWebEngineHistory *history() const;

    void setView(QWidget *view);
    QWidget *view() const;

    bool hasSelection() const;
    QString selectedText() const;

    QWebEngineProfile *profile() const;

#ifndef QT_NO_ACTION
    QAction *action(WebAction action) const;
#endif
    virtual void triggerAction(WebAction action, bool checked = false);

    virtual bool event(QEvent*);
#ifdef Q_QDOC
    void findText(const QString &subString, FindFlags options = FindFlags());
    void findText(const QString &subString, FindFlags options, FunctorOrLambda resultCallback);
#else
    void findText(const QString &subString, FindFlags options = FindFlags(), const QWebEngineCallback<bool> &resultCallback = QWebEngineCallback<bool>());
#endif
    QMenu *createStandardContextMenu();

    void setFeaturePermission(const QUrl &securityOrigin, Feature feature, PermissionPolicy policy);

    // Ex-QWebFrame methods
    void load(const QUrl &url);
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());

#ifdef Q_QDOC
    void toHtml(FunctorOrLambda resultCallback) const;
    void toPlainText(FunctorOrLambda resultCallback) const;
#else
    void toHtml(const QWebEngineCallback<const QString &> &resultCallback) const;
    void toPlainText(const QWebEngineCallback<const QString &> &resultCallback) const;
#endif

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QUrl requestedUrl() const;
    QUrl iconUrl() const;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    void runJavaScript(const QString& scriptSource);
#ifdef Q_QDOC
    void runJavaScript(const QString& scriptSource, FunctorOrLambda resultCallback);
#else
    void runJavaScript(const QString& scriptSource, const QWebEngineCallback<const QVariant &> &resultCallback);
#endif
    QWebEngineScriptCollection &scripts();
    QWebEngineSettings *settings() const;

    QWebChannel *webChannel() const;
    void setWebChannel(QWebChannel *);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool ok);

    void linkHovered(const QString &url);
    void selectionChanged();
    void geometryChangeRequested(const QRect& geom);
    void windowCloseRequested();

    void featurePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    void featurePermissionRequestCanceled(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    void fullScreenRequested(QWebEngineFullScreenRequest fullScreenRequest);

    void authenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator);
    void proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost);

    void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode);

    // Ex-QWebFrame signals
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    // Was iconChanged() in QWebFrame
    void iconUrlChanged(const QUrl &url);

protected:
    virtual QWebEnginePage *createWindow(WebWindowType type);
    virtual QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes);
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString& msg);
    virtual bool javaScriptConfirm(const QUrl &securityOrigin, const QString& msg);
    virtual bool javaScriptPrompt(const QUrl &securityOrigin, const QString& msg, const QString& defaultValue, QString* result);
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
    virtual bool certificateError(const QWebEngineCertificateError &certificateError);
    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

private:
    Q_DISABLE_COPY(QWebEnginePage)
    Q_DECLARE_PRIVATE(QWebEnginePage)
    QScopedPointer<QWebEnginePagePrivate> d_ptr;
#ifndef QT_NO_ACTION
    Q_PRIVATE_SLOT(d_func(), void _q_webActionTriggered(bool checked))
#endif

    friend class QWebEngineFullScreenRequest;
    friend class QWebEngineView;
    friend class QWebEngineViewPrivate;
#ifndef QT_NO_ACCESSIBILITY
    friend class QWebEngineViewAccessible;
#endif // QT_NO_ACCESSIBILITY
};


QT_END_NAMESPACE

#endif // QWEBENGINEPAGE_H
