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

#include "render_widget_host_view_qt_delegate_client.h"

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"

#include <QtCore/qvariant.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qwindow.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qsgimagenode.h>

namespace QtWebEngineCore {

RenderWidgetHostViewQtDelegateQuick::RenderWidgetHostViewQtDelegateQuick(RenderWidgetHostViewQtDelegateClient *client, bool isPopup)
    : m_client(client)
    , m_isPopup(isPopup)
{
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    if (!isPopup) {
        setFocus(true);
        setActiveFocusOnTab(true);
    }
    bind(client->compositorId());
}

RenderWidgetHostViewQtDelegateQuick::~RenderWidgetHostViewQtDelegateQuick()
{
    QQuickWebEngineViewPrivate::bindViewAndWidget(nullptr, this);
}

void RenderWidgetHostViewQtDelegateQuick::initAsPopup(const QRect &screenRect)
{
    //note this is called when there is no windowing system
    //otherwsie see RenderWidgetHostViewQtDelegateQuickWindow
    Q_ASSERT(m_isPopup && parentItem());
    setPosition(screenRect.topLeft());
    setSize(screenRect.size());
    setVisible(true);
}

QRectF RenderWidgetHostViewQtDelegateQuick::viewGeometry() const
{
    // Transform the entire rect to find the correct top left corner.
    const QPointF p1 = mapToGlobal(mapFromScene(QPointF(0, 0)));
    const QPointF p2 = mapToGlobal(mapFromScene(QPointF(width(), height())));
    QRectF geometry = QRectF(p1, p2).normalized();
    // But keep the size untransformed to behave like other QQuickItems.
    geometry.setSize(size());
    return geometry;
}

QRect RenderWidgetHostViewQtDelegateQuick::windowGeometry() const
{
    if (!window())
        return QRect();
    return window()->frameGeometry();
}

void RenderWidgetHostViewQtDelegateQuick::setKeyboardFocus()
{
    setFocus(true);
}

bool RenderWidgetHostViewQtDelegateQuick::hasKeyboardFocus()
{
    return hasActiveFocus();
}

void RenderWidgetHostViewQtDelegateQuick::lockMouse()
{
    grabMouse();
}

void RenderWidgetHostViewQtDelegateQuick::unlockMouse()
{
    ungrabMouse();
}

void RenderWidgetHostViewQtDelegateQuick::show()
{
    setVisible(true);
    m_client->notifyShown();
}

void RenderWidgetHostViewQtDelegateQuick::hide()
{
    setVisible(false);
    m_client->notifyHidden();
}

bool RenderWidgetHostViewQtDelegateQuick::isVisible() const
{
    return QQuickItem::isVisible();
}

QWindow* RenderWidgetHostViewQtDelegateQuick::window() const
{
    return QQuickItem::window();
}

void RenderWidgetHostViewQtDelegateQuick::readyToSwap()
{
    // Call update() on UI thread.
    QMetaObject::invokeMethod(this, &QQuickItem::update, Qt::QueuedConnection);
}

void RenderWidgetHostViewQtDelegateQuick::updateCursor(const QCursor &cursor)
{
    setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateQuick::resize(int width, int height)
{
    setSize(QSizeF(width, height));
}

void RenderWidgetHostViewQtDelegateQuick::inputMethodStateChanged(bool editorVisible, bool passwordInput)
{
    setFlag(QQuickItem::ItemAcceptsInputMethod, editorVisible && !passwordInput);

    if (parentItem())
        parentItem()->setFlag(QQuickItem::ItemAcceptsInputMethod, editorVisible && !passwordInput);

    qApp->inputMethod()->update(Qt::ImQueryInput | Qt::ImEnabled | Qt::ImHints);
    if (qApp->inputMethod()->isVisible() != editorVisible)
        qApp->inputMethod()->setVisible(editorVisible);
}

bool RenderWidgetHostViewQtDelegateQuick::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
        return m_client->forwardEvent(event);

#ifndef QT_NO_GESTURES
    if (event->type() == QEvent::NativeGesture)
        return m_client->forwardEvent(event);
#endif

    return QQuickItem::event(event);
}

void RenderWidgetHostViewQtDelegateQuick::focusInEvent(QFocusEvent *event)
{
#if QT_CONFIG(accessibility)
    if (QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(this)->focusChild()) {
        QAccessibleEvent focusEvent(iface, QAccessible::Focus);
        QAccessible::updateAccessibility(&focusEvent);
    }
#endif // QT_CONFIG(accessibility)

    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::focusOutEvent(QFocusEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mousePressEvent(QMouseEvent *event)
{
    QQuickItem *parent = parentItem();
    if (!m_isPopup && (parent && parent->property("activeFocusOnPress").toBool()))
        forceActiveFocus();
    if (!m_isPopup && parent && !parent->property("activeFocusOnPress").toBool() && !parent->hasActiveFocus()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseMoveEvent(QMouseEvent *event)
{
    QQuickItem *parent = parentItem();
    if (parent && !parent->property("activeFocusOnPress").toBool() && !parent->hasActiveFocus()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::mouseReleaseEvent(QMouseEvent *event)
{
    QQuickItem *parent = parentItem();
    if (!m_isPopup && parent && !parent->property("activeFocusOnPress").toBool() && !parent->hasActiveFocus()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::keyPressEvent(QKeyEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::keyReleaseEvent(QKeyEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::wheelEvent(QWheelEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::touchEvent(QTouchEvent *event)
{
    QQuickItem *parent = parentItem();
    if (event->type() == QEvent::TouchBegin && !m_isPopup
            && (parent && parent->property("activeFocusOnPress").toBool()))
        forceActiveFocus();
    if (parent && !parent->property("activeFocusOnPress").toBool() && !parent->hasActiveFocus()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::hoverMoveEvent(QHoverEvent *event)
{
    QQuickItem *parent = parentItem();
    if ((!m_isPopup && parent && !parent->property("activeFocusOnPress").toBool()
         && !parent->hasActiveFocus())
        || event->position() == event->oldPosF()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::hoverLeaveEvent(QHoverEvent *event)
{
    m_client->forwardEvent(event);
}

QVariant RenderWidgetHostViewQtDelegateQuick::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_client->inputMethodQuery(query);
}

void RenderWidgetHostViewQtDelegateQuick::inputMethodEvent(QInputMethodEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateQuick::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    m_client->visualPropertiesChanged();
}

void RenderWidgetHostViewQtDelegateQuick::itemChange(ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    if (change == QQuickItem::ItemSceneChange) {
        for (const QMetaObject::Connection &c : qAsConst(m_windowConnections))
            disconnect(c);
        m_windowConnections.clear();
        if (value.window) {
            m_windowConnections.append(connect(value.window, SIGNAL(beforeRendering()),
                                               SLOT(onBeforeRendering()), Qt::DirectConnection));
            m_windowConnections.append(connect(value.window, SIGNAL(xChanged(int)), SLOT(onWindowPosChanged())));
            m_windowConnections.append(connect(value.window, SIGNAL(yChanged(int)), SLOT(onWindowPosChanged())));
            if (!m_isPopup)
                m_windowConnections.append(connect(value.window, SIGNAL(closing(QQuickCloseEvent *)), SLOT(onHide())));
        }
        m_client->visualPropertiesChanged();
    } else if (change == QQuickItem::ItemVisibleHasChanged) {
        if (!m_isPopup && !value.boolValue)
            onHide();
    }
}

QSGNode *RenderWidgetHostViewQtDelegateQuick::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto comp = compositor();
    if (!comp)
        return nullptr;

    QQuickWindow *win = QQuickItem::window();

    // Delete old node before swapFrame to decrement refcount of
    // QImage in software mode.
    delete oldNode;
    QSGImageNode *node = win->createImageNode();
    node->setOwnsTexture(true);

    comp->swapFrame();

    QSize texSize = comp->size();
    QSizeF texSizeInDips = QSizeF(texSize) / comp->devicePixelRatio();
    node->setRect(QRectF(QPointF(0, 0), texSizeInDips));

    if (comp->type() == Compositor::Type::Software) {
        QImage image = comp->image();
        node->setTexture(win->createTextureFromImage(image));
    } else if (comp->type() == Compositor::Type::OpenGL) {
        QQuickWindow::CreateTextureOptions texOpts;
#if QT_CONFIG(opengl)
        if (comp->hasAlphaChannel())
            texOpts.setFlag(QQuickWindow::TextureHasAlphaChannel);
        int texId = comp->textureId();
        node->setTexture(QNativeInterface::QSGOpenGLTexture::fromNative(texId, win, texSize, texOpts));
        node->setTextureCoordinatesTransform(QSGImageNode::MirrorVertically);
#else
        Q_UNREACHABLE();
#endif
    } else {
        Q_UNREACHABLE();
    }

    return node;
}

void RenderWidgetHostViewQtDelegateQuick::onBeforeRendering()
{
    auto comp = compositor();
    if (!comp || comp->type() != Compositor::Type::OpenGL)
        return;
    comp->waitForTexture();
}

void RenderWidgetHostViewQtDelegateQuick::onWindowPosChanged()
{
    m_client->visualPropertiesChanged();
}

void RenderWidgetHostViewQtDelegateQuick::onHide()
{
    QFocusEvent event(QEvent::FocusOut, Qt::OtherFocusReason);
    m_client->forwardEvent(&event);
}

void RenderWidgetHostViewQtDelegateQuick::adapterClientChanged(WebContentsAdapterClient *client)
{
    QQuickWebEngineViewPrivate::bindViewAndWidget(
            static_cast<QQuickWebEngineViewPrivate *>(client)->q_func(), this);
}

#if QT_CONFIG(accessibility)
RenderWidgetHostViewQtDelegateQuickAccessible::RenderWidgetHostViewQtDelegateQuickAccessible(RenderWidgetHostViewQtDelegateQuick *o, QQuickWebEngineView *view)
    : QAccessibleObject(o)
    , m_view(view)
{
}

bool RenderWidgetHostViewQtDelegateQuickAccessible::isValid() const
{
    if (!viewAccessible() || !viewAccessible()->isValid())
        return false;

    return QAccessibleObject::isValid();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::parent() const
{
    return viewAccessible()->parent();
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::text(QAccessible::Text) const
{
    return QString();
}

QAccessible::Role RenderWidgetHostViewQtDelegateQuickAccessible::role() const
{
    return QAccessible::Client;
}

QAccessible::State RenderWidgetHostViewQtDelegateQuickAccessible::state() const
{
    return viewAccessible()->state();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::focusChild() const
{
    return viewAccessible()->focusChild();
}

int RenderWidgetHostViewQtDelegateQuickAccessible::childCount() const
{
    return viewAccessible()->childCount();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::child(int index) const
{
    return viewAccessible()->child(index);
}

int RenderWidgetHostViewQtDelegateQuickAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    return viewAccessible()->indexOfChild(c);
}

QQuickWebEngineViewAccessible *RenderWidgetHostViewQtDelegateQuickAccessible::viewAccessible() const
{
    return static_cast<QQuickWebEngineViewAccessible *>(QAccessible::queryAccessibleInterface(m_view));
}
#endif // QT_CONFIG(accessibility)

} // namespace QtWebEngineCore
