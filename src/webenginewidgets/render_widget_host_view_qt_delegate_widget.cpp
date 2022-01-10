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

#include "render_widget_host_view_qt_delegate_widget.h"

#include "render_widget_host_view_qt_delegate_client.h"

#include <QtWebEngineCore/private/qwebenginepage_p.h>
#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include <QGuiApplication>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QSGImageNode>
#include <QWindow>

namespace QtWebEngineCore {

class RenderWidgetHostViewQuickItem : public QQuickItem, public Compositor::Observer
{
public:
    RenderWidgetHostViewQuickItem(RenderWidgetHostViewQtDelegateClient *client) : m_client(client)
    {
        setFlag(ItemHasContents, true);
        // Mark that this item should receive focus when the parent QQuickWidget receives focus.
        setFocus(true);

        bind(client->compositorId());
    }
    ~RenderWidgetHostViewQuickItem() { unbind(); }

protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::ShortcutOverride)
            return m_client->forwardEvent(event);

        return QQuickItem::event(event);
    }
    void focusInEvent(QFocusEvent *event) override
    {
        Q_ASSERT(event->reason() != Qt::PopupFocusReason);
        m_client->forwardEvent(event);
    }
    void focusOutEvent(QFocusEvent *event) override
    {
        // The keyboard events are supposed to go to the parent RenderHostView and WebUI
        // popups should never have focus, therefore ignore focusOutEvent as losing focus
        // will trigger pop close request from blink
        if (event->reason() != Qt::PopupFocusReason)
            m_client->forwardEvent(event);
    }
    void inputMethodEvent(QInputMethodEvent *event) override
    {
        m_client->forwardEvent(event);
    }
    void itemChange(ItemChange change, const ItemChangeData &value) override
    {
        QQuickItem::itemChange(change, value);
        if (change == QQuickItem::ItemSceneChange) {
            for (const QMetaObject::Connection &c : qAsConst(m_windowConnections))
                disconnect(c);
            m_windowConnections.clear();
            if (value.window) {
                m_windowConnections.append(connect(
                        value.window, &QQuickWindow::beforeRendering, this,
                        &RenderWidgetHostViewQuickItem::onBeforeRendering, Qt::DirectConnection));
            }
        }
    }
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override
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
#if QT_CONFIG(opengl)
            QQuickWindow::CreateTextureOptions texOpts;
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
    void onBeforeRendering()
    {
        auto comp = compositor();
        if (!comp || comp->type() != Compositor::Type::OpenGL)
            return;
        comp->waitForTexture();
    }
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override
    {
        return m_client->inputMethodQuery(query);
    }
    void readyToSwap() override
    {
        // Call update() on UI thread.
        QMetaObject::invokeMethod(this, &QQuickItem::update, Qt::QueuedConnection);
    }

private:
    RenderWidgetHostViewQtDelegateClient *m_client;
    QList<QMetaObject::Connection> m_windowConnections;
};

RenderWidgetHostViewQtDelegateWidget::RenderWidgetHostViewQtDelegateWidget(RenderWidgetHostViewQtDelegateClient *client, QWidget *parent)
    : QQuickWidget(parent)
    , m_client(client)
    , m_rootItem(new RenderWidgetHostViewQuickItem(client))
    , m_isPopup(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_AlwaysShowToolTips);

    setContent(QUrl(), nullptr, m_rootItem.data());

    connectRemoveParentBeforeParentDelete();
}

RenderWidgetHostViewQtDelegateWidget::~RenderWidgetHostViewQtDelegateWidget()
{
    QWebEngineViewPrivate::bindPageAndWidget(nullptr, this);
}

void RenderWidgetHostViewQtDelegateWidget::connectRemoveParentBeforeParentDelete()
{
    disconnect(m_parentDestroyedConnection);

    if (QWidget *parent = parentWidget()) {
        m_parentDestroyedConnection = connect(parent, &QObject::destroyed,
                                              this,
                                              &RenderWidgetHostViewQtDelegateWidget::removeParentBeforeParentDelete);
    } else {
        m_parentDestroyedConnection = QMetaObject::Connection();
    }
}

void RenderWidgetHostViewQtDelegateWidget::removeParentBeforeParentDelete()
{
    // Unset the parent, because parent is being destroyed, but the owner of this
    // RenderWidgetHostViewQtDelegateWidget is actually a RenderWidgetHostViewQt instance.
    setParent(nullptr);

    // If this widget represents a popup window, make sure to close it, so that if the popup was the
    // last visible top level window, the application event loop can quit if it deems it necessarry.
    if (m_isPopup)
        close();
}

