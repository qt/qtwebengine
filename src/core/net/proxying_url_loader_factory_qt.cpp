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
#include "base/task/post_task.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "net/http/http_status_code.h"
#include "services/network/public/cpp/cors/cors.h"
#include "third_party/blink/public/mojom/loader/resource_load_info.mojom-shared.h"
#include "url/url_util.h"

#include "api/qwebengineurlrequestinfo_p.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

#include <QVariant>

// originally based on aw_proxying_url_loader_factory.cc:
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeMainFrame, blink::mojom::ResourceType::kMainFrame)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeSubFrame, blink::mojom::ResourceType::kSubFrame)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeStylesheet, blink::mojom::ResourceType::kStylesheet)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeScript, blink::mojom::ResourceType::kScript)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeImage, blink::mojom::ResourceType::kImage)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeFontResource, blink::mojom::ResourceType::kFontResource)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeSubResource, blink::mojom::ResourceType::kSubResource)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeObject, blink::mojom::ResourceType::kObject)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeMedia, blink::mojom::ResourceType::kMedia)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeWorker, blink::mojom::ResourceType::kWorker)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeSharedWorker, blink::mojom::ResourceType::kSharedWorker)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypePrefetch, blink::mojom::ResourceType::kPrefetch)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeFavicon, blink::mojom::ResourceType::kFavicon)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeXhr, blink::mojom::ResourceType::kXhr)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypePing, blink::mojom::ResourceType::kPing)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeServiceWorker, blink::mojom::ResourceType::kServiceWorker)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeCspReport, blink::mojom::ResourceType::kCspReport)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypePluginResource, blink::mojom::ResourceType::kPluginResource)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeNavigationPreloadMainFrame, blink::mojom::ResourceType::kNavigationPreloadMainFrame)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeNavigationPreloadSubFrame, blink::mojom::ResourceType::kNavigationPreloadSubFrame)
ASSERT_ENUMS_MATCH(QWebEngineUrlRequestInfo::ResourceTypeLast, blink::mojom::ResourceType::kMaxValue)

extern WebContentsAdapterClient::NavigationType pageTransitionToNavigationType(ui::PageTransition transition);

static QWebEngineUrlRequestInfo::ResourceType toQt(blink::mojom::ResourceType resourceType)
{
    if (resourceType >= blink::mojom::ResourceType::kMinValue && resourceType <= blink::mojom::ResourceType::kMaxValue)
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
    InterceptedRequest(ProfileAdapter *profile_adapter,
                       int process_id, uint64_t request_id, int32_t routing_id, uint32_t options,
                       const network::ResourceRequest &request,
                       const net::MutableNetworkTrafficAnnotationTag &traffic_annotation,
                       mojo::PendingReceiver<network::mojom::URLLoader> loader,
                       mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                       mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory);
    ~InterceptedRequest() override;

    void Restart();

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
                        const net::HttpRequestHeaders &modified_headers,
                        const net::HttpRequestHeaders &modified_cors_exempt_headers,
                        const base::Optional<GURL> &new_url) override;
    void SetPriority(net::RequestPriority priority, int32_t intra_priority_value) override;
    void PauseReadingBodyFromNet() override;
    void ResumeReadingBodyFromNet() override;

    void InterceptOnUIThread(QWebEngineUrlRequestInterceptor *profileInterceptor);
    void InterceptOnIOThread(base::WaitableEvent *event, QWebEngineUrlRequestInterceptor *profileInterceptor);
    void ContinueAfterIntercept();

    // This is called when the original URLLoaderClient has a connection error.
    void OnURLLoaderClientError();

    // This is called when the original URLLoader has a connection error.
    void OnURLLoaderError(uint32_t custom_reason, const std::string &description);

    // Call OnComplete on |target_client_|. If |wait_for_loader_error| is true
    // then this object will wait for |proxied_loader_binding_| to have a
    // connection error before destructing.
    void CallOnComplete(const network::URLLoaderCompletionStatus &status, bool wait_for_loader_error);

    void SendErrorAndCompleteImmediately(int error_code);

    content::WebContents* webContents();
    QWebEngineUrlRequestInterceptor* getProfileInterceptor();
    QWebEngineUrlRequestInterceptor* getPageInterceptor();

    QPointer<ProfileAdapter> profile_adapter_;
    const int process_id_;
    const uint64_t request_id_;
    const int32_t routing_id_;
    const uint32_t options_;
    bool allowed_cors_ = true;

    // If the |target_loader_| called OnComplete with an error this stores it.
    // That way the destructor can send it to OnReceivedError if safe browsing
    // error didn't occur.
    int error_status_ = net::OK;
    network::ResourceRequest request_;
    network::mojom::URLResponseHeadPtr current_response_;

    const net::MutableNetworkTrafficAnnotationTag traffic_annotation_;

    static inline void cleanup(QWebEngineUrlRequestInfo *info) { delete info; }
    QScopedPointer<QWebEngineUrlRequestInfo, InterceptedRequest> request_info_;

    mojo::Receiver<network::mojom::URLLoader> proxied_loader_receiver_;
    mojo::Remote<network::mojom::URLLoaderClient> target_client_;
    mojo::Receiver<network::mojom::URLLoaderClient> proxied_client_receiver_{this};
    mojo::Remote<network::mojom::URLLoader> target_loader_;
    mojo::Remote<network::mojom::URLLoaderFactory> target_factory_;

    base::WeakPtrFactory<InterceptedRequest> weak_factory_;
    DISALLOW_COPY_AND_ASSIGN(InterceptedRequest);
};

