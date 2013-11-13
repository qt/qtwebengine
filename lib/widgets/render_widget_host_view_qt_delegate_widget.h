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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WIDGET_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WIDGET_H

#include "render_widget_host_view_qt_delegate.h"
#include "web_contents_adapter_client.h"

#include <QWidget>

class BackingStoreQt;

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

class RenderWidgetHostViewQtDelegateWidget : public QWidget, public RenderWidgetHostViewQtDelegate
{
public:
    RenderWidgetHostViewQtDelegateWidget(RenderWidgetHostViewQtDelegateClient *client, QWidget *parent = 0);

    virtual void initAsChild(WebContentsAdapterClient* container);
    virtual void initAsPopup(const QRect&);
    virtual QRectF screenRect() const;
    virtual void setKeyboardFocus();
    virtual bool hasKeyboardFocus();
    virtual void show();
    virtual void hide();
    virtual bool isVisible() const;
    virtual WId nativeWindowIdForCompositor() const;
    virtual QWindow* window() const;
    virtual void update(const QRect& rect = QRect());
    virtual void updateCursor(const QCursor &);
    virtual void resize(int width, int height);
    virtual void inputMethodStateChanged(bool editorVisible);

protected:
    void paintEvent(QPaintEvent * event);
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *resizeEvent);

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

private:
    RenderWidgetHostViewQtDelegateClient *m_client;
};

#endif
