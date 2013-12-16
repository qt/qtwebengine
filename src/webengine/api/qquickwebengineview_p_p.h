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
    Q_PROPERTY(QQmlComponent *contextMenuExtraItems READ contextMenuExtraItems WRITE setContextMenuExtraItems NOTIFY contextMenuExtraItemsChanged)

public:
    QQuickWebEngineViewport *viewport() const;
    void setContextMenuExtraItems(QQmlComponent *);
    QQmlComponent *contextMenuExtraItems() const;

Q_SIGNALS:
    void contextMenuExtraItemsChanged();

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

    QQuickWebEngineViewExperimental *experimental() const;
    QQuickWebEngineViewport *viewport() const;
    UIDelegatesManager *ui();

    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client, RenderingMode) Q_DECL_OVERRIDE;
    virtual void titleChanged(const QString&) Q_DECL_OVERRIDE;
    virtual void urlChanged(const QUrl&) Q_DECL_OVERRIDE;
    virtual void iconChanged(const QUrl&) Q_DECL_OVERRIDE;
    virtual void loadingStateChanged() Q_DECL_OVERRIDE;
    virtual void loadProgressChanged(int progress) Q_DECL_OVERRIDE;
    virtual QRectF viewportRect() const Q_DECL_OVERRIDE;
    virtual qreal dpiScale() const Q_DECL_OVERRIDE;
    virtual void loadFinished(bool success) Q_DECL_OVERRIDE;
    virtual void focusContainer() Q_DECL_OVERRIDE;
    virtual void adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition, const QRect &) Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;
    virtual bool contextMenuRequested(const WebEngineContextMenuData &) Q_DECL_OVERRIDE;
    virtual void javascriptDialog(JavaScriptDialogController *) Q_DECL_OVERRIDE;
    virtual void runFileChooser(FileChooserMode, const QString &defaultFileName, const QString &title, const QStringList &acceptedMimeTypes);

    void setDevicePixelRatio(qreal);

    QExplicitlySharedDataPointer<WebContentsAdapter> adapter;
    QScopedPointer<QQuickWebEngineViewExperimental> e;
    QScopedPointer<QQuickWebEngineViewport> v;
    QQmlComponent *contextMenuExtraItems;

    QUrl icon;
    int loadProgress;
    bool inspectable;
    qreal devicePixelRatio;

private:
    QScopedPointer<UIDelegatesManager> m_uIDelegatesManager;
    qreal m_dpiScale;

};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineViewExperimental)
QML_DECLARE_TYPE(QQuickWebEngineViewport)

#endif // QQUICKWEBENGINEVIEW_P_P_H
