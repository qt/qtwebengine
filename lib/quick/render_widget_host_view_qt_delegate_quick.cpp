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

#include "render_widget_host_view_qt_delegate_quick.h"

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"
#include <QQuickWindow>
#include <QWindow>

template<typename ItemBaseType>
RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::RenderWidgetHostViewQtDelegateQuickBase(QQuickItem *parent)
    : ItemBaseType(parent)
{
    ItemBaseType::setAcceptedMouseButtons(Qt::AllButtons);
    ItemBaseType::setAcceptHoverEvents(true);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::initAsChild(WebContentsAdapterClient* container)
{
    QQuickWebEngineViewPrivate *viewPrivate = static_cast<QQuickWebEngineViewPrivate *>(container);
    ItemBaseType::setParentItem(viewPrivate->q_func());
}

template<typename ItemBaseType>
QRectF RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::screenRect() const
{
    QPointF pos = ItemBaseType::mapToScene(QPointF(0,0));
    return QRectF(pos.x(), pos.y(), ItemBaseType::width(), ItemBaseType::height());
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::setKeyboardFocus()
{
    ItemBaseType::setFocus(true);
}

template<typename ItemBaseType>
bool RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::hasKeyboardFocus()
{
    return ItemBaseType::hasFocus();
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::show()
{
    ItemBaseType::setVisible(true);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::hide()
{
    ItemBaseType::setVisible(false);
}

template<typename ItemBaseType>
bool RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::isVisible() const
{
    return ItemBaseType::isVisible();
}

template<typename ItemBaseType>
QWindow* RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::window() const
{
    return ItemBaseType::window();
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::updateCursor(const QCursor &cursor)
{
    ItemBaseType::setCursor(cursor);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::resize(int width, int height)
{
    ItemBaseType::setSize(QSizeF(width, height));
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::focusInEvent(QFocusEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::focusOutEvent(QFocusEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::mousePressEvent(QMouseEvent *event)
{
    ItemBaseType::setFocus(true);
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::mouseMoveEvent(QMouseEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::mouseReleaseEvent(QMouseEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::mouseDoubleClickEvent(QMouseEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::keyPressEvent(QKeyEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::keyReleaseEvent(QKeyEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::wheelEvent(QWheelEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::touchEvent(QTouchEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::hoverMoveEvent(QHoverEvent *event)
{
    forwardEvent(event);
}

template<typename ItemBaseType>
void RenderWidgetHostViewQtDelegateQuickBase<ItemBaseType>::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    ItemBaseType::geometryChanged(newGeometry, oldGeometry);
    notifyResize();
}


RenderWidgetHostViewQtDelegateQuickPainted::RenderWidgetHostViewQtDelegateQuickPainted(QQuickItem *parent)
    : RenderWidgetHostViewQtDelegateQuickBase<QQuickPaintedItem>(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
}

WId RenderWidgetHostViewQtDelegateQuickPainted::nativeWindowIdForCompositor() const
{
    // FIXME: update comment.
    // Only used to enable accelerated compositing by the compositor
    // directly on our native window, which we want to eventually do
    // through the delegated renderer instead.
    return 0;
}

void RenderWidgetHostViewQtDelegateQuickPainted::update(const QRect& rect)
{
    polish();
    QQuickPaintedItem::update(rect);
}

void RenderWidgetHostViewQtDelegateQuickPainted::paint(QPainter *painter)
{
    RenderWidgetHostViewQtDelegate::paint(painter, boundingRect());
}

void RenderWidgetHostViewQtDelegateQuickPainted::updatePolish()
{
    // paint will be called from the scene graph thread and this doesn't play well
    // with chromium's use of TLS while getting the backing store.
    // updatePolish() should be called from the GUI thread right before the rendering thread starts.
    fetchBackingStore();
}
