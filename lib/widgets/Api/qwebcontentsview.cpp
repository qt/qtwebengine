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

#include "qwebcontentsview.h"
#include "qwebcontentsview_p.h"

#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"

#include <QStackedLayout>
#include <QUrl>

QWebContentsViewPrivate::QWebContentsViewPrivate()
    : m_isLoading(false)
    , adapter(new WebContentsAdapter(this))
{
}

void QWebContentsViewPrivate::adjustSize(RenderWidgetHostViewQtDelegate *viewDelegate)
{
    Q_UNUSED(viewDelegate);
}

void QWebContentsViewPrivate::titleChanged(const QString &title)
{
    Q_EMIT q_ptr->titleChanged(title);
}

void QWebContentsViewPrivate::urlChanged(const QUrl &url)
{
    Q_EMIT q_ptr->urlChanged(url);
}

void QWebContentsViewPrivate::loadingStateChanged()
{
    Q_Q(QWebContentsView);
    const bool wasLoading = m_isLoading;
    m_isLoading = adapter->isLoading();
    if (m_isLoading != wasLoading) {
        if (m_isLoading)
            Q_EMIT q->loadStarted();
        else
            Q_EMIT q->loadFinished(true);
    }
}

RenderWidgetHostViewQtDelegate *QWebContentsViewPrivate::CreateRenderWidgetHostViewQtDelegate()
{
    Q_Q(QWebContentsView);
    RenderWidgetHostViewQtDelegateWidget *viewDelegate = new RenderWidgetHostViewQtDelegateWidget(q);
    // Parent the RWHV directly, this might have to be changed to handle popups and fullscreen.
    q->layout()->addWidget(viewDelegate);
    return viewDelegate;
}

QWebContentsView::QWebContentsView()
    : d_ptr(new QWebContentsViewPrivate)
{
    d_ptr->q_ptr=this;
    // This causes the child RenderWidgetHostViewQtDelegateWidgets to fill this widget.
    setLayout(new QStackedLayout);
}

QWebContentsView::~QWebContentsView()
{
}

void QWebContentsView::load(const QUrl& url)
{
    Q_D(QWebContentsView);
    d->adapter->load(url);
}

bool QWebContentsView::canGoBack() const
{
    Q_D(const QWebContentsView);
    return d->adapter->canGoBack();
}

bool QWebContentsView::canGoForward() const
{
    Q_D(const QWebContentsView);
    return d->adapter->canGoForward();
}

void QWebContentsView::back()
{
    Q_D(QWebContentsView);
    d->adapter->navigateHistory(-1);
}

void QWebContentsView::forward()
{
    Q_D(QWebContentsView);
    d->adapter->navigateHistory(1);
}

void QWebContentsView::reload()
{
    Q_D(QWebContentsView);
    d->adapter->reload();
}

void QWebContentsView::stop()
{
    Q_D(QWebContentsView);
    d->adapter->stop();
}

#include "moc_qwebcontentsview.cpp"
