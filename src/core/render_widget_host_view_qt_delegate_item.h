/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_ITEM_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_ITEM_H

#include "compositor/compositor.h"
#include "render_widget_host_view_qt_delegate.h"

#include <QtQuick/QQuickItem>

QT_BEGIN_NAMESPACE
class QQuickWebEngineView;
class QQuickWebEngineViewPrivate;
class QWebEnginePage;
class QWebEngineViewPrivate;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegateClient;
class WebContentsAdapterClient;
class WebEngineQuickWidget;

class WidgetDelegate
{
public:
    virtual ~WidgetDelegate() = default;
    virtual void InitAsPopup(const QRect &screenRect) = 0;
    virtual void SetInputMethodEnabled(bool) { }
    virtual void SetInputMethodHints(Qt::InputMethodHints) { }
    virtual void SetClearColor(const QColor &)  { }
    virtual bool ActiveFocusOnPress() = 0;
    virtual void MoveWindow(const QPoint & ) { }
    virtual void Bind(WebContentsAdapterClient *) = 0;
    virtual void Unbind() = 0;
    virtual void Destroy() = 0;
    virtual void Resize(int, int) { }
    virtual QWindow *Window() { return nullptr; }
};

// Useful information keyboard and mouse QEvent propagation.
// A RenderWidgetHostViewQtDelegateItem instance initialized as a popup will receive
// no keyboard focus (so all keyboard QEvents will be sent to the parent RWHVQD instance),
// but will still receive mouse input (all mouse QEvent moves and clicks will be given to the popup
// RWHVQD instance, and the mouse interaction area covers the surface of the whole parent
// QWebEngineView, and not only the smaller surface that an HTML select popup would occupy).
class Q_WEBENGINECORE_PRIVATE_EXPORT RenderWidgetHostViewQtDelegateItem
        : public QQuickItem
        , public RenderWidgetHostViewQtDelegate
        , public Compositor::Observer
{
    Q_OBJECT
public:
    RenderWidgetHostViewQtDelegateItem(RenderWidgetHostViewQtDelegateClient *client, bool isPopup);
    ~RenderWidgetHostViewQtDelegateItem();

    void initAsPopup(const QRect&) override;
    QRectF viewGeometry() const override;
    QRect windowGeometry() const override;
    void setKeyboardFocus() override;
    bool hasKeyboardFocus() override;
    void lockMouse() override;
    void unlockMouse() override;
    void show() override;
    void hide() override;
    bool isVisible() const override;
    QWindow *Window() const override;
    void updateCursor(const QCursor &) override;
    void resize(int width, int height) override;
    void move(const QPoint &screenPos) override;
    void inputMethodStateChanged(bool editorVisible, bool passwordInput) override;
    void setInputMethodHints(Qt::InputMethodHints) override;
    void setClearColor(const QColor &color) override;
    void unhandledWheelEvent(QWheelEvent *ev) override;

    void readyToSwap() override;

    void setWidgetDelegate(WidgetDelegate *delegate);

protected:
    bool event(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void touchEvent(QTouchEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

    void adapterClientChanged(WebContentsAdapterClient *client) override;

private Q_SLOTS:
    void onBeforeRendering();
    void onWindowPosChanged();
    void onHide();

private:
    friend QWebEngineViewPrivate;
    friend QQuickWebEngineViewPrivate;
    friend WebEngineQuickWidget;

    RenderWidgetHostViewQtDelegateClient *m_client;
    bool m_isPopup;
    QColor m_clearColor;
    Qt::InputMethodHints m_inputMethodHints = {};
    QList<QMetaObject::Connection> m_windowConnections;
    WebContentsAdapterClient *m_adapterClient = nullptr;
    QWebEnginePage *m_page = nullptr;
    QQuickWebEngineView *m_view = nullptr;
    WidgetDelegate *m_widgetDelegate = nullptr;
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_ITEM_H