void RenderWidgetHostViewQtDelegateWidget::initAsPopup(const QRect& screenRect)
{
    m_isPopup = true;

    // The keyboard events are supposed to go to the parent RenderHostView
    // so the WebUI popups should never have focus. Besides, if the parent view
    // loses focus, WebKit will cause its associated popups (including this one)
    // to be destroyed.
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    setGeometry(screenRect);
    show();
}

void RenderWidgetHostViewQtDelegateWidget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    // If a close event was received from the window manager (e.g. when moving the parent window,
    // clicking outside the popup area)
    // make sure to notify the Chromium WebUI popup and its underlying
    // RenderWidgetHostViewQtDelegate instance to be closed.
    if (m_isPopup)
        m_client->closePopup();
}

QRectF RenderWidgetHostViewQtDelegateWidget::viewGeometry() const
{
    return QRectF(mapToGlobal(pos()), size());
}

QRect RenderWidgetHostViewQtDelegateWidget::windowGeometry() const
{
    if (!window())
        return QRect();
    return window()->frameGeometry();
}

void RenderWidgetHostViewQtDelegateWidget::setKeyboardFocus()
{
    // The root item always has focus within the root focus scope:
    Q_ASSERT(m_rootItem->hasFocus());

    setFocus();
}

bool RenderWidgetHostViewQtDelegateWidget::hasKeyboardFocus()
{
    // The root item always has focus within the root focus scope:
    Q_ASSERT(m_rootItem->hasFocus());

    return hasFocus();
}

void RenderWidgetHostViewQtDelegateWidget::lockMouse()
{
    grabMouse();
}

void RenderWidgetHostViewQtDelegateWidget::unlockMouse()
{
    releaseMouse();
}

void RenderWidgetHostViewQtDelegateWidget::show()
{
    m_rootItem->setVisible(true);
    // Check if we're attached to a QWebEngineView, we don't
    // want to show anything else than popups as top-level.
    if (parent() || m_isPopup) {
        QQuickWidget::show();
    }
}

void RenderWidgetHostViewQtDelegateWidget::hide()
{
    m_rootItem->setVisible(false);
    QQuickWidget::hide();
}

bool RenderWidgetHostViewQtDelegateWidget::isVisible() const
{
    return QQuickWidget::isVisible() && m_rootItem->isVisible();
}

QWindow* RenderWidgetHostViewQtDelegateWidget::window() const
{
    const QWidget* root = QQuickWidget::window();
    return root ? root->windowHandle() : 0;
}

void RenderWidgetHostViewQtDelegateWidget::updateCursor(const QCursor &cursor)
{
    QQuickWidget::setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateWidget::resize(int width, int height)
{
    QQuickWidget::resize(width, height);
}

void RenderWidgetHostViewQtDelegateWidget::move(const QPoint &screenPos)
{
    Q_ASSERT(m_isPopup);
    QQuickWidget::move(screenPos);
}

void RenderWidgetHostViewQtDelegateWidget::inputMethodStateChanged(bool editorVisible, bool passwordInput)
{
    QQuickWidget::setAttribute(Qt::WA_InputMethodEnabled, editorVisible && !passwordInput);
    qApp->inputMethod()->update(Qt::ImQueryInput | Qt::ImEnabled | Qt::ImHints);
    if (qApp->inputMethod()->isVisible() != editorVisible)
        qApp->inputMethod()->setVisible(editorVisible);
}

void RenderWidgetHostViewQtDelegateWidget::setInputMethodHints(Qt::InputMethodHints hints)
{
    QQuickWidget::setInputMethodHints(hints);
}

void RenderWidgetHostViewQtDelegateWidget::setClearColor(const QColor &color)
{
    QQuickWidget::setClearColor(color);
    // QQuickWidget is usually blended by punching holes into widgets
    // above it to simulate the visual stacking order. If we want it to be
    // transparent we have to throw away the proper stacking order and always
    // blend the complete normal widgets backing store under it.
    bool isTranslucent = color.alpha() < 255;
    setAttribute(Qt::WA_AlwaysStackOnTop, isTranslucent);
    setAttribute(Qt::WA_OpaquePaintEvent, !isTranslucent);
    update();
}

QVariant RenderWidgetHostViewQtDelegateWidget::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_client->inputMethodQuery(query);
}

void RenderWidgetHostViewQtDelegateWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    QQuickWidget::resizeEvent(resizeEvent);
    m_client->visualPropertiesChanged();
}

