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

#ifndef QWEBENGINEPAGE_P_H
#define QWEBENGINEPAGE_P_H

#include "qwebenginepage.h"

#include "web_contents_adapter_client.h"
#include <QtCore/private/qobject_p.h>
#include <QSharedData>

class QWebEngineHistory;
class QWebEnginePage;
class QWebEngineView;
class RenderWidgetHostViewQtDelegate;
class WebContentsAdapter;

class QWebEnginePagePrivate : public QObjectPrivate, public WebContentsAdapterClient
{
public:
    Q_DECLARE_PUBLIC(QWebEnginePage)

    QWebEnginePagePrivate();
    ~QWebEnginePagePrivate();

    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate() Q_DECL_OVERRIDE;
    virtual void titleChanged(const QString&) Q_DECL_OVERRIDE;
    virtual void urlChanged(const QUrl&) Q_DECL_OVERRIDE;
    virtual void loadingStateChanged() Q_DECL_OVERRIDE;
    virtual QRectF viewportRect() const Q_DECL_OVERRIDE;
    virtual void loadFinished(bool success) Q_DECL_OVERRIDE;
    virtual void focusContainer() Q_DECL_OVERRIDE;
    virtual void adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition) Q_DECL_OVERRIDE;
    virtual bool contextMenuRequested(const WebEngineContextMenuData &data) Q_DECL_OVERRIDE;
    virtual bool javascriptDialog(JavascriptDialogType type, const QString &message, const QString &defaultValue = QString(), QString *result = 0) Q_DECL_OVERRIDE;

    void updateAction(QWebEnginePage::WebAction) const;
    void updateNavigationActions();
    void _q_webActionTriggered(bool checked);

    QExplicitlySharedDataPointer<WebContentsAdapter> adapter;
    QWebEngineHistory *history;
    QWebEngineView *view;
    mutable QAction *actions[QWebEnginePage::WebActionCount];
    bool m_isLoading;
    WebEngineContextMenuData m_menuData;
};

#endif // QWEBENGINEPAGE_P_H
