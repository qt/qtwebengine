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

#include "web_contents_view_qt.h"

#include "browser_context_qt.h"
#include "content_browser_client_qt.h"

#include "content/browser/renderer_host/render_view_host_impl.h"

namespace content {
WebContentsViewPort* CreateWebContentsView(WebContentsImpl* web_contents, WebContentsViewDelegate* delegate, RenderViewHostDelegateView** render_view_host_delegate_view) { return 0; }
}

WebContentsViewQtClient::WebContentsViewQtClient()
// This has to be the first thing we do.
    : context(WebEngineContext::current())
{
    content::BrowserContext* browser_context = static_cast<ContentBrowserClientQt*>(content::GetContentClient()->browser())->browser_context();
    webContentsDelegate.reset(new WebContentsDelegateQt(browser_context, NULL, MSG_ROUTING_NONE, gfx::Size()));
}

content::RenderWidgetHostView* WebContentsViewQt::CreateViewForWidget(content::RenderWidgetHost* render_widget_host)
{
    RenderWidgetHostViewQt *view = new RenderWidgetHostViewQt(render_widget_host);
    RenderWidgetHostViewQtDelegate *viewDelegate = m_client->CreateRenderWidgetHostViewQtDelegate(view);
    view->SetDelegate(viewDelegate);

    return view;
}

void WebContentsViewQt::SetPageTitle(const string16& title)
{
    QString string = QString::fromUtf16(title.data());
    Q_EMIT m_client->webContentsDelegate->titleChanged(string);
}

void WebContentsViewQt::GetContainerBounds(gfx::Rect* out) const
{
    content::RenderWidgetHostView* rwhv = m_client->webContentsDelegate->web_contents()->GetRenderWidgetHostView();
    if (rwhv)
      *out = rwhv->GetViewBounds();
}
