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

#include "render_widget_host_view_qt_delegate_popup.h"

#include "qwebengineview.h"
#include "qwebenginepage_p.h"
#include <QtGlobal>
#include <QLayout>
#include <QResizeEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QWindow>
#include <QtWidgets/QApplication>

RenderWidgetHostViewQtDelegatePopup::RenderWidgetHostViewQtDelegatePopup(RenderWidgetHostViewQtDelegateClient *client, QWidget *parent)
    : m_client(client)
    , m_parentView(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    // The keyboard events are supposed to go to the parent RenderHostView
    // so the WebUI popups should never have focus. Besides, if the parent view
    // loses focus, WebKit will cause its associated popups (including this one)
    // to be destroyed.
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setFocusPolicy(Qt::NoFocus);
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
}

void RenderWidgetHostViewQtDelegatePopup::initAsChild(WebContentsAdapterClient*)
{
    Q_UNREACHABLE();
}

void RenderWidgetHostViewQtDelegatePopup::initAsPopup(const QRect& rect)
{
    QPoint pos = m_parentView ? m_parentView->mapToGlobal(rect.topLeft()) : QPoint(0,0);
    QRect qrect = QRect(pos, rect.size());
    setGeometry(qrect);
    raise();
    show();
}

QRectF RenderWidgetHostViewQtDelegatePopup::screenRect() const
{
    return QRectF(x(), y(), width(), height());
}

void RenderWidgetHostViewQtDelegatePopup::setKeyboardFocus()
{
    Q_UNREACHABLE();
}

bool RenderWidgetHostViewQtDelegatePopup::hasKeyboardFocus()
{
    return false;
}

void RenderWidgetHostViewQtDelegatePopup::show()
{
    QWidget::show();
}

void RenderWidgetHostViewQtDelegatePopup::hide()
{
    QWidget::hide();
}

bool RenderWidgetHostViewQtDelegatePopup::isVisible() const
{
    return QWidget::isVisible();
}

QWindow* RenderWidgetHostViewQtDelegatePopup::window() const
{
    const QWidget* root = QWidget::window();
    return root ? root->windowHandle() : Q_NULLPTR;
}

void RenderWidgetHostViewQtDelegatePopup::update(const QRect& rect)
{
    QWidget::update(rect);
}

void RenderWidgetHostViewQtDelegatePopup::updateCursor(const QCursor &cursor)
{
    QWidget::setCursor(cursor);
}

void RenderWidgetHostViewQtDelegatePopup::resize(int width, int height)
{
    QWidget::resize(width, height);
}

void RenderWidgetHostViewQtDelegatePopup::move(const QPoint &pos)
{
    QPoint mapped = m_parentView ? m_parentView->mapToGlobal(pos) : pos;
    QWidget::move(mapped);
}

void RenderWidgetHostViewQtDelegatePopup::setTooltip(const QString &tooltip)
{
    setToolTip(tooltip);
}

void RenderWidgetHostViewQtDelegatePopup::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    m_client->fetchBackingStore();
    m_client->paint(&painter, event->rect());
}

void RenderWidgetHostViewQtDelegatePopup::resizeEvent(QResizeEvent *resizeEvent)
{
    Q_UNUSED(resizeEvent);
    m_client->notifyResize();
}

bool RenderWidgetHostViewQtDelegatePopup::event(QEvent *event)
{
    if (!m_client->forwardEvent(event))
        return QWidget::event(event);
    return true;
}