InterceptedRequest::InterceptedRequest(ProfileAdapter *profile_adapter,
                                       int process_id, uint64_t request_id, int32_t routing_id, uint32_t options,
                                       const network::ResourceRequest &request,
                                       const net::MutableNetworkTrafficAnnotationTag &traffic_annotation,
                                       mojo::PendingReceiver<network::mojom::URLLoader> loader_receiver,
                                       mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                                       mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory)
    : profile_adapter_(profile_adapter)
    , process_id_(process_id)
    , request_id_(request_id)
    , routing_id_(routing_id)
    , options_(options)
    , request_(request)
    , traffic_annotation_(traffic_annotation)
    , proxied_loader_receiver_(this, std::move(loader_receiver))
    , target_client_(std::move(client))
    , target_factory_(std::move(target_factory))
    , weak_factory_(this)
{
    const bool disable_web_security = base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kDisableWebSecurity);
    current_response_ = network::mojom::URLResponseHead::New();
    current_response_->response_type = network::cors::CalculateResponseType(
        request_.mode,
        disable_web_security || (
            request_.request_initiator && request_.request_initiator->IsSameOriginWith(url::Origin::Create(request_.url))));
    // If there is a client error, clean up the request.
    target_client_.set_disconnect_handler(
            base::BindOnce(&InterceptedRequest::OnURLLoaderClientError, base::Unretained(this)));
    proxied_loader_receiver_.set_disconnect_with_reason_handler(
            base::BindOnce(&InterceptedRequest::OnURLLoaderError, base::Unretained(this)));
    if (!disable_web_security && request_.request_initiator) {
        const std::vector<std::string> &localSchemes = url::GetLocalSchemes();
        std::string fromScheme = request_.request_initiator->GetTupleOrPrecursorTupleIfOpaque().scheme();
        if (base::Contains(localSchemes, fromScheme)) {
            content::WebContents *wc = webContents();
            std::string toScheme = request_.url.scheme();
            // local schemes must have universal access, or be accessing something local and have local access.
            if (fromScheme != toScheme) {
                // note allow_file_access_from_file_urls maps to LocalContentCanAccessFileUrls in our API
                // and allow_universal_access_from_file_urls to LocalContentCanAccessRemoteUrls, so we are
                // using them as proxies for our API here.
                if (toScheme == "file")
                    allowed_cors_ = wc && wc->GetOrCreateWebPreferences().allow_file_access_from_file_urls;
                else if (!base::Contains(localSchemes, toScheme))
                    allowed_cors_ = wc && wc->GetOrCreateWebPreferences().allow_universal_access_from_file_urls;
                else
                    allowed_cors_ = true; // We should think about this for future patches
            }
        }
    }
}

InterceptedRequest::~InterceptedRequest()
{
    weak_factory_.InvalidateWeakPtrs();
}

content::WebContents* InterceptedRequest::webContents()
{
    if (process_id_) {
        content::RenderFrameHost *frameHost = content::RenderFrameHost::FromID(process_id_, request_.render_frame_id);
        return content::WebContents::FromRenderFrameHost(frameHost);
    }

    return content::WebContents::FromFrameTreeNodeId(request_.render_frame_id);
}

QWebEngineUrlRequestInterceptor* InterceptedRequest::getProfileInterceptor()
{
    return profile_adapter_ ? profile_adapter_->requestInterceptor() : nullptr;
}

QWebEngineUrlRequestInterceptor* InterceptedRequest::getPageInterceptor()
{
    if (auto wc = webContents()) {
        auto view = static_cast<content::WebContentsImpl *>(wc)->GetView();
        if (WebContentsAdapterClient *client = WebContentsViewQt::from(view)->client())
            return client->webContentsAdapter()->requestInterceptor();
    }
    return nullptr;
}

