// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "render_widget_host_view_qt_delegate_quickwindow_p.h"

#include "api/qquickwebengineview_p_p.h"

namespace QtWebEngineCore {

struct ItemTransform {
    qreal rotation = 0.;
    qreal scale = 1.;
};

// Helper function to calculate the cumulative rotation and scale.
static inline struct ItemTransform getTransformValuesFromItemTree(QQuickItem *item)
{
    struct ItemTransform returnValue;

    while (item) {
        returnValue.rotation += item->rotation();
        returnValue.scale *= item->scale();
        item = item->parentItem();
    }

    return returnValue;
}

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
    : QQuickWindow(parent), m_realDelegate(realDelegate), m_virtualParent(nullptr), m_transformed(false)
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
    // To decide if there is a scale or rotation, we check it from the transfrom
    // to also cover the case where the scale is higher up in the item tree.
    QTransform transform = m_virtualParent->itemTransform(nullptr, nullptr);
    m_transformed = transform.isRotating() || transform.isScaling();

    if (m_transformed) {
        // code below tries to cover the case where webengine view is rotated or scaled,
        // the code assumes the rotation is in the form of  90, 180, 270 degrees
        // to archive that we keep chromium unaware of transformation and we transform
        // just the window content.
        m_rect = rect;
        // get parent window (scene) offset
        QPointF offset = m_virtualParent->mapFromScene(QPoint(0, 0));
        offset = m_virtualParent->mapToGlobal(offset);
        // get local transform
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

        // We need to read the values for scale and rotation from the item tree as it is not
        // sufficient to only use the virtual parent item and its parent for the case that the
        // scale or rotation is applied higher up the item tree.
        struct ItemTransform transformValues = getTransformValuesFromItemTree(m_virtualParent);
        m_realDelegate->setRotation(transformValues.rotation);
        m_realDelegate->setScale(transformValues.scale);
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
    if (!m_transformed)
        QQuickWindow::resize(width, height);
}

void RenderWidgetHostViewQtDelegateQuickWindow::MoveWindow(const QPoint &screenPos)
{
    if (!m_transformed)
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
