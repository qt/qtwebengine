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

#include "content/public/browser/web_contents.h"

#include "browser_context_qt.h"
#include "content_browser_client_qt.h"
#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_delegate_qt.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"

#include <QVBoxLayout>
#include <QUrl>


QWebContentsView::QWebContentsView()
    : d_ptr(new QWebContentsViewPrivate)
{
    d_ptr->q_ptr = this;

    Q_D(QWebContentsView);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    WebContentsDelegateQt* delegate = d->webContentsDelegate.get();
    connect(delegate, SIGNAL(titleChanged(const QString&)), this, SIGNAL(titleChanged(const QString&)));
    connect(delegate, SIGNAL(urlChanged(const QUrl&)), this, SIGNAL(urlChanged(const QUrl&)));
    connect(delegate, SIGNAL(loadingStateChanged()), this, SLOT(_q_onLoadingStateChanged()));
}

QWebContentsView::~QWebContentsView()
{
}

void QWebContentsView::load(const QUrl& url)
{
    Q_D(QWebContentsView);
    QString urlString = url.toString();
    GURL gurl(urlString.toStdString());
    if (!gurl.has_scheme())
        gurl = GURL(std::string("http://") + urlString.toStdString());

    content::NavigationController::LoadURLParams params(gurl);
    params.transition_type = content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
    d->webContentsDelegate->web_contents()->GetController().LoadURLWithParams(params);
    d->webContentsDelegate->web_contents()->GetView()->Focus();
}

bool QWebContentsView::canGoBack() const
{
    Q_D(const QWebContentsView);
    return d->webContentsDelegate->web_contents()->GetController().CanGoBack();
}

bool QWebContentsView::canGoForward() const
{
    Q_D(const QWebContentsView);
    return d->webContentsDelegate->web_contents()->GetController().CanGoForward();
}

void QWebContentsView::back()
{
    Q_D(QWebContentsView);
    d->webContentsDelegate->web_contents()->GetController().GoToOffset(-1);
    d->webContentsDelegate->web_contents()->GetView()->Focus();
}

void QWebContentsView::forward()
{
    Q_D(QWebContentsView);
    d->webContentsDelegate->web_contents()->GetController().GoToOffset(1);
    d->webContentsDelegate->web_contents()->GetView()->Focus();
}

void QWebContentsView::reload()
{
    Q_D(QWebContentsView);
    d->webContentsDelegate->web_contents()->GetController().Reload(false);
    d->webContentsDelegate->web_contents()->GetView()->Focus();
}

void QWebContentsView::stop()
{
    Q_D(QWebContentsView);
    content::NavigationController& controller = d->webContentsDelegate->web_contents()->GetController();

    int index = controller.GetPendingEntryIndex();
    if (index != -1)
        controller.RemoveEntryAtIndex(index);

    d->webContentsDelegate->web_contents()->GetView()->Focus();

}

QWebContentsViewPrivate::QWebContentsViewPrivate()
    // This has to be the first thing we do.
    : context(WebEngineContext::current())
    , m_isLoading(false)
{
    content::BrowserContext* browser_context = static_cast<ContentBrowserClientQt*>(content::GetContentClient()->browser())->browser_context();
    webContentsDelegate.reset(new WebContentsDelegateQt(browser_context, NULL, MSG_ROUTING_NONE, gfx::Size()));

    WebContentsViewQt* contents_view = static_cast<WebContentsViewQt*>(webContentsDelegate->web_contents()->GetView());
    contents_view->SetClient(this);
}

void QWebContentsViewPrivate::_q_onLoadingStateChanged()
{
    Q_Q(QWebContentsView);
    bool isLoading = webContentsDelegate->web_contents()->IsLoading();
    if (m_isLoading != isLoading) {
        m_isLoading = isLoading;
        if (m_isLoading)
            Q_EMIT q->loadStarted();
        else
            Q_EMIT q->loadFinished(true);
    }
}

RenderWidgetHostViewQtDelegate *QWebContentsViewPrivate::CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQt *view)
{
    Q_Q(QWebContentsView);
    RenderWidgetHostViewQtDelegateWidget *viewDelegate = new RenderWidgetHostViewQtDelegateWidget(view, q);
    // Parent the RWHV directly, this might have to be changed to handle popups and fullscreen.
    q->layout()->addWidget(viewDelegate);
    return viewDelegate;
}

#include "moc_qwebcontentsview.cpp"
