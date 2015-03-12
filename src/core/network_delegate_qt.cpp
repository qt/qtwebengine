/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "network_delegate_qt.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_details.h"
#include "content/public/browser/resource_request_info.h"
#include "ui/base/page_transition_types.h"
#include "net/base/load_flags.h"
#include "net/url_request/url_request.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

namespace QtWebEngineCore {

int pageTransitionToNavigationType(ui::PageTransition transition)
{
    int32 qualifier = ui::PageTransitionGetQualifier(transition);

    if (qualifier & ui::PAGE_TRANSITION_FORWARD_BACK)
        return WebContentsAdapterClient::BackForwardNavigation;

    ui::PageTransition stippedTransition = ui::PageTransitionStripQualifier(transition);

    switch (stippedTransition) {
    case ui::PAGE_TRANSITION_LINK:
        return WebContentsAdapterClient::LinkClickedNavigation;
    case ui::PAGE_TRANSITION_TYPED:
        return WebContentsAdapterClient::TypedNavigation;
    case ui::PAGE_TRANSITION_FORM_SUBMIT:
        return WebContentsAdapterClient::FormSubmittedNavigation;
    case ui::PAGE_TRANSITION_RELOAD:
        return WebContentsAdapterClient::ReloadNavigation;
    default:
        return WebContentsAdapterClient::OtherNavigation;
    }
}

int NetworkDelegateQt::OnBeforeURLRequest(net::URLRequest *request, const net::CompletionCallback &callback, GURL *)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    const content::ResourceRequestInfo *info = content::ResourceRequestInfo::ForRequest(request);
    if (!info)
        return net::OK;

    content::ResourceType resourceType = info->GetResourceType();
    int renderProcessId;
    int renderFrameId;
    // Only intercept MAIN_FRAME and SUB_FRAME with an associated render frame.
    if (!content::IsResourceTypeFrame(resourceType) || !info->GetRenderFrameForRequest(request, &renderProcessId, &renderFrameId))
        return net::OK;

    // Track active requests since |callback| and |new_url| are valid
    // only until OnURLRequestDestroyed is called for this request.
    m_activeRequests.insert(request);

    int navigationType = pageTransitionToNavigationType(info->GetPageTransition());

    RequestParams params = {
        toQt(request->url()),
        info->IsMainFrame(),
        navigationType,
        renderProcessId,
        renderFrameId
    };

    content::BrowserThread::PostTask(
                content::BrowserThread::UI,
                FROM_HERE,
                base::Bind(&NetworkDelegateQt::NotifyNavigationRequestedOnUIThread,
                           base::Unretained(this),
                           request,
                           params,
                           callback)
                );

    // We'll run the callback after we notified the UI thread.
    return net::ERR_IO_PENDING;
}

void NetworkDelegateQt::OnURLRequestDestroyed(net::URLRequest* request)
{
    m_activeRequests.remove(request);
}

void NetworkDelegateQt::CompleteURLRequestOnIOThread(net::URLRequest *request,
                                                     int navigationRequestAction,
                                                     const net::CompletionCallback &callback)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    if (!m_activeRequests.contains(request))
        return;

    if (request->status().status() == net::URLRequestStatus::CANCELED)
        return;

    int error = net::OK;
    switch (navigationRequestAction) {
    case WebContentsAdapterClient::AcceptRequest:
        error = net::OK;
        break;
    case WebContentsAdapterClient::IgnoreRequest:
        error = net::ERR_ABORTED;
        break;
    default:
        error = net::ERR_FAILED;
        Q_UNREACHABLE();
    }
    callback.Run(error);
}

void NetworkDelegateQt::NotifyNavigationRequestedOnUIThread(net::URLRequest *request,
                                                            RequestParams params,
                                                            const net::CompletionCallback &callback)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    int navigationRequestAction = WebContentsAdapterClient::AcceptRequest;
    content::RenderFrameHost *rfh = content::RenderFrameHost::FromID(params.renderProcessId, params.renderFrameId);

    if (rfh) {
        content::WebContents *webContents = content::WebContents::FromRenderViewHost(rfh->GetRenderViewHost());
        WebContentsAdapterClient *client = WebContentsViewQt::from(static_cast<content::WebContentsImpl*>(webContents)->GetView())->client();
        client->navigationRequested(params.navigationType, params.url, navigationRequestAction, params.isMainFrameRequest);
    }

    // Run the callback on the IO thread.
    content::BrowserThread::PostTask(
                content::BrowserThread::IO,
                FROM_HERE,
                base::Bind(&NetworkDelegateQt::CompleteURLRequestOnIOThread,
                           base::Unretained(this),
                           request,
                           navigationRequestAction,
                           callback)
                );
}

} // namespace QtWebEngineCore
