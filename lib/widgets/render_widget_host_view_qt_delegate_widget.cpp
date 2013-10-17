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

#include "render_widget_host_view_qt_delegate_widget.h"

#include "qwebengineview.h"
#include "qwebenginepage_p.h"
#include <QtGlobal>
#include <QLayout>
#include <QResizeEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QWindow>

RenderWidgetHostViewQtDelegateWidget::RenderWidgetHostViewQtDelegateWidget(WebContentsAdapterClient::CompositingMode mode, QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_OpaquePaintEvent);

#if defined(Q_OS_LINUX)
    // FOR TESTING ONLY, use at your own risks.
    // Supporting this properly on all platforms would require duplicating
    // many tricks done by RenderWidgetHostView[Win|Mac].
    if (mode == WebContentsAdapterClient::ForcedGpuProcessCompositing) {
        // This sets Qt::WA_NativeWindow and force a native window creation
        // that we can give to the GPU process for it to render directly
        // on through windowHandle().
        winId();
        // This makes sure that we won't try to paint the regular backing store
        // on the window at the same time as the compositor.
        setUpdatesEnabled(false);
    }
#endif
}

void RenderWidgetHostViewQtDelegateWidget::initAsChild(WebContentsAdapterClient* container)
{
    QWebEnginePagePrivate *pagePrivate = static_cast<QWebEnginePagePrivate *>(container);
    // FIXME: What is going to trigger this if the page is attached later to the view?
    if (pagePrivate->view)
        pagePrivate->view->layout()->addWidget(this);
}

void RenderWidgetHostViewQtDelegateWidget::initAsPopup(QRect& rect)
{
    setGeometry(rect);
    show();
}

void RenderWidgetHostViewQtDelegateWidget::setParentWidget(RenderWidgetHostViewQtDelegate* container)
{
    RenderWidgetHostViewQtDelegateWidget* parent = static_cast<RenderWidgetHostViewQtDelegateWidget*>(container);
    setParent(parent);
}

QRectF RenderWidgetHostViewQtDelegateWidget::screenRect() const
{
    return QRectF(x(), y(), width(), height());
}

void RenderWidgetHostViewQtDelegateWidget::setKeyboardFocus()
{
    setFocus();
}

bool RenderWidgetHostViewQtDelegateWidget::hasKeyboardFocus()
{
    return hasFocus();
}

void RenderWidgetHostViewQtDelegateWidget::show()
{
    QWidget::show();
}

void RenderWidgetHostViewQtDelegateWidget::hide()
{
    QWidget::hide();
}

bool RenderWidgetHostViewQtDelegateWidget::isVisible() const
{
    return QWidget::isVisible();
}

WId RenderWidgetHostViewQtDelegateWidget::nativeWindowIdForCompositor() const
{
    QWindow* window = QWidget::windowHandle();
    return window ? window->winId() : 0;
}

QWindow* RenderWidgetHostViewQtDelegateWidget::window() const
{
    const QWidget* root = QWidget::window();
    return root ? root->windowHandle() : 0;
}

void RenderWidgetHostViewQtDelegateWidget::update(const QRect& rect)
{
    QWidget::update(rect);
}

void RenderWidgetHostViewQtDelegateWidget::updateCursor(const QCursor &cursor)
{
    QWidget::setCursor(cursor);
}

void RenderWidgetHostViewQtDelegateWidget::resize(int width, int height)
{
    QWidget::resize(width, height);
}

void RenderWidgetHostViewQtDelegateWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    fetchBackingStore();
    paint(&painter, event->rect());
}

void RenderWidgetHostViewQtDelegateWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    Q_UNUSED(resizeEvent);
    notifyResize();
}

bool RenderWidgetHostViewQtDelegateWidget::event(QEvent *event)
{
    if (!forwardEvent(event))
        return QWidget::event(event);
    return true;
}