void InterceptedRequest::Restart()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    // This is a CORS check on the from URL, the normal check on the to URL is applied later
    if (!allowed_cors_ && current_response_->response_type == network::mojom::FetchResponseType::kCors) {
        target_client_->OnComplete(network::URLLoaderCompletionStatus(
                                       network::CorsErrorStatus(network::mojom::CorsError::kCorsDisabledScheme)));
        delete this;
        return;
    }

    // MEMO since all codepatch leading to Restart scheduled and executed as asynchronous tasks in main thread,
    //      interceptors may change in meantime and also during intercept call, so they should be resolved anew.
    //      Set here only profile's interceptor since it runs first without going to user code.
    auto profileInterceptor = getProfileInterceptor();
    if (!profileInterceptor && !getPageInterceptor()) {
        ContinueAfterIntercept();
        return;
    }

    auto resourceType = toQt(blink::mojom::ResourceType(request_.resource_type));
    auto navigationType = toQt(pageTransitionToNavigationType(ui::PageTransition(request_.transition_type)));
    const QUrl originalUrl = toQt(request_.url);
    const QUrl initiator = request_.request_initiator.has_value() ? toQt(request_.request_initiator->GetURL()) : QUrl();

    auto wc = webContents();
    GURL top_document_url = wc ? wc->GetVisibleURL() : GURL();
    QUrl firstPartyUrl;
    if (!top_document_url.is_empty())
        firstPartyUrl = toQt(top_document_url);
    else
        firstPartyUrl = toQt(request_.site_for_cookies.first_party_url()); // m_topDocumentUrl can be empty for the main-frame.

    auto info = new QWebEngineUrlRequestInfoPrivate(resourceType, navigationType, originalUrl, firstPartyUrl,
                                                    initiator, QByteArray::fromStdString(request_.method));
    Q_ASSERT(!request_info_);
    request_info_.reset(new QWebEngineUrlRequestInfo(info));

    // TODO: remove for Qt6
    bool isDeprecatedProfileInterceptor = profileInterceptor == nullptr;
    if (profileInterceptor && profileInterceptor->property("deprecated").toBool()) {
        isDeprecatedProfileInterceptor = true;
        // sync call supports depracated call of an interceptor on io thread
        base::WaitableEvent event;
        base::PostTask(FROM_HERE, { content::BrowserThread::IO },
                       base::BindOnce(&InterceptedRequest::InterceptOnIOThread,
                           base::Unretained(this), &event, profileInterceptor));
        event.Wait();
        if (request_info_->changed()) {
            ContinueAfterIntercept();
            return;
        }
    }
    InterceptOnUIThread(isDeprecatedProfileInterceptor ? nullptr : profileInterceptor);
    ContinueAfterIntercept();
}

void InterceptedRequest::InterceptOnIOThread(base::WaitableEvent *event, QWebEngineUrlRequestInterceptor *interceptor)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    interceptor->interceptRequest(*request_info_);
    event->Signal();
}

void InterceptedRequest::InterceptOnUIThread(QWebEngineUrlRequestInterceptor *profileInterceptor)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (profileInterceptor)
        profileInterceptor->interceptRequest(*request_info_);

    if (!request_info_->changed()) {
        if (auto interceptor = getPageInterceptor())
            interceptor->interceptRequest(*request_info_);
    }
}

void InterceptedRequest::ContinueAfterIntercept()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    if (request_info_) {
        // cleanup in scope because of delete this and it's not needed else where after
        decltype(request_info_) scoped_request_info(request_info_.take());
        QWebEngineUrlRequestInfoPrivate &info = *scoped_request_info->d_ptr;

        if (info.changed) {
            if (info.shouldBlockRequest)
                return SendErrorAndCompleteImmediately(net::ERR_BLOCKED_BY_CLIENT);

            for (auto header = info.extraHeaders.constBegin(); header != info.extraHeaders.constEnd(); ++header) {
                std::string h = header.key().toStdString();
                if (base::LowerCaseEqualsASCII(h, "referer")) {
                    request_.referrer = GURL(header.value().toStdString());
                } else {
                    request_.headers.SetHeader(h, header.value().toStdString());
                }
            }

            if (info.shouldRedirectRequest) {
                net::RedirectInfo::FirstPartyURLPolicy first_party_url_policy =
                        request_.update_first_party_url_on_redirect ? net::RedirectInfo::FirstPartyURLPolicy::UPDATE_URL_ON_REDIRECT
                                                                    : net::RedirectInfo::FirstPartyURLPolicy::NEVER_CHANGE_URL;
                net::RedirectInfo redirectInfo = net::RedirectInfo::ComputeRedirectInfo(
                        request_.method, request_.url, request_.site_for_cookies,
                        first_party_url_policy, request_.referrer_policy, request_.referrer.spec(),
                        net::HTTP_TEMPORARY_REDIRECT, toGurl(info.url), base::nullopt,
                        false /*insecure_scheme_was_upgraded*/);

                // FIXME: Should probably create a new header.
                current_response_->encoded_data_length = 0;
                request_.method = redirectInfo.new_method;
                request_.url = redirectInfo.new_url;
                request_.site_for_cookies = redirectInfo.new_site_for_cookies;
                request_.referrer = GURL(redirectInfo.new_referrer);
                request_.referrer_policy = redirectInfo.new_referrer_policy;
                if (request_.method == net::HttpRequestHeaders::kGetMethod)
                    request_.request_body = nullptr;
                target_client_->OnReceiveRedirect(redirectInfo, std::move(current_response_));
                return;
            }
        }
    }

    if (!target_loader_ && target_factory_) {
        target_factory_->CreateLoaderAndStart(target_loader_.BindNewPipeAndPassReceiver(), routing_id_, request_id_,
                                              options_, request_, proxied_client_receiver_.BindNewPipeAndPassRemote(),
                                              traffic_annotation_);
    }
}

