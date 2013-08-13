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

#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"

#include <QStackedLayout>
#include <QUrl>

QWebEngineViewPrivate::QWebEngineViewPrivate()
    : QWidgetPrivate(QObjectPrivateVersion)
    , m_isLoading(false)
    , adapter(new WebContentsAdapter(this))
{
}

void QWebEngineViewPrivate::titleChanged(const QString &title)
{
    Q_Q(QWebEngineView);
    Q_EMIT q->titleChanged(title);
}

void QWebEngineViewPrivate::urlChanged(const QUrl &url)
{
    Q_Q(QWebEngineView);
    Q_EMIT q->urlChanged(url);
}

void QWebEngineViewPrivate::loadingStateChanged()
{
    Q_Q(QWebEngineView);
    const bool wasLoading = m_isLoading;
    m_isLoading = adapter->isLoading();
    if (m_isLoading != wasLoading) {
        if (m_isLoading)
            Q_EMIT q->loadStarted();
    }
}

QRectF QWebEngineViewPrivate::viewportRect() const
{
    Q_Q(const QWebEngineView);
    return q->geometry();
}

void QWebEngineViewPrivate::loadFinished(bool success)
{
    Q_Q(QWebEngineView);
    m_isLoading = adapter->isLoading();
    Q_EMIT q->loadFinished(success);
}

void QWebEngineViewPrivate::focusContainer()
{
    Q_Q(QWebEngineView);
    q->setFocus();
}

RenderWidgetHostViewQtDelegate *QWebEngineViewPrivate::CreateRenderWidgetHostViewQtDelegate()
{
    return new RenderWidgetHostViewQtDelegateWidget;
}

QWebEngineView::QWebEngineView(QWidget *parent)
    : QWidget(*(new QWebEngineViewPrivate), parent, 0)
{
    // This causes the child RenderWidgetHostViewQtDelegateWidgets to fill this widget.
    setLayout(new QStackedLayout);
}

QWebEngineView::~QWebEngineView()
{
}

void QWebEngineView::load(const QUrl& url)
{
    Q_D(QWebEngineView);
    d->adapter->load(url);
}

bool QWebEngineView::canGoBack() const
{
    Q_D(const QWebEngineView);
    return d->adapter->canGoBack();
}

bool QWebEngineView::canGoForward() const
{
    Q_D(const QWebEngineView);
    return d->adapter->canGoForward();
}

void QWebEngineView::back()
{
    Q_D(QWebEngineView);
    d->adapter->navigateHistory(-1);
}

void QWebEngineView::forward()
{
    Q_D(QWebEngineView);
    d->adapter->navigateHistory(1);
}

void QWebEngineView::reload()
{
    Q_D(QWebEngineView);
    d->adapter->reload();
}

void QWebEngineView::stop()
{
    Q_D(QWebEngineView);
    d->adapter->stop();
}

#include "moc_qwebengineview.cpp"
