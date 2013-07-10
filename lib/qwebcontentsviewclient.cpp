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
#include "qwebcontentsviewclient.h"

#include "browser_context_qt.h"
#include "content_browser_client_qt.h"
#include "web_contents_delegate_qt.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/navigation_entry.h"

QWebContentsViewClient::QWebContentsViewClient()
{
    // This has to be the first thing we do.
    scoped_refptr<WebEngineContext> webEngineContext(WebEngineContext::current());
    context = webEngineContext.get();
    // We can't use a scoped_refptr member so we ref-count by hand here.
    context->AddRef();

    content::BrowserContext* browser_context = ContentBrowserClientQt::Get()->browser_context();
    webContentsDelegate.reset(new WebContentsDelegateQt(browser_context, NULL, MSG_ROUTING_NONE, gfx::Size()));
    webContentsDelegate->m_viewClient = this;
    WebContentsViewQt* contents_view = static_cast<WebContentsViewQt*>(webContentsDelegate->web_contents()->GetView());
    contents_view->SetClient(this);
}

QWebContentsViewClient::~QWebContentsViewClient()
{
    context->Release();
}

bool QWebContentsViewClient::canGoBack() const
{
    return webContentsDelegate->web_contents()->GetController().CanGoBack();
}

bool QWebContentsViewClient::canGoForward() const
{
    return webContentsDelegate->web_contents()->GetController().CanGoForward();
}
bool QWebContentsViewClient::isLoading() const
{
    return webContentsDelegate->web_contents()->IsLoading();
}

void QWebContentsViewClient::navigateHistory(int offset)
{
    webContentsDelegate->web_contents()->GetController().GoToOffset(offset);
    webContentsDelegate->web_contents()->GetView()->Focus();
}

void QWebContentsViewClient::stop()
{
    content::NavigationController& controller = webContentsDelegate->web_contents()->GetController();

    int index = controller.GetPendingEntryIndex();
    if (index != -1)
        controller.RemoveEntryAtIndex(index);

    webContentsDelegate->web_contents()->GetView()->Focus();
}

void QWebContentsViewClient::reload()
{
    webContentsDelegate->web_contents()->GetController().Reload(/*checkRepost = */false);
    webContentsDelegate->web_contents()->GetView()->Focus();
}

void QWebContentsViewClient::load(const QUrl &url)
{
    QString urlString = url.toString();
    GURL gurl(urlString.toStdString());
    if (!gurl.has_scheme())
        gurl = GURL(std::string("http://") + urlString.toStdString());

    content::NavigationController::LoadURLParams params(gurl);
    params.transition_type = content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
    webContentsDelegate->web_contents()->GetController().LoadURLWithParams(params);
    webContentsDelegate->web_contents()->GetView()->Focus();
}

QUrl QWebContentsViewClient::activeUrl() const
{
    GURL gurl = webContentsDelegate->web_contents()->GetActiveURL();
    return QUrl(QString::fromStdString(gurl.spec()));
}

QString QWebContentsViewClient::pageTitle() const
{
    content::NavigationEntry* entry = webContentsDelegate->web_contents()->GetController().GetVisibleEntry();
    if (!entry)
        return QString();
    return QString::fromUtf16(entry->GetTitle().data());
}
