// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

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

RenderWidgetHostViewQtDelegateQuickWindow::RenderWidgetHostViewQtDelegateQuickWindow(
        RenderWidgetHostViewQtDelegateItem *realDelegate, QWindow *parent)
    : QQuickWindow(), m_realDelegate(realDelegate), m_virtualParent(nullptr), m_transformed(false)
{
    setFlags(Qt::Popup | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
             | Qt::WindowDoesNotAcceptFocus);
    realDelegate->setParentItem(contentItem());
    setTransientParent(parent);
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

// rect is visual geometry in form of global screen coordinates
// if menu is transformed screen rect is simply given in screen
// coordinates where parent geometry is simply QRect(0,0,size())
void RenderWidgetHostViewQtDelegateQuickWindow::InitAsPopup(const QRect &rect)
{
    // To decide if there is a scale or rotation, we check it from the transfrom
    // to also cover the case where the scale is higher up in the item tree.
    QTransform transform = m_virtualParent->itemTransform(nullptr, nullptr);
    m_transformed = transform.type() > QTransform::TxTranslate;

    if (m_transformed) {
        // code below tries to cover the case where webengine view is rotated or scaled,
        // the code assumes the rotation is in the form of  90, 180, 270 degrees
        // to archive that we keep chromium unaware of transformation and we transform
        // just the window content.
        QRectF popupRect = transform.mapRect(rect);
        // adjust for scene offset
        const QPointF offset =
                m_virtualParent->mapToGlobal(m_virtualParent->mapFromScene(QPoint(0, 0)));
        popupRect.translate(offset);
        setGeometry(popupRect.normalized().toRect());
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
        setGeometry(rect);
    }
    m_realDelegate->show();
    m_realDelegate->forceActiveFocus(Qt::FocusReason::PopupFocusReason);
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
    if (!m_transformed) {
        // Note we assume popup is frameless (no decorations), as screenPos is from
        // visual gemometry and not window gemetry, however here we set
        // positon of window frame.
        QQuickWindow::setPosition(screenPos);
    }
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