void RenderWidgetHostViewQtDelegateWidget::showEvent(QShowEvent *event)
{
    QQuickWidget::showEvent(event);
    // We don't have a way to catch a top-level window change with QWidget
    // but a widget will most likely be shown again if it changes, so do
    // the reconnection at this point.
    for (const QMetaObject::Connection &c : qAsConst(m_windowConnections))
        disconnect(c);
    m_windowConnections.clear();
    if (QWindow *w = window()) {
        m_windowConnections.append(connect(w, SIGNAL(xChanged(int)), SLOT(onWindowPosChanged())));
        m_windowConnections.append(connect(w, SIGNAL(yChanged(int)), SLOT(onWindowPosChanged())));
    }
    m_client->visualPropertiesChanged();
    m_client->notifyShown();
}

void RenderWidgetHostViewQtDelegateWidget::hideEvent(QHideEvent *event)
{
    QQuickWidget::hideEvent(event);
    m_client->notifyHidden();
}

bool RenderWidgetHostViewQtDelegateWidget::event(QEvent *event)
{
    bool handled = false;

    // Track parent to make sure we don't get deleted.
    switch (event->type()) {
    case QEvent::ParentChange:
        connectRemoveParentBeforeParentDelete();
        break;
    default:
        break;
    }

    // Mimic QWidget::event() by ignoring mouse, keyboard, touch and tablet events if the widget is
    // disabled.
    if (!isEnabled()) {
        switch (event->type()) {
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TabletMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
        case QEvent::ContextMenu:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
#ifndef QT_NO_WHEELEVENT
        case QEvent::Wheel:
#endif
            return false;
        default:
            break;
        }
    }

    switch (event->type()) {
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        // We forward focus events later, once they have made it to the m_rootItem.
        return QQuickWidget::event(event);
    case QEvent::DragEnter:
    case QEvent::DragLeave:
    case QEvent::DragMove:
    case QEvent::Drop:
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
        // Let the parent handle these events.
        return false;
    default:
        break;
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
        // QWidget keeps the Qt4 behavior where the DblClick event would replace the Press event.
        // QtQuick is different by sending both the Press and DblClick events for the second press
        // where we can simply ignore the DblClick event.
        QMouseEvent *dblClick = static_cast<QMouseEvent *>(event);
        QMouseEvent press(QEvent::MouseButtonPress, dblClick->position(), dblClick->scenePosition(),
                          dblClick->globalPosition(), dblClick->button(), dblClick->buttons(),
                          dblClick->modifiers(), dblClick->source());
        press.setTimestamp(dblClick->timestamp());
        handled = m_client->forwardEvent(&press);
    } else
        handled = m_client->forwardEvent(event);

    if (!handled)
        return QQuickWidget::event(event);
    event->accept();
    return true;
}

void RenderWidgetHostViewQtDelegateWidget::unhandledWheelEvent(QWheelEvent *ev)
{
    if (QWidget *p = parentWidget())
        qApp->sendEvent(p, ev);
}

void RenderWidgetHostViewQtDelegateWidget::onWindowPosChanged()
{
    m_client->visualPropertiesChanged();
}

void RenderWidgetHostViewQtDelegateWidget::adapterClientChanged(WebContentsAdapterClient *client)
{
    if (m_pageDestroyedConnection)
        disconnect(m_pageDestroyedConnection);
    QWebEnginePage *page = static_cast<QWebEnginePagePrivate *>(client)->q_func();
    QWebEngineViewPrivate::bindPageAndWidget(page, this);
    m_pageDestroyedConnection = connect(page, &QWebEnginePage::_q_aboutToDelete, this,
            [this]() { QWebEngineViewPrivate::bindPageAndWidget(nullptr, this); });
}

#if QT_CONFIG(accessibility)
RenderWidgetHostViewQtDelegateWidgetAccessible::RenderWidgetHostViewQtDelegateWidgetAccessible(RenderWidgetHostViewQtDelegateWidget *o, QWebEngineView *view)
    : QAccessibleWidget(o)
    , m_view(view)
{
}

bool RenderWidgetHostViewQtDelegateWidgetAccessible::isValid() const
{
    if (!viewAccessible() || !viewAccessible()->isValid())
        return false;

    return QAccessibleWidget::isValid();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateWidgetAccessible::focusChild() const
{
    return viewAccessible()->focusChild();
}

int RenderWidgetHostViewQtDelegateWidgetAccessible::childCount() const
{
    return viewAccessible()->childCount();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateWidgetAccessible::child(int index) const
{
    return viewAccessible()->child(index);
}

int RenderWidgetHostViewQtDelegateWidgetAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    return viewAccessible()->indexOfChild(c);
}

QWebEngineViewAccessible *RenderWidgetHostViewQtDelegateWidgetAccessible::viewAccessible() const
{
    return static_cast<QWebEngineViewAccessible *>(QAccessible::queryAccessibleInterface(m_view));
}
#endif // QT_CONFIG(accessibility)

} // namespace QtWebEngineCore
