/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "proxying_url_loader_factory_qt.h"

#include <utility>

#include "base/bind.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/task/post_task.h"
#include "components/safe_browsing/common/safebrowsing_constants.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/global_request_id.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/url_utils.h"
#include "net/base/load_flags.h"
#include "net/http/http_status_code.h"
#include "net/http/http_util.h"

#include "api/qwebengineurlrequestinfo_p.h"
#include "profile_io_data_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

#include <QVariant>

// originally based on aw_proxying_url_loader_factory.cc:
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace QtWebEngineCore {

extern WebContentsAdapterClient::NavigationType pageTransitionToNavigationType(ui::PageTransition transition);

static QWebEngineUrlRequestInfo::ResourceType toQt(content::ResourceType resourceType)
{
    if (resourceType >= content::ResourceType::kMainFrame && resourceType <= content::ResourceType::kMaxValue)
        return static_cast<QWebEngineUrlRequestInfo::ResourceType>(resourceType);
    return QWebEngineUrlRequestInfo::ResourceTypeUnknown;
}

static QWebEngineUrlRequestInfo::NavigationType toQt(WebContentsAdapterClient::NavigationType navigationType)
{
    return static_cast<QWebEngineUrlRequestInfo::NavigationType>(navigationType);
}

// Handles intercepted, in-progress requests/responses, so that they can be
// controlled and modified accordingly.
class InterceptedRequest : public network::mojom::URLLoader
                         , public network::mojom::URLLoaderClient
{
public:
    InterceptedRequest(int process_id, uint64_t request_id, int32_t routing_id, uint32_t options,
                       const network::ResourceRequest &request, const GURL &top_document_url,
                       const net::MutableNetworkTrafficAnnotationTag &traffic_annotation,
                       ProfileIODataQt *profileData,
                       network::mojom::URLLoaderRequest loader_request, network::mojom::URLLoaderClientPtr client,
                       network::mojom::URLLoaderFactoryPtr target_factory);
    ~InterceptedRequest() override;

    void Restart();
    void InterceptOnUIThread();

    // network::mojom::URLLoaderClient
    void OnReceiveResponse(network::mojom::URLResponseHeadPtr head) override;
    void OnReceiveRedirect(const net::RedirectInfo &redirect_info, network::mojom::URLResponseHeadPtr head) override;
    void OnUploadProgress(int64_t current_position, int64_t total_size, OnUploadProgressCallback callback) override;
    void OnReceiveCachedMetadata(mojo_base::BigBuffer data) override;
    void OnTransferSizeUpdated(int32_t transfer_size_diff) override;
    void OnStartLoadingResponseBody(mojo::ScopedDataPipeConsumerHandle body) override;
    void OnComplete(const network::URLLoaderCompletionStatus &status) override;

    // network::mojom::URLLoader
    void FollowRedirect(const std::vector<std::string> &removed_headers,
                        const net::HttpRequestHeaders &modified_headers, const base::Optional<GURL> &new_url) override;
    void SetPriority(net::RequestPriority priority, int32_t intra_priority_value) override;
    void PauseReadingBodyFromNet() override;
    void ResumeReadingBodyFromNet() override;

    void ContinueAfterIntercept();

private:
    // This is called when the original URLLoaderClient has a connection error.
    void OnURLLoaderClientError();

    // This is called when the original URLLoader has a connection error.
    void OnURLLoaderError(uint32_t custom_reason, const std::string &description);

    // Call OnComplete on |target_client_|. If |wait_for_loader_error| is true
    // then this object will wait for |proxied_loader_binding_| to have a
    // connection error before destructing.
    void CallOnComplete(const network::URLLoaderCompletionStatus &status, bool wait_for_loader_error);

    void SendErrorAndCompleteImmediately(int error_code);

    const int process_id_;
    const uint64_t request_id_;
    const int32_t routing_id_;
    const uint32_t options_;
    bool input_stream_previously_failed_ = false;
    bool request_was_redirected_ = false;

    // If the |target_loader_| called OnComplete with an error this stores it.
    // That way the destructor can send it to OnReceivedError if safe browsing
    // error didn't occur.
    int error_status_ = net::OK;
    QUrl m_originalUrl;
    GURL m_topDocumentUrl;

    network::ResourceRequest request_;
    network::ResourceResponseHead current_response_;

    const net::MutableNetworkTrafficAnnotationTag traffic_annotation_;

    QWebEngineUrlRequestInfo m_requestInfo;
    ProfileIODataQt *m_profileData;
    mojo::Binding<network::mojom::URLLoader> proxied_loader_binding_;
    network::mojom::URLLoaderClientPtr target_client_;

    mojo::Binding<network::mojom::URLLoaderClient> proxied_client_binding_;
    network::mojom::URLLoaderPtr target_loader_;
    network::mojom::URLLoaderFactoryPtr target_factory_;

    base::WeakPtrFactory<InterceptedRequest> m_weakFactory;
    base::WeakPtr<InterceptedRequest> m_weakPtr;
    DISALLOW_COPY_AND_ASSIGN(InterceptedRequest);
};

InterceptedRequest::InterceptedRequest(int process_id, uint64_t request_id, int32_t routing_id, uint32_t options,
                                       const network::ResourceRequest &request, const GURL &top_document_url,
                                       const net::MutableNetworkTrafficAnnotationTag &traffic_annotation,
                                       ProfileIODataQt *profileData,
                                       network::mojom::URLLoaderRequest loader_request,
                                       network::mojom::URLLoaderClientPtr client,
                                       network::mojom::URLLoaderFactoryPtr target_factory)
    : process_id_(process_id)
    , request_id_(request_id)
    , routing_id_(routing_id)
    , options_(options)
    , m_topDocumentUrl(top_document_url)
    , request_(request)
    , traffic_annotation_(traffic_annotation)
    , m_profileData(profileData)
    , proxied_loader_binding_(this, std::move(loader_request))
    , target_client_(std::move(client))
    , proxied_client_binding_(this)
    , target_factory_(std::move(target_factory))
    , m_weakFactory(this)
    , m_weakPtr(m_weakFactory.GetWeakPtr())
{
    // If there is a client error, clean up the request.
    target_client_.set_connection_error_handler(
            base::BindOnce(&InterceptedRequest::OnURLLoaderClientError,  m_weakFactory.GetWeakPtr()));
    proxied_loader_binding_.set_connection_error_with_reason_handler(
            base::BindOnce(&InterceptedRequest::OnURLLoaderError,  m_weakFactory.GetWeakPtr()));
}

InterceptedRequest::~InterceptedRequest()
{
    m_weakFactory.InvalidateWeakPtrs();
}

void InterceptedRequest::Restart()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    content::ResourceType resourceType = content::ResourceType(request_.resource_type);
    WebContentsAdapterClient::NavigationType navigationType =
            pageTransitionToNavigationType(ui::PageTransition(request_.transition_type));

    m_originalUrl = toQt(request_.url);

    const QUrl initiator = request_.request_initiator.has_value() ? toQt(request_.request_initiator->GetURL()) : QUrl();

    QUrl firstPartyUrl;
    if (!m_topDocumentUrl.is_empty())
        firstPartyUrl = toQt(m_topDocumentUrl);
    else
        firstPartyUrl = toQt(request_.site_for_cookies); // m_topDocumentUrl can be empty for the main-frame.

    QWebEngineUrlRequestInfoPrivate *infoPrivate =
            new QWebEngineUrlRequestInfoPrivate(toQt(resourceType), toQt(navigationType),
                                                m_originalUrl, firstPartyUrl, initiator,
                                                QByteArray::fromStdString(request_.method));
    m_requestInfo = QWebEngineUrlRequestInfo(infoPrivate);

    if (m_profileData && m_profileData->isInterceptorDeprecated()) {
        QWebEngineUrlRequestInterceptor *interceptor = m_profileData->acquireInterceptor();
        if (interceptor && m_profileData->isInterceptorDeprecated())
            interceptor->interceptRequest(m_requestInfo);
        m_profileData->releaseInterceptor();
    }

    if (m_requestInfo.changed()) {
        ContinueAfterIntercept();
    } else {
        // FIXME: unretained post?
        base::PostTask(FROM_HERE, {content::BrowserThread::UI},
                       base::BindOnce(&InterceptedRequest::InterceptOnUIThread, base::Unretained(this)));
    }
}

