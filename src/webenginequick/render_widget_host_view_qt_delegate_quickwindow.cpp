/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "render_widget_host_view_qt_delegate_quick.h"
#include "render_widget_host_view_qt_delegate_quickwindow.h"

#include <QtQuick/qquickitem.h>

namespace QtWebEngineCore {

static inline QPoint getOffset(QQuickItem *item)
{
    // get parent window (scene) offset
    QPointF offset = item->mapFromScene(QPoint(0, 0));
    offset = item->mapToGlobal(offset);
    // get local offset
    offset -= item->mapToScene(QPoint(0, 0));
    return offset.toPoint();
}

static inline QPointF transformPoint(const QPointF &point, const QTransform &transform,
                                     const QPointF &offset, const QQuickItem *parent)
{
    // make scene vector
    QPointF a = point - offset;
    // apply local transformation
    a = transform.map(a);
    // make screen coordinates
    a = parent->mapFromScene(a);
    a = parent->mapToGlobal(a);
    return a;
}

RenderWidgetHostViewQtDelegateQuickWindow::RenderWidgetHostViewQtDelegateQuickWindow(
        RenderWidgetHostViewQtDelegateQuick *realDelegate, QWindow *parent)
    : QQuickWindow(parent), m_realDelegate(realDelegate), m_virtualParent(nullptr), m_rotated(false)
{
    setFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
}

RenderWidgetHostViewQtDelegateQuickWindow::~RenderWidgetHostViewQtDelegateQuickWindow()
{
}

void RenderWidgetHostViewQtDelegateQuickWindow::setVirtualParent(QQuickItem *virtualParent)
{
    Q_ASSERT(virtualParent);
    m_virtualParent = virtualParent;
}

// rect is window geometry in form of parent window offset + offset in scene coordinates
// chromium knows nothing about local transformation
void RenderWidgetHostViewQtDelegateQuickWindow::initAsPopup(const QRect &rect)
{
    m_rotated = m_virtualParent->parentItem()->rotation() > 0;
    if (m_rotated) {
        // code below tries to cover the case where webengine view is rotated,
        // the code assumes the rotation is in the form of  90, 180, 270 degrees
        // to archive that we keep chromium unaware of transformation and we transform
        // just the window content.
        m_rect = rect;
        // get parent window (scene) offset
        QPointF offset = m_virtualParent->mapFromScene(QPoint(0, 0));
        offset = m_virtualParent->mapToGlobal(offset);
        // get local transform
        QTransform transform = m_virtualParent->itemTransform(nullptr, nullptr);
        QPointF tl = transformPoint(rect.topLeft(), transform, offset, m_virtualParent);
        QPointF br = transformPoint(rect.bottomRight(), transform, offset, m_virtualParent);
        QRectF popupRect(tl, br);
        popupRect = popupRect.normalized();
        m_realDelegate->setSize(rect.size());
        // include offset from parent window
        popupRect.moveTo(popupRect.topLeft() - offset);
        setGeometry(popupRect.adjusted(0, 0, 1, 1).toRect());
        // add offset since screenRect and transformed popupRect one are different and
        // we want to rotate in center.
        m_realDelegate->setX(-rect.width() / 2.0 + geometry().width() / 2.0);
        m_realDelegate->setY(-rect.height() / 2.0 + geometry().height() / 2.0);
        m_realDelegate->setTransformOrigin(QQuickItem::Center);
        m_realDelegate->setRotation(m_virtualParent->parentItem()->rotation());
    } else {
        m_realDelegate->setSize(rect.size());
        QRect geometry(rect);
        geometry.moveTo(rect.topLeft() - getOffset(m_virtualParent));
        setGeometry(geometry);
    }
    raise();
    show();
}

QRectF RenderWidgetHostViewQtDelegateQuickWindow::viewGeometry() const
{
    return m_rotated ? m_rect : geometry();
}

QRect RenderWidgetHostViewQtDelegateQuickWindow::windowGeometry() const
{
    return m_rotated ? m_rect : frameGeometry();
}

void RenderWidgetHostViewQtDelegateQuickWindow::show()
{
    QQuickWindow::show();
    m_realDelegate->show();
}

void RenderWidgetHostViewQtDelegateQuickWindow::hide()
{
    QQuickWindow::hide();
    m_realDelegate->hide();
}

bool RenderWidgetHostViewQtDelegateQuickWindow::isVisible() const
{
    return QQuickWindow::isVisible();
}

QWindow *RenderWidgetHostViewQtDelegateQuickWindow::window() const
{
    return const_cast<RenderWidgetHostViewQtDelegateQuickWindow*>(this);
}

void RenderWidgetHostViewQtDelegateQuickWindow::updateCursor(const QCursor &cursor)
{
    setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateQuickWindow::resize(int width, int height)
{
    if (!m_rotated) {
        QQuickWindow::resize(width, height);
        m_realDelegate->resize(width, height);
    }
}

void RenderWidgetHostViewQtDelegateQuickWindow::move(const QPoint &screenPos)
{
    if (!m_rotated)
        QQuickWindow::setPosition(screenPos - getOffset(m_virtualParent));
}

} // namespace QtWebEngineCore
