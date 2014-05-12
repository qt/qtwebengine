/*
    Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBENGINEPAGE_H
#define QWEBENGINEPAGE_H

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE
class QMenu;
class QWebEngineHistory;
class QWebEnginePage;
class QWebEnginePagePrivate;

namespace QtWebEnginePrivate {

template <typename T>
class QWebEngineCallbackPrivateBase : public QSharedData {
public:
    virtual ~QWebEngineCallbackPrivateBase() {}
    virtual void operator()(T) = 0;
};

template <typename T, typename F>
class QWebEngineCallbackPrivate : public QWebEngineCallbackPrivateBase<T> {
public:
    QWebEngineCallbackPrivate(F callable) : m_callable(callable) {}
    virtual void operator()(T value) Q_DECL_OVERRIDE { m_callable(value); }
private:
    F m_callable;
};

} // namespace QtWebEnginePrivate

template <typename T>
class QWebEngineCallback {
public:
    template <typename F>
    QWebEngineCallback(F f) : d(new QtWebEnginePrivate::QWebEngineCallbackPrivate<T, F>(f)) { }
    QWebEngineCallback() { }
private:
    QExplicitlySharedDataPointer<QtWebEnginePrivate::QWebEngineCallbackPrivateBase<T> > d;
    friend class QWebEnginePage;
};

class QWEBENGINEWIDGETS_EXPORT QWebEnginePage : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(bool hasSelection READ hasSelection)

    // Ex-QWebFrame properties
    Q_PROPERTY(QUrl requestedUrl READ requestedUrl)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

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

    enum Feature {
        Notifications,
        Geolocation,
        MediaAudioDevices,
        MediaVideoDevices,
        MediaAudioVideoDevices
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

    explicit QWebEnginePage(QObject *parent = 0);
    ~QWebEnginePage();
    QWebEngineHistory *history() const;

    void setView(QWidget *view);
    QWidget *view() const;

    bool hasSelection() const;
    QString selectedText() const;

#ifndef QT_NO_ACTION
    QAction *action(WebAction action) const;
#endif
    virtual void triggerAction(WebAction action, bool checked = false);

    virtual bool event(QEvent*);
    void findText(const QString &subString, FindFlags options = 0, const QWebEngineCallback<bool> &resultCallback = QWebEngineCallback<bool>());
    QMenu *createStandardContextMenu();

    void setFeaturePermission(const QUrl &securityOrigin, Feature feature, PermissionPolicy policy);

    // Ex-QWebFrame methods
    void load(const QUrl &url);
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());

    void toHtml(const QWebEngineCallback<const QString &> &resultCallback) const;
    void toPlainText(const QWebEngineCallback<const QString &> &resultCallback) const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QUrl requestedUrl() const;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    void runJavaScript(const QString& scriptSource, const QString &xPath = QString());
    void runJavaScript(const QString& scriptSource, const QWebEngineCallback<const QVariant &> &resultCallback, const QString &xPath = QString());

Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool ok);

    void selectionChanged();
    void geometryChangeRequested(const QRect& geom);
    void windowCloseRequested();

    void featurePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    void featurePermissionRequestCanceled(const QUrl &securityOrigin, QWebEnginePage::Feature feature);

    void authenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator);
    void proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *authenticator, const QString &proxyHost);

    // Ex-QWebFrame signals
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);

protected:
    virtual QWebEnginePage *createWindow(WebWindowType type);

    virtual QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes);
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString& msg);
    virtual bool javaScriptConfirm(const QUrl &securityOrigin, const QString& msg);
    virtual bool javaScriptPrompt(const QUrl &securityOrigin, const QString& msg, const QString& defaultValue, QString* result);
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);

private:
    Q_DECLARE_PRIVATE(QWebEnginePage);
#ifndef QT_NO_ACTION
    Q_PRIVATE_SLOT(d_func(), void _q_webActionTriggered(bool checked))
#endif

    friend class QWebEngineView;
    friend class QWebEngineViewPrivate;
};


QT_END_NAMESPACE

#endif // QWEBENGINEPAGE_H
