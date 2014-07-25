/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "network_delegate_qt.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/common/page_transition_types.h"
#include "net/url_request/url_request.h"

#include <QSet>
#include <QStringBuilder>

#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

namespace {

int pageTransitionToNavigationType(content::PageTransition transition)
{
    int32 qualifier = content::PageTransitionGetQualifier(transition);

    if (qualifier & content::PAGE_TRANSITION_FORWARD_BACK)
        return WebContentsAdapterClient::BackForwardNavigation;

    content::PageTransition stippedTransition = content::PageTransitionStripQualifier(transition);

    switch (stippedTransition) {
    case content::PAGE_TRANSITION_LINK:
        return WebContentsAdapterClient::LinkClickedNavigation;
    case content::PAGE_TRANSITION_TYPED:
        return WebContentsAdapterClient::TypedNavigation;
    case content::PAGE_TRANSITION_FORM_SUBMIT:
        return WebContentsAdapterClient::FormSubmittedNavigation;
    case content::PAGE_TRANSITION_RELOAD:
        return WebContentsAdapterClient::ReloadNavigation;
    default:
        return WebContentsAdapterClient::OtherNavigation;
    }
}

}

QSet<qintptr> NetworkDelegateQt::m_activeRequests = QSet<qintptr>();

int NetworkDelegateQt::OnBeforeURLRequest(net::URLRequest *request, const net::CompletionCallback &callback, GURL *new_url)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    const content::ResourceRequestInfo *info = content::ResourceRequestInfo::ForRequest(request);
    int renderProcessId;
    int renderViewId;
    if (!info || !info->GetRenderViewForRequest(request, &renderProcessId, &renderViewId))
        // Abort the request if it has no associated render info / render view.
        return net::ERR_ABORTED;

    qintptr requestId = reinterpret_cast<qintptr>(request);

    // Track active requests since |callback| and |new_url| are valid
    // only until OnURLRequestDestroyed is called for this request.
    m_activeRequests.insert(requestId);

    int navigationType = WebContentsAdapterClient::OtherNavigation;

    // Mark user redirects.
    if (request->is_redirecting())
        navigationType = WebContentsAdapterClient::UserRedirectedNavigation;
    else
        navigationType = pageTransitionToNavigationType(info->GetPageTransition());

    content::BrowserThread::PostTask(
                content::BrowserThread::UI,
                FROM_HERE,
                base::Bind(&NotifyNavigationRequestedOnUIThread,
                           requestId,
                           navigationType,
                           &request->url(),
                           new_url,
                           callback,
                           renderProcessId,
                           renderViewId)
                );

    // We'll run the callback after we notified the UI thread.
    return net::ERR_IO_PENDING;
}

void NetworkDelegateQt::OnURLRequestDestroyed(net::URLRequest* request)
{
    m_activeRequests.remove(reinterpret_cast<qintptr>(request));
}

void NetworkDelegateQt::CompleteURLRequestOnIOThread(qintptr requestId,
                                                     int navigationRequestAction,
                                                     QUrl url,
                                                     GURL *new_url,
                                                     const net::CompletionCallback &callback)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    if (!NetworkDelegateQt::IsValidRequest(requestId))
        return;

    int error = net::OK;
    switch (navigationRequestAction) {
    case WebContentsAdapterClient::AcceptRequest:
        error = net::OK;
        break;
    case WebContentsAdapterClient::RedirectRequest: {
        error = net::OK;
        GURL gUrl = toGurl(url);
        std::swap(*new_url, gUrl);
        break;
    }
    case WebContentsAdapterClient::IgnoreRequest:
        error = net::OK;
        // It is safe to cancel the request here since we are on the IO thread.
        reinterpret_cast<net::URLRequest*>(requestId)->Cancel();
        break;
    default:
        error = net::ERR_FAILED;
        Q_UNREACHABLE();
    }

    callback.Run(error);
}

void NetworkDelegateQt::NotifyNavigationRequestedOnUIThread(qintptr requestId,
                                                            int navigationType,
                                                            const GURL *url,
                                                            GURL *new_url,
                                                            const net::CompletionCallback &callback,
                                                            int renderProcessId,
                                                            int renderViewId)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    if (!NetworkDelegateQt::IsValidRequest(requestId))
        return;

    QUrl qUrl = toQt(*url);
    int navigationRequestAction = WebContentsAdapterClient::AcceptRequest;
    content::RenderViewHost *rvh = content::RenderViewHost::FromID(renderProcessId, renderViewId);

    if (rvh) {
        content::WebContents *webContents = content::WebContents::FromRenderViewHost(rvh);
        WebContentsAdapterClient *client = WebContentsViewQt::from(webContents->GetView())->client();
        client->navigationRequested(navigationType, qUrl, navigationRequestAction);
    }

    // Run the callback on the IO thread.
    content::BrowserThread::PostTask(
                content::BrowserThread::IO,
                FROM_HERE,
                base::Bind(&CompleteURLRequestOnIOThread,
                           requestId,
                           navigationRequestAction,
                           qUrl,
                           new_url,
                           callback)
                );
}
