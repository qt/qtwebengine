/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKWEBENGINEVIEW_P_P_H
#define QQUICKWEBENGINEVIEW_P_P_H

#include "qquickwebengineview_p.h"
#include "web_contents_adapter_client.h"

#include <QScopedPointer>
#include <QSharedData>
#include <QString>
#include <QtCore/qcompilerdetection.h>
#include <QtQuick/private/qquickitem_p.h>

class WebContentsAdapter;
class UIDelegatesManager;

QT_BEGIN_NAMESPACE
class QQuickWebEngineHistory;
class QQuickWebEngineNewViewRequest;
class QQuickWebEngineView;
class QQmlComponent;
class QQmlContext;

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineViewport : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio WRITE setDevicePixelRatio NOTIFY devicePixelRatioChanged)
public:
    QQuickWebEngineViewport(QQuickWebEngineViewPrivate *viewPrivate);

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(qreal);

Q_SIGNALS:
    void devicePixelRatioChanged();

private:
    QQuickWebEngineViewPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QQuickWebEngineView)
};

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineViewExperimental : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineViewport *viewport READ viewport)
    Q_PROPERTY(QQmlComponent *extraContextMenuEntriesComponent READ extraContextMenuEntriesComponent WRITE setExtraContextMenuEntriesComponent NOTIFY extraContextMenuEntriesComponentChanged)
    Q_PROPERTY(bool isFullScreen READ isFullScreen WRITE setIsFullScreen NOTIFY isFullScreenChanged)
    Q_PROPERTY(QQuickWebEngineHistory *navigationHistory READ navigationHistory CONSTANT FINAL)

public:
    void setIsFullScreen(bool fullscreen);
    bool isFullScreen() const;
    QQuickWebEngineViewport *viewport() const;
    void setExtraContextMenuEntriesComponent(QQmlComponent *);
    QQmlComponent *extraContextMenuEntriesComponent() const;
    QQuickWebEngineHistory *navigationHistory() const;

public Q_SLOTS:
    void goBackTo(int index);
    void goForwardTo(int index);
    void runJavaScript(const QString&, const QJSValue & = QJSValue());

Q_SIGNALS:
    void newViewRequested(QQuickWebEngineNewViewRequest *request);
    void fullScreenRequested(bool fullScreen);
    void isFullScreenChanged();
    void extraContextMenuEntriesComponentChanged();

private:
    QQuickWebEngineViewExperimental(QQuickWebEngineViewPrivate* viewPrivate);
    QQuickWebEngineView *q_ptr;
    QQuickWebEngineViewPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QQuickWebEngineView)
    Q_DECLARE_PUBLIC(QQuickWebEngineView)
};

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineViewPrivate : public QQuickItemPrivate, public WebContentsAdapterClient
{
public:
    Q_DECLARE_PUBLIC(QQuickWebEngineView)
    QQuickWebEngineViewPrivate();
    ~QQuickWebEngineViewPrivate();

    QQuickWebEngineViewExperimental *experimental() const;
    QQuickWebEngineViewport *viewport() const;
    UIDelegatesManager *ui();

    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client) Q_DECL_OVERRIDE;
    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegateForPopup(RenderWidgetHostViewQtDelegateClient *client) Q_DECL_OVERRIDE;
    virtual void titleChanged(const QString&) Q_DECL_OVERRIDE;
    virtual void urlChanged(const QUrl&) Q_DECL_OVERRIDE;
    virtual void iconChanged(const QUrl&) Q_DECL_OVERRIDE;
    virtual void loadProgressChanged(int progress) Q_DECL_OVERRIDE;
    virtual void didUpdateTargetURL(const QUrl&) Q_DECL_OVERRIDE;
    virtual void selectionChanged() Q_DECL_OVERRIDE { }
    virtual QRectF viewportRect() const Q_DECL_OVERRIDE;
    virtual QPoint mapToGlobal(const QPoint &posInView) const Q_DECL_OVERRIDE;
    virtual qreal dpiScale() const Q_DECL_OVERRIDE;
    virtual void loadStarted(const QUrl &provisionalUrl) Q_DECL_OVERRIDE;
    virtual void loadCommitted() Q_DECL_OVERRIDE;
    virtual void loadFinished(bool success, int error_code = 0, const QString &error_description = QString()) Q_DECL_OVERRIDE;
    virtual void focusContainer() Q_DECL_OVERRIDE;
    virtual void adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition, bool userGesture, const QRect &) Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;
    virtual void requestFullScreen(bool) Q_DECL_OVERRIDE;
    virtual bool isFullScreen() const Q_DECL_OVERRIDE;
    virtual bool contextMenuRequested(const WebEngineContextMenuData &) Q_DECL_OVERRIDE;
    virtual void javascriptDialog(QSharedPointer<JavaScriptDialogController>) Q_DECL_OVERRIDE;
    virtual void runFileChooser(FileChooserMode, const QString &defaultFileName, const QStringList &acceptedMimeTypes) Q_DECL_OVERRIDE;
    virtual void didRunJavaScript(quint64, const QVariant&) Q_DECL_OVERRIDE;
    virtual void didFetchDocumentMarkup(quint64, const QString&) Q_DECL_OVERRIDE { }
    virtual void didFetchDocumentInnerText(quint64, const QString&) Q_DECL_OVERRIDE { }
    virtual void didFindText(quint64, int) Q_DECL_OVERRIDE { }
    virtual void passOnFocus(bool reverse) Q_DECL_OVERRIDE;
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) Q_DECL_OVERRIDE;
    virtual void authenticationRequired(const QUrl&, const QString&, bool, const QString&, QString*, QString*) Q_DECL_OVERRIDE { }

    void setDevicePixelRatio(qreal);
    void adoptWebContents(WebContentsAdapter *webContents);

    QExplicitlySharedDataPointer<WebContentsAdapter> adapter;
    QScopedPointer<QQuickWebEngineViewExperimental> e;
    QScopedPointer<QQuickWebEngineViewport> v;
    QScopedPointer<QQuickWebEngineHistory> m_history;
    QQmlComponent *contextMenuExtraItems;
    QUrl icon;
    int loadProgress;
    bool inspectable;
    bool m_isFullScreen;
    qreal devicePixelRatio;
    QMap<quint64, QJSValue> m_callbacks;

private:
    QScopedPointer<UIDelegatesManager> m_uIDelegatesManager;
    qreal m_dpiScale;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineViewExperimental)
QML_DECLARE_TYPE(QQuickWebEngineViewport)

#endif // QQUICKWEBENGINEVIEW_P_P_H
