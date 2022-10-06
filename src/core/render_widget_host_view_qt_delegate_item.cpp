// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "render_widget_host_view_qt_delegate_item.h"

#include "render_widget_host_view_qt_delegate_client.h"

#include <QGuiApplication>
#include <QMouseEvent>
#include <QSGImageNode>
#include <QWindow>

namespace QtWebEngineCore {

RenderWidgetHostViewQtDelegateItem::RenderWidgetHostViewQtDelegateItem(RenderWidgetHostViewQtDelegateClient *client, bool isPopup)
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

RenderWidgetHostViewQtDelegateItem::~RenderWidgetHostViewQtDelegateItem()
{
    if (m_widgetDelegate) {
        m_widgetDelegate->Unbind();
        m_widgetDelegate->Destroy();
    }
}

void RenderWidgetHostViewQtDelegateItem::initAsPopup(const QRect &screenRect)
{
    Q_ASSERT(m_isPopup);
    setSize(screenRect.size());
    if (m_widgetDelegate)
        m_widgetDelegate->InitAsPopup(screenRect);
}

QRectF RenderWidgetHostViewQtDelegateItem::viewGeometry() const
{
    // Transform the entire rect to find the correct top left corner.
    const QPointF p1 = mapToGlobal(mapFromScene(QPointF(0, 0)));
    const QPointF p2 = mapToGlobal(mapFromScene(QPointF(width(), height())));
    QRectF geometry = QRectF(p1, p2).normalized();
    // But keep the size untransformed to behave like other QQuickItems.
    geometry.setSize(size());
    return geometry;
}

QRect RenderWidgetHostViewQtDelegateItem::windowGeometry() const
{
    if (!Window())
        return QRect();
    return Window()->frameGeometry();
}

void RenderWidgetHostViewQtDelegateItem::setKeyboardFocus()
{
    setFocus(true);
}

bool RenderWidgetHostViewQtDelegateItem::hasKeyboardFocus()
{
    return hasActiveFocus();
}

void RenderWidgetHostViewQtDelegateItem::lockMouse()
{
    grabMouse();
}

void RenderWidgetHostViewQtDelegateItem::unlockMouse()
{
    ungrabMouse();
}

void RenderWidgetHostViewQtDelegateItem::show()
{
    if (isVisible())
        m_client->notifyShown();
    else
        setVisible(true);
}

void RenderWidgetHostViewQtDelegateItem::hide()
{
    if (isVisible())
        setVisible(false);
    else
        m_client->notifyHidden();
}

bool RenderWidgetHostViewQtDelegateItem::isVisible() const
{
    return QQuickItem::isVisible();
}

QWindow *RenderWidgetHostViewQtDelegateItem::Window() const
{
    if (m_widgetDelegate) {
        if (auto *window = m_widgetDelegate->Window())
            return window;
    }
    return QQuickItem::window();
}

void RenderWidgetHostViewQtDelegateItem::readyToSwap()
{
    // Call update() on UI thread.
    QMetaObject::invokeMethod(this, &QQuickItem::update, Qt::QueuedConnection);
}

void RenderWidgetHostViewQtDelegateItem::updateCursor(const QCursor &cursor)
{
    setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateItem::resize(int width, int height)
{
    setSize(QSizeF(width, height));
    if (m_widgetDelegate)
        m_widgetDelegate->Resize(width, height);
}

void RenderWidgetHostViewQtDelegateItem::move(const QPoint &point)
{
    if (m_widgetDelegate && m_isPopup)
        m_widgetDelegate->MoveWindow(point);
}

void RenderWidgetHostViewQtDelegateItem::inputMethodStateChanged(bool editorVisible, bool passwordInput)
{
    setFlag(QQuickItem::ItemAcceptsInputMethod, editorVisible && !passwordInput);

    if (parentItem())
        parentItem()->setFlag(QQuickItem::ItemAcceptsInputMethod, editorVisible && !passwordInput);

    if (m_widgetDelegate)
        m_widgetDelegate->SetInputMethodEnabled(editorVisible && !passwordInput);

    qApp->inputMethod()->update(Qt::ImQueryInput | Qt::ImEnabled | Qt::ImHints);
    if (qApp->inputMethod()->isVisible() != editorVisible)
        qApp->inputMethod()->setVisible(editorVisible);
}

void RenderWidgetHostViewQtDelegateItem::setWidgetDelegate(WidgetDelegate *delegate)
{
    Q_ASSERT(!m_widgetDelegate || !delegate);
    m_widgetDelegate = delegate;
    if (m_widgetDelegate) {
        if (m_inputMethodHints)
            m_widgetDelegate->SetInputMethodHints(m_inputMethodHints);
        if (m_clearColor.isValid())
            m_widgetDelegate->SetClearColor(m_clearColor);
        if (flags().testFlag(QQuickItem::ItemAcceptsInputMethod))
            m_widgetDelegate->SetInputMethodEnabled(true);
        if (m_adapterClient)
            m_widgetDelegate->Bind(m_adapterClient);
    }
}

void RenderWidgetHostViewQtDelegateItem::setInputMethodHints(Qt::InputMethodHints hints)
{
    m_inputMethodHints = hints;
    if (m_widgetDelegate)
        m_widgetDelegate->SetInputMethodHints(hints);
}

void RenderWidgetHostViewQtDelegateItem::setClearColor(const QColor &color)
{
    m_clearColor = color;
    if (m_widgetDelegate)
        m_widgetDelegate->SetClearColor(color);
}

bool RenderWidgetHostViewQtDelegateItem::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
        return m_client->forwardEvent(event);

#if QT_CONFIG(gestures)
    if (event->type() == QEvent::NativeGesture)
        return m_client->forwardEvent(event);
#endif

    return QQuickItem::event(event);
}

void RenderWidgetHostViewQtDelegateItem::focusInEvent(QFocusEvent *event)
{
#if QT_CONFIG(accessibility)
    if (QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(this)) {
        if (auto *focusChild = iface->focusChild()) {
            QAccessibleEvent focusEvent(focusChild, QAccessible::Focus);
            QAccessible::updateAccessibility(&focusEvent);
        }
    }
#endif // QT_CONFIG(accessibility)

    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::focusOutEvent(QFocusEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::mousePressEvent(QMouseEvent *event)
{
    if (!m_isPopup && m_widgetDelegate) {
        if (m_widgetDelegate->ActiveFocusOnPress()) {
            forceActiveFocus();
        } else {
            event->ignore();
            return;
        }
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isPopup && m_widgetDelegate && !m_widgetDelegate->ActiveFocusOnPress()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_isPopup && m_widgetDelegate && !m_widgetDelegate->ActiveFocusOnPress()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::keyPressEvent(QKeyEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::keyReleaseEvent(QKeyEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::wheelEvent(QWheelEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::touchEvent(QTouchEvent *event)
{
    if (!m_isPopup && m_widgetDelegate) {
        if (event->type() == QEvent::TouchBegin && m_widgetDelegate->ActiveFocusOnPress())
            forceActiveFocus();

        if (!m_widgetDelegate->ActiveFocusOnPress()) {
            event->ignore();
            return;
        }
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::hoverMoveEvent(QHoverEvent *event)
{
    if ((!m_isPopup && m_widgetDelegate && !m_widgetDelegate->ActiveFocusOnPress())
        || event->position() == event->oldPosF()) {
        event->ignore();
        return;
    }
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::hoverLeaveEvent(QHoverEvent *event)
{
    m_client->forwardEvent(event);
}

QVariant RenderWidgetHostViewQtDelegateItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_client->inputMethodQuery(query);
}

void RenderWidgetHostViewQtDelegateItem::inputMethodEvent(QInputMethodEvent *event)
{
    m_client->forwardEvent(event);
}

void RenderWidgetHostViewQtDelegateItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    m_client->visualPropertiesChanged();
}

void RenderWidgetHostViewQtDelegateItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    if (change == QQuickItem::ItemSceneChange) {
        for (const QMetaObject::Connection &c : std::as_const(m_windowConnections))
            disconnect(c);
        m_windowConnections.clear();
        if (value.window) {
            m_windowConnections.append(connect(value.window, &QQuickWindow::beforeRendering,
                                               this, &RenderWidgetHostViewQtDelegateItem::onBeforeRendering, Qt::DirectConnection));
            m_windowConnections.append(connect(value.window, SIGNAL(xChanged(int)), SLOT(onWindowPosChanged())));
            m_windowConnections.append(connect(value.window, SIGNAL(yChanged(int)), SLOT(onWindowPosChanged())));
            if (!m_isPopup)
                m_windowConnections.append(connect(value.window, SIGNAL(closing(QQuickCloseEvent *)), SLOT(onHide())));
        }
        m_client->visualPropertiesChanged();
    } else if (change == QQuickItem::ItemVisibleHasChanged) {
        if (value.boolValue) {
            m_client->notifyShown();
        } else {
            m_client->notifyHidden();
            if (!m_isPopup)
                onHide();
        }
    }
}

QSGNode *RenderWidgetHostViewQtDelegateItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
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
#if QT_CONFIG(opengl)
    } else if (comp->type() == Compositor::Type::OpenGL) {
        QQuickWindow::CreateTextureOptions texOpts;
        if (comp->hasAlphaChannel())
            texOpts.setFlag(QQuickWindow::TextureHasAlphaChannel);
        int texId = comp->textureId();
        node->setTexture(QNativeInterface::QSGOpenGLTexture::fromNative(texId, win, texSize, texOpts));
        node->setTextureCoordinatesTransform(QSGImageNode::MirrorVertically);
#endif
    } else {
        Q_UNREACHABLE();
    }

    return node;
}

void RenderWidgetHostViewQtDelegateItem::onBeforeRendering()
{
    auto comp = compositor();
    if (!comp || comp->type() != Compositor::Type::OpenGL)
        return;
    comp->waitForTexture();
}

void RenderWidgetHostViewQtDelegateItem::onWindowPosChanged()
{
    m_client->visualPropertiesChanged();
}

void RenderWidgetHostViewQtDelegateItem::onHide()
{
    QFocusEvent event(QEvent::FocusOut, Qt::OtherFocusReason);
    m_client->forwardEvent(&event);
}

void RenderWidgetHostViewQtDelegateItem::adapterClientChanged(WebContentsAdapterClient *client)
{
    m_adapterClient = client;
    if (m_widgetDelegate)
        m_widgetDelegate->Bind(client);
}

void RenderWidgetHostViewQtDelegateItem::updateAdapterClientIfNeeded(WebContentsAdapterClient *client)
{
    if (client == m_adapterClient)
        return;
    adapterClientChanged(client);
}

void RenderWidgetHostViewQtDelegateItem::unhandledWheelEvent(QWheelEvent *ev)
{
    if (QWindow *w = Window()) {
        if (QWindow *p = w->parent())
            qApp->sendEvent(p, ev);
    }
}

} // namespace QtWebEngineCore
