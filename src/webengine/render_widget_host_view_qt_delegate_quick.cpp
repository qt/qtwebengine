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

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"
#include <QGuiApplication>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QVariant>
#include <QWindow>

RenderWidgetHostViewQtDelegateQuick::RenderWidgetHostViewQtDelegateQuick(RenderWidgetHostViewQtDelegateClient *client, bool isPopup, QQuickWebEngineView *view)
    : m_client(client)
    , m_view(view)
    , m_isPopup(isPopup)
    , m_initialized(false)
    , m_isVisible(true)
{
}

void RenderWidgetHostViewQtDelegateQuick::initAsChild(WebContentsAdapterClient* container)
{
    QQuickWebEngineViewPrivate *viewPrivate = static_cast<QQuickWebEngineViewPrivate *>(container);
    viewPrivate->q_func()->setDelegate(this);
    m_initialized = true;
}

void RenderWidgetHostViewQtDelegateQuick::initAsPopup(const QRect &r)
{
    m_initialized = true;
}

QRectF RenderWidgetHostViewQtDelegateQuick::screenRect() const
{
    QPointF pos = m_view->mapToScene(QPointF(0,0));
    return QRectF(pos.x(), pos.y(), m_view->width(), m_view->height());
}

QRectF RenderWidgetHostViewQtDelegateQuick::contentsRect() const
{
    QPointF scenePoint = m_view->mapToScene(QPointF(0, 0));
    QPointF screenPos = window()->mapToGlobal(scenePoint.toPoint());
    return QRectF(screenPos.x(), screenPos.y(), m_view->width(), m_view->height());
}

void RenderWidgetHostViewQtDelegateQuick::setKeyboardFocus()
{
    m_view->setFocus(true);
}

bool RenderWidgetHostViewQtDelegateQuick::hasKeyboardFocus()
{
    return m_view->hasFocus();
}

void RenderWidgetHostViewQtDelegateQuick::show()
{
    m_isVisible = true;
    update();
}

void RenderWidgetHostViewQtDelegateQuick::hide()
{
    m_isVisible = false;
    update();
}

bool RenderWidgetHostViewQtDelegateQuick::isVisible() const
{
    return m_view->isVisible();
}

QWindow* RenderWidgetHostViewQtDelegateQuick::window() const
{
    return m_view->window();
}

void RenderWidgetHostViewQtDelegateQuick::update()
{
    m_view->update();
}

void RenderWidgetHostViewQtDelegateQuick::updateCursor(const QCursor &cursor)
{
    m_view->setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateQuick::resize(int width, int height)
{
}

void RenderWidgetHostViewQtDelegateQuick::inputMethodStateChanged(bool editorVisible)
{
    if (qApp->inputMethod()->isVisible() == editorVisible)
        return;

    m_view->setFlag(QQuickItem::ItemAcceptsInputMethod, editorVisible);
    qApp->inputMethod()->update(Qt::ImQueryInput | Qt::ImEnabled | Qt::ImHints);
    qApp->inputMethod()->setVisible(editorVisible);
}

bool RenderWidgetHostViewQtDelegateQuick::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FocusIn:
        setKeyboardFocus();
        break;
    case QEvent::MouseButtonPress:
        if (!m_isPopup) m_view->forceActiveFocus();
        break;
    default: break;
    }
    return m_client->forwardEvent(event);
}



QVariant RenderWidgetHostViewQtDelegateQuick::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_client->inputMethodQuery(query);
}

void RenderWidgetHostViewQtDelegateQuick::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    m_client->notifyResize();
}

void RenderWidgetHostViewQtDelegateQuick::windowChanged()
{
    if (m_client)
        m_client->windowChanged();
}

QSGNode *RenderWidgetHostViewQtDelegateQuick::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    if (m_view)
        return m_client->updatePaintNode(oldNode, QQuickWindowPrivate::get(m_view->window())->context);
    return 0;
}

void RenderWidgetHostViewQtDelegateQuick::setView(QQuickWebEngineView* view)
{
    m_view = view;
}
