// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "render_widget_host_view_qt_delegate_quickwindow.h"

#include "api/qquickwebengineview_p_p.h"

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
        RenderWidgetHostViewQtDelegateItem *realDelegate, QWindow *parent)
    : QQuickWindow(parent), m_realDelegate(realDelegate), m_virtualParent(nullptr), m_rotated(false)
{
    setFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    realDelegate->setParentItem(contentItem());
}

RenderWidgetHostViewQtDelegateQuickWindow::~RenderWidgetHostViewQtDelegateQuickWindow()
{
    if (m_realDelegate) {
        m_realDelegate->setWidgetDelegate(nullptr);
        m_realDelegate->setParentItem(nullptr);
    }
}

void RenderWidgetHostViewQtDelegateQuickWindow::setVirtualParent(QQuickItem *virtualParent)
{
    Q_ASSERT(virtualParent);
    m_virtualParent = virtualParent;
}

// rect is window geometry in form of parent window offset + offset in scene coordinates
// chromium knows nothing about local transformation
void RenderWidgetHostViewQtDelegateQuickWindow::InitAsPopup(const QRect &rect)
{
    m_rotated = m_virtualParent->rotation() > 0 || m_virtualParent->parentItem()->rotation() > 0;
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
        QRect geometry(rect);
        geometry.moveTo(rect.topLeft() - getOffset(m_virtualParent));
        setGeometry(geometry);
    }
    m_realDelegate->show();
    raise();
    show();
}

void RenderWidgetHostViewQtDelegateQuickWindow::Resize(int width, int height)
{
    if (!m_rotated)
        QQuickWindow::resize(width, height);
}

void RenderWidgetHostViewQtDelegateQuickWindow::MoveWindow(const QPoint &screenPos)
{
    if (!m_rotated)
        QQuickWindow::setPosition(screenPos - getOffset(m_virtualParent));
}

void RenderWidgetHostViewQtDelegateQuickWindow::SetClearColor(const QColor &color)
{
    QQuickWindow::setColor(color);
}

bool RenderWidgetHostViewQtDelegateQuickWindow::ActiveFocusOnPress()
{
    return false;
}

void RenderWidgetHostViewQtDelegateQuickWindow::Bind(QtWebEngineCore::WebContentsAdapterClient *client)
{
    QQuickWebEngineViewPrivate::bindViewAndDelegateItem(
            static_cast<QQuickWebEngineViewPrivate *>(client), m_realDelegate.data());
}

void RenderWidgetHostViewQtDelegateQuickWindow::Unbind()
{
    QQuickWebEngineViewPrivate::bindViewAndDelegateItem(nullptr, m_realDelegate.data());
}

void RenderWidgetHostViewQtDelegateQuickWindow::Destroy()
{
    deleteLater();
}

} // namespace QtWebEngineCore
