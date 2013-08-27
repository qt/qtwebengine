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
#include "web_contents_adapter.h"

#include "content_browser_client_qt.h"
#include "browser_context_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_delegate_qt.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/common/renderer_preferences.h"

#include <QGuiApplication>
#include <QStyleHints>

static const int kTestWindowWidth = 800;
static const int kTestWindowHeight = 600;

// Used to maintain a WebEngineContext for as long as we need one
class WebContentsAdapterPrivate {
public:
    WebContentsAdapterPrivate();
    scoped_refptr<WebEngineContext> engineContext;
    scoped_ptr<content::WebContents> webContents;
    scoped_ptr<WebContentsDelegateQt> webContentsDelegate;
    WebContentsAdapterClient *adapterClient;
};

WebContentsAdapterPrivate::WebContentsAdapterPrivate()
    // This has to be the first thing we create, and the last we destroy.
    : engineContext(WebEngineContext::current())
{
}

WebContentsAdapter::WebContentsAdapter(WebContentsAdapterClient *adapterClient)
    : d_ptr(new WebContentsAdapterPrivate)
{
    Q_D(WebContentsAdapter);
    d->adapterClient = adapterClient;

    content::BrowserContext* browserContext = ContentBrowserClientQt::Get()->browser_context();
    content::WebContents::CreateParams create_params(browserContext, NULL);
    create_params.routing_id = MSG_ROUTING_NONE;
    create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);
    d->webContents.reset(content::WebContents::Create(create_params));

    content::RendererPreferences* rendererPrefs = d->webContents->GetMutableRendererPrefs();
    rendererPrefs->use_custom_colors = true;
    // Qt returns a flash time (the whole cycle) in ms, chromium expects just the interval in seconds
    const int qtCursorFlashTime = QGuiApplication::styleHints()->cursorFlashTime();
    rendererPrefs->caret_blink_interval = 0.5 * static_cast<double>(qtCursorFlashTime) / 1000;
    d->webContents->GetRenderViewHost()->SyncRendererPrefs();

    // Create and attach a WebContentsDelegateQt to the WebContents.
    d->webContentsDelegate.reset(new WebContentsDelegateQt(d->webContents.get(), adapterClient));

    // Let the WebContent's view know about the WebContentsAdapterClient.
    WebContentsViewQt* contentsView = static_cast<WebContentsViewQt*>(d->webContents->GetView());
    contentsView->SetClient(adapterClient);
}

WebContentsAdapter::~WebContentsAdapter()
{
}

bool WebContentsAdapter::canGoBack() const
{
    return webContents()->GetController().CanGoBack();
}

bool WebContentsAdapter::canGoForward() const
{
    return webContents()->GetController().CanGoForward();
}

bool WebContentsAdapter::isLoading() const
{
    return webContents()->IsLoading();
}

void WebContentsAdapter::stop()
{
    content::NavigationController& controller = webContents()->GetController();

    int index = controller.GetPendingEntryIndex();
    if (index != -1)
        controller.RemoveEntryAtIndex(index);

    webContents()->GetView()->Focus();
}

void WebContentsAdapter::reload()
{
    webContents()->GetController().Reload(/*checkRepost = */false);
    webContents()->GetView()->Focus();
}

void WebContentsAdapter::load(const QUrl &url)
{
    content::NavigationController::LoadURLParams params(toGurl(url));
    params.transition_type = content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
    webContents()->GetController().LoadURLWithParams(params);
    webContents()->GetView()->Focus();
}

QUrl WebContentsAdapter::activeUrl() const
{
    return toQt(webContents()->GetVisibleURL());
}

QString WebContentsAdapter::pageTitle() const
{
    content::NavigationEntry* entry = webContents()->GetController().GetVisibleEntry();
    return entry ? toQt(entry->GetTitle()) : QString();
}

void WebContentsAdapter::navigateToIndex(int offset)
{
    webContents()->GetController().GoToIndex(offset);
    webContents()->GetView()->Focus();
}

void WebContentsAdapter::navigateToOffset(int offset)
{
    webContents()->GetController().GoToOffset(offset);
    webContents()->GetView()->Focus();
}

int WebContentsAdapter::navigationEntryCount()
{
    return webContents()->GetController().GetEntryCount();
}

int WebContentsAdapter::currentNavigationEntryIndex()
{
    return webContents()->GetController().GetCurrentEntryIndex();
}

QUrl WebContentsAdapter::getNavigationEntryOriginalUrl(int index)
{
    content::NavigationEntry *entry = webContents()->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetOriginalRequestURL()) : QUrl();
}

QUrl WebContentsAdapter::getNavigationEntryUrl(int index)
{
    content::NavigationEntry *entry = webContents()->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetURL()) : QUrl();
}

QString WebContentsAdapter::getNavigationEntryTitle(int index)
{
    content::NavigationEntry *entry = webContents()->GetController().GetEntryAtIndex(index);
    return entry ? toQt(entry->GetTitle()) : QString();
}

void WebContentsAdapter::clearNavigationHistory()
{
    if (webContents()->GetController().CanPruneAllButVisible())
        webContents()->GetController().PruneAllButVisible();
}

content::WebContents *WebContentsAdapter::webContents() const
{
    Q_D(const WebContentsAdapter);
    return d->webContents.get();
}
