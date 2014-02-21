/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WEBPAGE_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WEBPAGE_H

#include "render_widget_host_view_qt_delegate.h"
#include "web_contents_adapter_client.h"

#include <QObject>

class BackingStoreQt;

QT_BEGIN_NAMESPACE
class QWindow;
class QWebEnginePage;
class QWebEnginePagePrivate;
QT_END_NAMESPACE

class RenderWidgetHostViewQtDelegateWebPage : public QObject, public RenderWidgetHostViewQtDelegate
{
public:
    RenderWidgetHostViewQtDelegateWebPage(RenderWidgetHostViewQtDelegateClient *client);

    virtual void initAsChild(WebContentsAdapterClient* container) Q_DECL_OVERRIDE;
    virtual void initAsPopup(const QRect&) Q_DECL_OVERRIDE { Q_UNREACHABLE(); }
    virtual QRectF screenRect() const Q_DECL_OVERRIDE;
    virtual void setKeyboardFocus() Q_DECL_OVERRIDE;
    virtual bool hasKeyboardFocus() Q_DECL_OVERRIDE;
    virtual void show() Q_DECL_OVERRIDE {}
    virtual void hide() Q_DECL_OVERRIDE {}
    virtual bool isVisible() const Q_DECL_OVERRIDE;
    virtual QWindow* window() const Q_DECL_OVERRIDE;
    virtual void update(const QRect& rect = QRect()) Q_DECL_OVERRIDE;
    virtual void updateCursor(const QCursor &) Q_DECL_OVERRIDE;
    virtual void resize(int width, int height) Q_DECL_OVERRIDE;
    virtual void move(const QPoint&) Q_DECL_OVERRIDE {}
    virtual void inputMethodStateChanged(bool editorVisible) Q_DECL_OVERRIDE;
    virtual bool supportsHardwareAcceleration() const Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QRectF &boundingRect);
    void notifyResize();
    bool forwardEvent(QEvent *event);

protected:

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

private:
    RenderWidgetHostViewQtDelegateClient *m_client;
    QWebEnginePage *m_page;
    QWebEnginePagePrivate *m_pagePrivate;
};

#endif