// URLLoaderClient methods.

void InterceptedRequest::OnReceiveResponse(network::mojom::URLResponseHeadPtr head)
{
    current_response_ = head.Clone();

    target_client_->OnReceiveResponse(std::move(head));
}

void InterceptedRequest::OnReceiveRedirect(const net::RedirectInfo &redirect_info, network::mojom::URLResponseHeadPtr head)
{
    // TODO(timvolodine): handle redirect override.
    current_response_ = head.Clone();
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
                                        const net::HttpRequestHeaders &modified_cors_exempt_headers,
                                        const base::Optional<GURL> &new_url)
{
    if (target_loader_)
        target_loader_->FollowRedirect(removed_headers, modified_headers, modified_cors_exempt_headers, new_url);

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
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    // Save an error status so that we call onReceiveError at destruction if there
    // was no safe browsing error.
    if (status.error_code != net::OK)
        error_status_ = status.error_code;

    if (target_client_)
        target_client_->OnComplete(status);

    if (proxied_loader_receiver_.is_bound() && wait_for_loader_error) {
        // Since the original client is gone no need to continue loading the
        // request.
        proxied_client_receiver_.reset();
        target_loader_.reset();

        // Don't delete |this| yet, in case the |proxied_loader_receiver_|'s
        // error_handler is called with a reason to indicate an error which we want
        // to send to the client bridge. Also reset |target_client_| so we don't
        // get its error_handler called and then delete |this|.
        target_client_.reset();

        // In case there are pending checks as to whether this request should be
        // intercepted, we don't want that causing |target_client_| to be used
        // later.
        weak_factory_.InvalidateWeakPtrs();
    } else {
        delete this;
    }
}

void InterceptedRequest::SendErrorAndCompleteImmediately(int error_code)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    auto status = network::URLLoaderCompletionStatus(error_code);
    target_client_->OnComplete(status);
    delete this;
}

ProxyingURLLoaderFactoryQt::ProxyingURLLoaderFactoryQt(ProfileAdapter *adapter, int process_id,
                                                       mojo::PendingReceiver<network::mojom::URLLoaderFactory> loader_receiver,
                                                       mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory_info)
    : m_profileAdapter(adapter), m_processId(process_id), m_weakFactory(this)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (target_factory_info) {
        m_targetFactory.Bind(std::move(target_factory_info));
        m_targetFactory.set_disconnect_handler(
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

void ProxyingURLLoaderFactoryQt::CreateLoaderAndStart(mojo::PendingReceiver<network::mojom::URLLoader> loader, int32_t routing_id,
                                                      int32_t request_id, uint32_t options, const network::ResourceRequest &request,
                                                      mojo::PendingRemote<network::mojom::URLLoaderClient> url_loader_client,
                                                      const net::MutableNetworkTrafficAnnotationTag &traffic_annotation)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory_clone;
    if (m_targetFactory)
        m_targetFactory->Clone(target_factory_clone.InitWithNewPipeAndPassReceiver());

    // Will manage its own lifetime
    InterceptedRequest *req = new InterceptedRequest(m_profileAdapter, m_processId, request_id, routing_id, options,
                                                     request, traffic_annotation, std::move(loader),
                                                     std::move(url_loader_client), std::move(target_factory_clone));
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
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    m_proxyReceivers.Add(this, std::move(receiver));
}

} // namespace QtWebEngineCore
