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

#include "render_widget_host_view_qt_delegate_quickwindow.h"

#include "qquickwebengineview_p_p.h"
#include <QQuickItem>

namespace QtWebEngineCore {

RenderWidgetHostViewQtDelegateQuickWindow::RenderWidgetHostViewQtDelegateQuickWindow(RenderWidgetHostViewQtDelegateQuick *realDelegate)
    : m_realDelegate(realDelegate)
    , m_virtualParent(nullptr)
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

static inline QRectF mapRectToGlobal(const QQuickItem *item, const QRectF &rect)
{
    const QPointF p1 = item->mapToGlobal(rect.topLeft());
    const QPointF p2 = item->mapToGlobal(rect.bottomRight());
    return QRectF(p1, p2).normalized();
}

static inline QRectF mapRectFromGlobal(const QQuickItem *item, const QRectF &rect)
{
    const QPointF p1 = item->mapFromGlobal(rect.topLeft());
    const QPointF p2 = item->mapFromGlobal(rect.bottomRight());
    return QRectF(p1, p2).normalized();
}

void RenderWidgetHostViewQtDelegateQuickWindow::initAsPopup(const QRect &screenRect)
{
    QRectF popupRect(screenRect);
    popupRect = mapRectFromGlobal(m_virtualParent, popupRect);
    popupRect = m_virtualParent->mapRectToScene(popupRect);
    popupRect = mapRectToGlobal(m_virtualParent, popupRect);

    m_realDelegate->initAsPopup(QRect(QPoint(0, 0), popupRect.size().toSize()));
    popupRect.setSize(screenRect.size());
    setGeometry(popupRect.toAlignedRect());
    raise();
    show();
}

QRectF RenderWidgetHostViewQtDelegateQuickWindow::viewGeometry() const
{
    return geometry();
}

QRect RenderWidgetHostViewQtDelegateQuickWindow::windowGeometry() const
{
    return frameGeometry();
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

QSGTexture *RenderWidgetHostViewQtDelegateQuickWindow::createTextureFromImage(const QImage &image)
{
    return m_realDelegate->createTextureFromImage(image);
}

QSGLayer *RenderWidgetHostViewQtDelegateQuickWindow::createLayer()
{
    return m_realDelegate->createLayer();
}

QSGImageNode *RenderWidgetHostViewQtDelegateQuickWindow::createImageNode()
{
    return m_realDelegate->createImageNode();
}

QSGRectangleNode *RenderWidgetHostViewQtDelegateQuickWindow::createRectangleNode()
{
    return m_realDelegate->createRectangleNode();
}

void RenderWidgetHostViewQtDelegateQuickWindow::update()
{
    QQuickWindow::update();
    m_realDelegate->update();
}

void RenderWidgetHostViewQtDelegateQuickWindow::updateCursor(const QCursor &cursor)
{
    setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateQuickWindow::resize(int width, int height)
{
    QQuickWindow::resize(width, height);
    m_realDelegate->resize(width, height);
}

void RenderWidgetHostViewQtDelegateQuickWindow::move(const QPoint &screenPos)
{
    QRectF popupRect(screenPos, size());
    popupRect = mapRectFromGlobal(m_virtualParent, popupRect);
    popupRect = m_virtualParent->mapRectToScene(popupRect);
    popupRect = mapRectToGlobal(m_virtualParent, popupRect);

    QQuickWindow::setPosition(popupRect.topLeft().toPoint());
}

} // namespace QtWebEngineCore
