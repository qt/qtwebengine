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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H

#include "render_widget_host_view_qt_delegate.h"

#include <QQuickItem>

class RenderWidgetHostViewQtDelegateQuick : public QQuickItem, public RenderWidgetHostViewQtDelegate
{
    Q_OBJECT
public:
    RenderWidgetHostViewQtDelegateQuick(RenderWidgetHostViewQtDelegateClient *client, bool isPopup);

    virtual void initAsChild(WebContentsAdapterClient* container) Q_DECL_OVERRIDE;
    virtual void initAsPopup(const QRect&) Q_DECL_OVERRIDE;
    virtual QRectF screenRect() const Q_DECL_OVERRIDE;
    virtual void setKeyboardFocus() Q_DECL_OVERRIDE;
    virtual bool hasKeyboardFocus() Q_DECL_OVERRIDE;
    virtual void show() Q_DECL_OVERRIDE;
    virtual void hide() Q_DECL_OVERRIDE;
    virtual bool isVisible() const Q_DECL_OVERRIDE;
    virtual QWindow* window() const Q_DECL_OVERRIDE;
    virtual void update() Q_DECL_OVERRIDE;
    virtual void updateCursor(const QCursor &) Q_DECL_OVERRIDE;
    virtual void resize(int width, int height) Q_DECL_OVERRIDE;
    virtual void move(const QPoint&) Q_DECL_OVERRIDE { }
    virtual void inputMethodStateChanged(bool editorVisible) Q_DECL_OVERRIDE;
    virtual void setTooltip(const QString&) Q_DECL_OVERRIDE { }

protected:
    virtual void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void touchEvent(QTouchEvent *event) Q_DECL_OVERRIDE;
    virtual void hoverMoveEvent(QHoverEvent *event) Q_DECL_OVERRIDE;
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const Q_DECL_OVERRIDE;
    virtual void inputMethodEvent(QInputMethodEvent *event) Q_DECL_OVERRIDE;
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) Q_DECL_OVERRIDE;
    virtual void itemChange(ItemChange change, const ItemChangeData &value) Q_DECL_OVERRIDE;
    virtual QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) Q_DECL_OVERRIDE;

private:
    RenderWidgetHostViewQtDelegateClient *m_client;
    bool m_isPopup;
    bool m_initialized;
};
#endif