void InterceptedRequest::InterceptOnUIThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    content::WebContents *webContents = nullptr;
    if (process_id_) {
        content::RenderFrameHost *frameHost = content::RenderFrameHost::FromID(process_id_, request_.render_frame_id);
        webContents = content::WebContents::FromRenderFrameHost(frameHost);
    } else
        webContents = content::WebContents::FromFrameTreeNodeId(request_.render_frame_id);

    if (webContents) {
        if (m_profileData) {
            QWebEngineUrlRequestInterceptor *interceptor = m_profileData->requestInterceptor();
            if (interceptor && !interceptor->property("deprecated").toBool())
                interceptor->interceptRequest(m_requestInfo);
        }

        WebContentsAdapterClient *client =
            WebContentsViewQt::from(static_cast<content::WebContentsImpl*>(webContents)->GetView())->client();

        if (!m_requestInfo.changed())
            client->interceptRequest(m_requestInfo);
    }
    base::PostTask(FROM_HERE, {content::BrowserThread::IO},
                   base::BindOnce(&InterceptedRequest::ContinueAfterIntercept, m_weakPtr));
}

void InterceptedRequest::ContinueAfterIntercept()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (m_requestInfo.changed()) {
        if (m_requestInfo.d_ptr->shouldBlockRequest)
            return SendErrorAndCompleteImmediately(net::ERR_BLOCKED_BY_CLIENT);
        if (m_requestInfo.requestUrl() != m_originalUrl) {
            net::URLRequest::FirstPartyURLPolicy first_party_url_policy =
                    request_.update_first_party_url_on_redirect ? net::URLRequest::UPDATE_FIRST_PARTY_URL_ON_REDIRECT
                                                                : net::URLRequest::NEVER_CHANGE_FIRST_PARTY_URL;
            net::RedirectInfo redirectInfo = net::RedirectInfo::ComputeRedirectInfo(request_.method, request_.url,
                                                                                    request_.site_for_cookies,
                                                                                    first_party_url_policy, request_.referrer_policy,
                                                                                    request_.referrer.spec(), net::HTTP_TEMPORARY_REDIRECT,
                                                                                    toGurl(m_requestInfo.requestUrl()), base::nullopt,
                                                                                    false /*insecure_scheme_was_upgraded*/);

            // FIXME: Should probably create a new header.
            current_response_.encoded_data_length = 0;
            request_.method = redirectInfo.new_method;
            request_.url = redirectInfo.new_url;
            request_.site_for_cookies = redirectInfo.new_site_for_cookies;
            request_.referrer = GURL(redirectInfo.new_referrer);
            request_.referrer_policy = redirectInfo.new_referrer_policy;
            if (request_.method == net::HttpRequestHeaders::kGetMethod)
                request_.request_body = nullptr;
            target_client_->OnReceiveRedirect(redirectInfo, current_response_);
            return;
        }

        if (!m_requestInfo.d_ptr->extraHeaders.isEmpty()) {
            auto end = m_requestInfo.d_ptr->extraHeaders.constEnd();
            for (auto header = m_requestInfo.d_ptr->extraHeaders.constBegin(); header != end; ++header) {
                std::string h = header.key().toStdString();
                if (base::LowerCaseEqualsASCII(h, "referer")) {
                    request_.referrer = GURL(header.value().toStdString());
                } else {
                    request_.headers.SetHeader(h, header.value().toStdString());
                }
            }
        }
    }

    if (!target_loader_ && target_factory_) {
        network::mojom::URLLoaderClientPtr proxied_client;
        proxied_client_binding_.Bind(mojo::MakeRequest(&proxied_client));
        target_factory_->CreateLoaderAndStart(mojo::MakeRequest(&target_loader_), routing_id_, request_id_, options_,
                                              request_, std::move(proxied_client), traffic_annotation_);
    }
}

