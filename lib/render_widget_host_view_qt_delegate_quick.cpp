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

#include "backing_store_qt.h"
#include "render_widget_host_view_qt.h"

#include "content/browser/renderer_host/render_view_host_impl.h"

#include <QQuickWindow>
#include <QWindow>

RenderWidgetHostViewQtDelegateQuick::RenderWidgetHostViewQtDelegateQuick(RenderWidgetHostViewQt* view, QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , RenderWidgetHostViewQtDelegate(view)
    , m_backingStore(0)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

QRectF RenderWidgetHostViewQtDelegateQuick::screenRect() const
{
    QPointF pos = mapToScene(QPointF(0,0));
    return QRectF(pos.x(), pos.y(), width(), height());
}

void RenderWidgetHostViewQtDelegateQuick::setKeyboardFocus()
{
    setFocus(true);
}

bool RenderWidgetHostViewQtDelegateQuick::hasKeyboardFocus()
{
    return hasFocus();
}

void RenderWidgetHostViewQtDelegateQuick::show()
{
    setVisible(true);
}

void RenderWidgetHostViewQtDelegateQuick::hide()
{
    setVisible(false);
}

bool RenderWidgetHostViewQtDelegateQuick::isVisible() const
{
    return QQuickPaintedItem::isVisible();
}

QWindow* RenderWidgetHostViewQtDelegateQuick::window() const
{
    return QQuickPaintedItem::window();
}

void RenderWidgetHostViewQtDelegateQuick::update(const QRect& rect)
{
    polish();
    QQuickPaintedItem::update(rect);
}

void RenderWidgetHostViewQtDelegateQuick::paint(QPainter *painter)
{
    if (!m_backingStore)
        return;

    m_backingStore->paintToTarget(painter, boundingRect());
}

void RenderWidgetHostViewQtDelegateQuick::updatePolish()
{
    // paint will be called from the scene graph thread and this doesn't play well
    // with chromium's use of TLS while getting the backing store.
    // updatePolish() should be called from the GUI thread right before the rendering thread starts.
    m_backingStore = m_view->GetBackingStore();
}

void RenderWidgetHostViewQtDelegateQuick::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);

    m_view->GetRenderWidgetHost()->WasResized();
}

void RenderWidgetHostViewQtDelegateQuick::focusInEvent(QFocusEvent *event)
{
    m_view->handleFocusEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::focusOutEvent(QFocusEvent *event)
{
    m_view->handleFocusEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mousePressEvent(QMouseEvent *event)
{
    setFocus(true);
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseMoveEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseReleaseEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_view->handleMouseEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::keyPressEvent(QKeyEvent *event)
{
    m_view->handleKeyEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::keyReleaseEvent(QKeyEvent *event)
{
    m_view->handleKeyEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::wheelEvent(QWheelEvent *event)
{
    m_view->handleWheelEvent(event);
}
