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

#include "qquickwebcontentsview_p.h"
#include "qquickwebcontentsview_p_p.h"

#include "web_contents_adapter.h"
#include "render_widget_host_view_qt_delegate_quick.h"

#include <QUrl>


QQuickWebContentsViewPrivate::QQuickWebContentsViewPrivate()
    : adapter(new WebContentsAdapter(this))
{
}

RenderWidgetHostViewQtDelegate *QQuickWebContentsViewPrivate::CreateRenderWidgetHostViewQtDelegate()
{
    Q_Q(QQuickWebContentsView);
    // Parent the RWHVQtDelegate directly, this might have to be changed to handle popups and fullscreen.
    RenderWidgetHostViewQtDelegateQuick *viewDelegate = new RenderWidgetHostViewQtDelegateQuick(q);
    return viewDelegate;
}

void QQuickWebContentsViewPrivate::titleChanged(const QString &title)
{
    Q_Q(QQuickWebContentsView);
    Q_UNUSED(title);
    Q_EMIT q->titleChanged();
}

void QQuickWebContentsViewPrivate::urlChanged(const QUrl &url)
{
    Q_Q(QQuickWebContentsView);
    Q_UNUSED(url);
    Q_EMIT q->urlChanged();
}

void QQuickWebContentsViewPrivate::loadingStateChanged()
{
    Q_Q(QQuickWebContentsView);
    Q_EMIT q->loadingStateChanged();
}

QRectF QQuickWebContentsViewPrivate::viewportRect() const
{
    Q_Q(const QQuickWebContentsView);
    return QRectF(q->x(), q->y(), q->width(), q->height());
}

QQuickWebContentsView::QQuickWebContentsView()
    : d_ptr(new QQuickWebContentsViewPrivate)
{
    d_ptr->q_ptr = this;
}

QQuickWebContentsView::~QQuickWebContentsView()
{
}

QUrl QQuickWebContentsView::url() const
{
    Q_D(const QQuickWebContentsView);
    return d->adapter->activeUrl();
}

void QQuickWebContentsView::setUrl(const QUrl& url)
{
    Q_D(QQuickWebContentsView);
    d->adapter->load(url);
}

void QQuickWebContentsView::goBack()
{
    Q_D(QQuickWebContentsView);
    d->adapter->navigateHistory(-1);
}

void QQuickWebContentsView::goForward()
{
    Q_D(QQuickWebContentsView);
    d->adapter->navigateHistory(1);
}

void QQuickWebContentsView::reload()
{
    Q_D(QQuickWebContentsView);
    d->adapter->reload();
}

void QQuickWebContentsView::stop()
{
    Q_D(QQuickWebContentsView);
    d->adapter->stop();
}

bool QQuickWebContentsView::isLoading() const
{
    Q_D(const QQuickWebContentsView);
    return d->adapter->isLoading();
}

QString QQuickWebContentsView::title() const
{
    Q_D(const QQuickWebContentsView);
    return d->adapter->pageTitle();
}

bool QQuickWebContentsView::canGoBack() const
{
    Q_D(const QQuickWebContentsView);
    return d->adapter->canGoBack();
}

bool QQuickWebContentsView::canGoForward() const
{
    Q_D(const QQuickWebContentsView);
    return d->adapter->canGoForward();
}

void QQuickWebContentsView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    Q_FOREACH(QQuickItem *child, childItems()) {
        Q_ASSERT(qobject_cast<RenderWidgetHostViewQtDelegateQuick *>(child));
        child->setSize(newGeometry.size());
    }
}