// URLLoaderClient methods.

void InterceptedRequest::OnReceiveResponse(network::mojom::URLResponseHeadPtr head)
{
    current_response_ = head;

    target_client_->OnReceiveResponse(std::move(head));
}

void InterceptedRequest::OnReceiveRedirect(const net::RedirectInfo &redirect_info, network::mojom::URLResponseHeadPtr head)
{
    // TODO(timvolodine): handle redirect override.
    request_was_redirected_ = true;
    current_response_ = head;
    target_client_->OnReceiveRedirect(redirect_info, std::move(head));
    request_.url = redirect_info.new_url;
    request_.method = redirect_info.new_method;
    request_.site_for_cookies = redirect_info.new_site_for_cookies;
    request_.referrer = GURL(redirect_info.new_referrer);
    request_.referrer_policy = redirect_info.new_referrer_policy;
}

void InterceptedRequest::OnUploadProgress(int64_t current_position, int64_t total_size, OnUploadProgressCallback callback)
{
    target_client_->OnUploadProgress(current_position, total_size, std::move(callback));
}

void InterceptedRequest::OnReceiveCachedMetadata(mojo_base::BigBuffer data)
{
    target_client_->OnReceiveCachedMetadata(std::move(data));
}

void InterceptedRequest::OnTransferSizeUpdated(int32_t transfer_size_diff)
{
    target_client_->OnTransferSizeUpdated(transfer_size_diff);
}

