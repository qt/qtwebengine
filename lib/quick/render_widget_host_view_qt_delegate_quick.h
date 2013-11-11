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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H

#include "render_widget_host_view_qt_delegate.h"

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"
#include <QGuiApplication>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QVariant>
#include <QWindow>

template<typename ItemBaseT>
class RenderWidgetHostViewQtDelegateQuickBase : public ItemBaseT, public RenderWidgetHostViewQtDelegate
{
public:
    RenderWidgetHostViewQtDelegateQuickBase(RenderWidgetHostViewQtDelegateClient *client, QQuickItem *parent = 0)
        : ItemBaseT(parent)
        , m_client(client)
    {
        this->setAcceptedMouseButtons(Qt::AllButtons);
        this->setAcceptHoverEvents(true);
    }

    virtual void initAsChild(WebContentsAdapterClient* container)
    {
        QQuickWebEngineViewPrivate *viewPrivate = static_cast<QQuickWebEngineViewPrivate *>(container);
        this->setParentItem(viewPrivate->q_func());
    }

    virtual void initAsPopup(const QRect& rect)
    {
        this->setX(rect.x());
        this->setY(rect.y());
        this->setWidth(rect.width());
        this->setHeight(rect.height());
        this->setVisible(true);
    }

    virtual QRectF screenRect() const
    {
        QPointF pos = this->mapToScene(QPointF(0,0));
        return QRectF(pos.x(), pos.y(), this->width(), this->height());
    }

    virtual void setKeyboardFocus()
    {
        this->setFocus(true);
    }

    virtual bool hasKeyboardFocus()
    {
        return this->hasFocus();
    }

    virtual void show()
    {
        this->setVisible(true);
    }

    virtual void hide()
    {
        this->setVisible(false);
    }

    virtual bool isVisible() const
    {
        return ItemBaseT::isVisible();
    }

    virtual QWindow* window() const
    {
        return ItemBaseT::window();
    }

    virtual void updateCursor(const QCursor &cursor)
    {
        this->setCursor(cursor);
    }

    virtual void resize(int width, int height)
    {
        this->setSize(QSizeF(width, height));
    }

    void focusInEvent(QFocusEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void focusOutEvent(QFocusEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void mousePressEvent(QMouseEvent *event)
    {
        this->setFocus(true);
        m_client->forwardEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void keyPressEvent(QKeyEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void keyReleaseEvent(QKeyEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void wheelEvent(QWheelEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void touchEvent(QTouchEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void hoverMoveEvent(QHoverEvent *event)
    {
        m_client->forwardEvent(event);
    }

    void inputMethodStateChanged(bool editorVisible)
    {
        if (qApp->inputMethod()->isVisible() == editorVisible)
            return;

        this->setFlag(QQuickItem::ItemAcceptsInputMethod, editorVisible);
        qApp->inputMethod()->update(Qt::ImQueryInput | Qt::ImEnabled | Qt::ImHints);
        qApp->inputMethod()->setVisible(editorVisible);
    }

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const
    {
        return m_client->inputMethodQuery(query);
    }

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
    {
        ItemBaseT::geometryChanged(newGeometry, oldGeometry);
        m_client->notifyResize();
    }

    RenderWidgetHostViewQtDelegateClient *m_client;
};

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
class RenderWidgetHostViewQtDelegateQuick : public RenderWidgetHostViewQtDelegateQuickBase<QQuickItem>
{
    Q_OBJECT
public:
    RenderWidgetHostViewQtDelegateQuick(RenderWidgetHostViewQtDelegateClient *client, QQuickItem *parent = 0);

    virtual WId nativeWindowIdForCompositor() const;
    virtual void update(const QRect& rect = QRect());
    virtual QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
};
#endif // QT_VERSION

class RenderWidgetHostViewQtDelegateQuickPainted : public RenderWidgetHostViewQtDelegateQuickBase<QQuickPaintedItem>
{
    Q_OBJECT
public:
    RenderWidgetHostViewQtDelegateQuickPainted(RenderWidgetHostViewQtDelegateClient *client, QQuickItem *parent = 0);

    virtual WId nativeWindowIdForCompositor() const;
    virtual void update(const QRect& rect = QRect());

    void paint(QPainter *painter);

protected:
    void updatePolish();
};

#endif
