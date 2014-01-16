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
#include <QtWebEngineWidgets/qwebenginesettings.h>

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE
class QMenu;

// FIXME: Just forward-declare the to-be-removed frame classe for now.
// Referencing calls should be ported to be page-friendly or removed individually.
class QWebEngineFrame;

class QWebEngineHistory;
class QWebEnginePagePrivate;

namespace QtWebEnginePrivate {

struct FunctorBase {
    virtual ~FunctorBase() {}
    virtual void operator()(const QVariant &) = 0;
};

template <typename F>
struct FunctorCallback : public FunctorBase {
    FunctorCallback(F callback) : m_callback(callback) {}
    virtual void operator()(const QVariant &value) Q_DECL_OVERRIDE { m_callback(value); }
private:
    F m_callback;
};
}

class QWEBENGINEWIDGETS_EXPORT QWebEngineHitTestResult {
};

class QWEBENGINEWIDGETS_EXPORT QWebEnginePage : public QObject {
    Q_OBJECT

    // Ex-QWebFrame properties
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

public:
    enum NavigationType {
    };

    enum WebAction {
        NoWebAction = - 1,
        Back,
        Forward,
        Stop,
        Reload,
        ReloadAndBypassCache,
        WebActionCount
    };

    enum WebWindowType {
        WebBrowserWindow,
        WebModalDialog
    };

    // Ex-QWebFrame enum

    enum FileSelectionMode {
        FileSelectOpen,
        FileSelectOpenMultiple,
    };

    explicit QWebEnginePage(QObject *parent = 0);
    ~QWebEnginePage();
    QWebEngineHistory *history() const;

    void setView(QWidget *view);
    QWidget *view() const;

#ifndef QT_NO_ACTION
    QAction *action(WebAction action) const;
#endif
    virtual void triggerAction(WebAction action, bool checked = false);

    QMenu *createStandardContextMenu();

    enum Extension {
    };
    class ExtensionOption
    {};
    class ExtensionReturn
    {};

    // Ex-QWebFrame methods
    void load(const QUrl &url);

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    void runJavaScript(const QString& scriptSource, const QString &xPath = QString());

    template <typename F>
    void runJavaScript(const QString& scriptSource, F func, const QString &xPath = QString());

Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool ok);

    void geometryChangeRequested(const QRect& geom);
    void windowCloseRequested();

    // Ex-QWebFrame signals
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);

protected:
    virtual QWebEnginePage *createWindow(WebWindowType type);

    virtual QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes);
    virtual void javaScriptAlert(QWebEngineFrame *originatingFrame, const QString& msg);
    virtual bool javaScriptConfirm(QWebEngineFrame *originatingFrame, const QString& msg);
    virtual bool javaScriptPrompt(QWebEngineFrame *originatingFrame, const QString& msg, const QString& defaultValue, QString* result);

private:
    Q_DECLARE_PRIVATE(QWebEnginePage);
#ifndef QT_NO_ACTION
    Q_PRIVATE_SLOT(d_func(), void _q_webActionTriggered(bool checked))
#endif
    void runJavaScriptHelper(const QString &source, QtWebEnginePrivate::FunctorBase *, const QString &xPath);

    friend class QWebEngineView;
    friend class QWebEngineViewPrivate;
};

template <typename F>
inline void QWebEnginePage::runJavaScript(const QString &scriptSource, F func, const QString &xPath)
{
    runJavaScriptHelper(scriptSource, new QtWebEnginePrivate::FunctorCallback<F>(func), xPath);
}

QT_END_NAMESPACE

#endif // QWEBENGINEPAGE_H