void InterceptedRequest::OnStartLoadingResponseBody(mojo::ScopedDataPipeConsumerHandle body)
{
    target_client_->OnStartLoadingResponseBody(std::move(body));
}

void InterceptedRequest::OnComplete(const network::URLLoaderCompletionStatus &status)
{
    // Only wait for the original loader to possibly have a custom error if the
    // target loader succeeded. If the target loader failed, then it was a race as
    // to whether that error or the safe browsing error would be reported.
    CallOnComplete(status, status.error_code == net::OK);
}

// URLLoader methods.

void InterceptedRequest::FollowRedirect(const std::vector<std::string> &removed_headers,
                                        const net::HttpRequestHeaders &modified_headers,
                                        const base::Optional<GURL> &new_url)
{
    if (target_loader_)
        target_loader_->FollowRedirect(removed_headers, modified_headers, new_url);

    // If |OnURLLoaderClientError| was called then we're just waiting for the
    // connection error handler of |proxied_loader_binding_|. Don't restart the
    // job since that'll create another URLLoader
    if (!target_client_)
        return;

    Restart();
}

void InterceptedRequest::SetPriority(net::RequestPriority priority, int32_t intra_priority_value)
{
    if (target_loader_)
        target_loader_->SetPriority(priority, intra_priority_value);
}

void InterceptedRequest::PauseReadingBodyFromNet()
{
    if (target_loader_)
        target_loader_->PauseReadingBodyFromNet();
}

void InterceptedRequest::ResumeReadingBodyFromNet()
{
    if (target_loader_)
        target_loader_->ResumeReadingBodyFromNet();
}

void InterceptedRequest::OnURLLoaderClientError()
{
    // We set |wait_for_loader_error| to true because if the loader did have a
    // custom_reason error then the client would be reset as well and it would be
    // a race as to which connection error we saw first.
    CallOnComplete(network::URLLoaderCompletionStatus(net::ERR_ABORTED), true /* wait_for_loader_error */);
}

void InterceptedRequest::OnURLLoaderError(uint32_t custom_reason, const std::string &description)
{
    // If CallOnComplete was already called, then this object is ready to be deleted.
    if (!target_client_)
        delete this;
}

void InterceptedRequest::CallOnComplete(const network::URLLoaderCompletionStatus &status, bool wait_for_loader_error)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    // Save an error status so that we call onReceiveError at destruction if there
    // was no safe browsing error.
    if (status.error_code != net::OK)
        error_status_ = status.error_code;

    if (target_client_)
        target_client_->OnComplete(status);

    if (proxied_loader_binding_ && wait_for_loader_error) {
        // Don't delete |this| yet, in case the |proxied_loader_binding_|'s
        // error_handler is called with a reason to indicate an error which we want
        // to send to the client bridge. Also reset |target_client_| so we don't
        // get its error_handler called and then delete |this|.
        target_client_.reset();

        // Since the original client is gone no need to continue loading the
        // request.
        proxied_client_binding_.Close();
        target_loader_.reset();

        // In case there are pending checks as to whether this request should be
        // intercepted, we don't want that causing |target_client_| to be used
        // later.
        m_weakFactory.InvalidateWeakPtrs();
    } else {
        delete this;
    }
}

void InterceptedRequest::SendErrorAndCompleteImmediately(int error_code)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    auto status = network::URLLoaderCompletionStatus(error_code);
    target_client_->OnComplete(status);
    delete this;
}

ProxyingURLLoaderFactoryQt::ProxyingURLLoaderFactoryQt(int process_id,
                                                       content::ResourceContext *resourceContext,
                                                       content::RenderFrameHostImpl *host,
                                                       mojo::PendingReceiver<network::mojom::URLLoaderFactory> loader_receiver,
                                                       network::mojom::URLLoaderFactoryPtrInfo target_factory_info)
    : m_processId(process_id), m_resourceContext(resourceContext), m_renderFrameHost(host), m_weakFactory(this)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (target_factory_info) {
        m_targetFactory.Bind(std::move(target_factory_info));
        m_targetFactory.set_connection_error_handler(
                base::BindOnce(&ProxyingURLLoaderFactoryQt::OnTargetFactoryError, m_weakFactory.GetWeakPtr()));
    }
    m_proxyReceivers.Add(this, std::move(loader_receiver));
    m_proxyReceivers.set_disconnect_handler(
            base::BindRepeating(&ProxyingURLLoaderFactoryQt::OnProxyBindingError, m_weakFactory.GetWeakPtr()));
}

ProxyingURLLoaderFactoryQt::~ProxyingURLLoaderFactoryQt()
{
    m_weakFactory.InvalidateWeakPtrs();
}

// static
void ProxyingURLLoaderFactoryQt::CreateProxy(int process_id,
                                             content::ResourceContext *resourceContext,
                                             content::RenderFrameHostImpl *host,
                                             mojo::PendingReceiver<network::mojom::URLLoaderFactory> loader_receiver,
                                             network::mojom::URLLoaderFactoryPtrInfo target_factory_info)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    // Will manage its own lifetime
    new ProxyingURLLoaderFactoryQt(process_id, resourceContext, host, std::move(loader_receiver), std::move(target_factory_info));
}

void ProxyingURLLoaderFactoryQt::CreateLoaderAndStart(network::mojom::URLLoaderRequest loader, int32_t routing_id,
                                                      int32_t request_id, uint32_t options,
                                                      const network::ResourceRequest &request,
                                                      network::mojom::URLLoaderClientPtr client,
                                                      const net::MutableNetworkTrafficAnnotationTag &traffic_annotation)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    ProfileIODataQt *profileIOData = ProfileIODataQt::FromResourceContext(m_resourceContext);

    QWebEngineUrlRequestInterceptor *profileInterceptor = profileIOData ? profileIOData->requestInterceptor() : nullptr;
    if (!profileIOData || !(profileInterceptor || profileIOData->hasPageInterceptors())) {
        m_targetFactory->CreateLoaderAndStart(
                    std::move(loader), routing_id, request_id, options, request,
                    std::move(client), traffic_annotation);
        return;
    }

    network::mojom::URLLoaderFactoryPtr target_factory_clone;
    if (m_targetFactory)
        m_targetFactory->Clone(mojo::MakeRequest(&target_factory_clone));

    // Follows a similar path to the root as RenderFrameHostImpl::CalculateSiteForCookies()
    GURL top_document_url;
    if (m_renderFrameHost)
        top_document_url = m_renderFrameHost->frame_tree_node()->frame_tree()->root()->current_frame_host()->GetLastCommittedURL();
    else
        LOG(INFO) << "ProxyingURLLoaderFactoryQt::CreateLoaderAndStart() - null m_renderFrameHost, shouldn't happen";

    // Will manage its own lifetime
    InterceptedRequest *req = new InterceptedRequest(m_processId, request_id, routing_id, options, request,
                                                     top_document_url,
                                                     traffic_annotation, profileIOData,
                                                     std::move(loader), std::move(client),
                                                     std::move(target_factory_clone));
    req->Restart();
}

void ProxyingURLLoaderFactoryQt::OnTargetFactoryError()
{
    delete this;
}

void ProxyingURLLoaderFactoryQt::OnProxyBindingError()
{
    if (m_proxyReceivers.empty())
        delete this;
}

void ProxyingURLLoaderFactoryQt::Clone(mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    m_proxyReceivers.Add(this, std::move(receiver));
}

} // namespace QtWebEngineCore
