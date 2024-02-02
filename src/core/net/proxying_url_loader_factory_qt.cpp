// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "proxying_url_loader_factory_qt.h"

#include <utility>

#include "base/functional/bind.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "net/base/filename_util.h"
#include "net/http/http_status_code.h"
#include "services/network/public/cpp/cors/cors.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/early_hints.mojom.h"
#include "third_party/blink/public/mojom/loader/resource_load_info.mojom-shared.h"
#include "url/url_util.h"
#include "url/url_util_qt.h"

#include "api/qwebengineurlrequestinfo_p.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

// originally based on aw_proxying_url_loader_factory.cc:
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace {
    network::mojom::URLResponseHeadPtr createResponse(const network::ResourceRequest &request) {
        const bool disable_web_security = base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kDisableWebSecurity);
        network::mojom::URLResponseHeadPtr response = network::mojom::URLResponseHead::New();
        response->response_type = network::cors::CalculateResponseType(
            request.mode, disable_web_security || (
            request.request_initiator && request.request_initiator->IsSameOriginWith(url::Origin::Create(request.url))));

        return response;
    }
}

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

static QHash<QByteArray, QByteArray> toQt(const net::HttpRequestHeaders &headers)
{
    const auto vector = headers.GetHeaderVector();
    QHash<QByteArray, QByteArray> hash;

    for (const auto &header : vector) {
        hash.insert(QByteArray::fromStdString(header.key), QByteArray::fromStdString(header.value));
    }

    return hash;
}

// Handles intercepted, in-progress requests/responses, so that they can be
// controlled and modified accordingly.
class InterceptedRequest : public network::mojom::URLLoader
                         , public network::mojom::URLLoaderClient
{
public:
    InterceptedRequest(ProfileAdapter *profile_adapter,
                       int frame_tree_node_id, int32_t request_id, uint32_t options,
                       const network::ResourceRequest &request,
                       const net::MutableNetworkTrafficAnnotationTag &traffic_annotation,
                       mojo::PendingReceiver<network::mojom::URLLoader> loader,
                       mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                       mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory);
    ~InterceptedRequest() override;

    void Restart();

    // network::mojom::URLLoaderClient
    void OnReceiveResponse(network::mojom::URLResponseHeadPtr head, mojo::ScopedDataPipeConsumerHandle, absl::optional<mojo_base::BigBuffer>) override;
    void OnReceiveRedirect(const net::RedirectInfo &redirect_info, network::mojom::URLResponseHeadPtr head) override;
    void OnUploadProgress(int64_t current_position, int64_t total_size, OnUploadProgressCallback callback) override;
    void OnTransferSizeUpdated(int32_t transfer_size_diff) override;
    void OnComplete(const network::URLLoaderCompletionStatus &status) override;
    void OnReceiveEarlyHints(network::mojom::EarlyHintsPtr) override {}

    // network::mojom::URLLoader
    void FollowRedirect(const std::vector<std::string> &removed_headers,
                        const net::HttpRequestHeaders &modified_headers,
                        const net::HttpRequestHeaders &modified_cors_exempt_headers,
                        const absl::optional<GURL> &new_url) override;
    void SetPriority(net::RequestPriority priority, int32_t intra_priority_value) override;
    void PauseReadingBodyFromNet() override;
    void ResumeReadingBodyFromNet() override;

private:
    void InterceptOnUIThread();
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
    const int frame_tree_node_id_;
    const int32_t request_id_;
    const uint32_t options_;
    bool allow_local_ = false;
    bool allow_remote_ = true;
    bool local_access_ = false;
    bool remote_access_ = true;

    bool loader_error_seen_ = false;

    // If the |target_loader_| called OnComplete with an error this stores it.
    // That way the destructor can send it to OnReceivedError if safe browsing
    // error didn't occur.
    int error_status_ = net::OK;
    network::ResourceRequest request_;
    network::mojom::URLResponseHeadPtr current_response_;

    const net::MutableNetworkTrafficAnnotationTag traffic_annotation_;

    struct RequestInfoDeleter
    {
        void operator()(QWebEngineUrlRequestInfo *ptr) const
        { delete ptr; }
    };

    std::unique_ptr<QWebEngineUrlRequestInfo, RequestInfoDeleter> request_info_;

    mojo::Receiver<network::mojom::URLLoader> proxied_loader_receiver_;
    mojo::Remote<network::mojom::URLLoaderClient> target_client_;
    mojo::Receiver<network::mojom::URLLoaderClient> proxied_client_receiver_{this};
    mojo::Remote<network::mojom::URLLoader> target_loader_;
    mojo::Remote<network::mojom::URLLoaderFactory> target_factory_;

    base::WeakPtrFactory<InterceptedRequest> weak_factory_;
};

InterceptedRequest::InterceptedRequest(ProfileAdapter *profile_adapter,
                                       int frame_tree_node_id, int32_t request_id, uint32_t options,
                                       const network::ResourceRequest &request,
                                       const net::MutableNetworkTrafficAnnotationTag &traffic_annotation,
                                       mojo::PendingReceiver<network::mojom::URLLoader> loader_receiver,
                                       mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                                       mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory)
    : profile_adapter_(profile_adapter)
    , frame_tree_node_id_(frame_tree_node_id)
    , request_id_(request_id)
    , options_(options)
    , request_(request)
    , traffic_annotation_(traffic_annotation)
    , proxied_loader_receiver_(this, std::move(loader_receiver))
    , target_client_(std::move(client))
    , target_factory_(std::move(target_factory))
    , weak_factory_(this)
{
    const bool disable_web_security = base::CommandLine::ForCurrentProcess()->HasSwitch(switches::kDisableWebSecurity);
    current_response_ = createResponse(request_);
    // If there is a client error, clean up the request.
    target_client_.set_disconnect_handler(
            base::BindOnce(&InterceptedRequest::OnURLLoaderClientError, base::Unretained(this)));
    proxied_loader_receiver_.set_disconnect_with_reason_handler(
            base::BindOnce(&InterceptedRequest::OnURLLoaderError, base::Unretained(this)));
    if (!disable_web_security && request_.request_initiator) {
        const std::vector<std::string> &localSchemes = url::GetLocalSchemes();
        const std::string fromScheme = request_.request_initiator->GetTupleOrPrecursorTupleIfOpaque().scheme();
        const std::string toScheme = request_.url.scheme();
        const bool fromLocal = base::Contains(localSchemes, fromScheme);
        const bool toLocal = base::Contains(localSchemes, toScheme);
        bool hasLocalAccess = false;
        local_access_ = toLocal;
        remote_access_ = !toLocal && (toScheme != "data") && (toScheme != "qrc");
        if (const url::CustomScheme *cs = url::CustomScheme::FindScheme(fromScheme))
            hasLocalAccess = cs->flags & url::CustomScheme::LocalAccessAllowed;
        if (fromLocal || toLocal) {
            content::WebContents *wc = webContents();
            // local schemes must have universal access, or be accessing something local and have local access.
            allow_local_ = hasLocalAccess || (fromLocal && wc && wc->GetOrCreateWebPreferences().allow_file_access_from_file_urls);
            allow_remote_ = !fromLocal || (wc && wc->GetOrCreateWebPreferences().allow_remote_access_from_local_urls);
        }
    }
}

InterceptedRequest::~InterceptedRequest()
{
    weak_factory_.InvalidateWeakPtrs();
}

content::WebContents* InterceptedRequest::webContents()
{
    if (frame_tree_node_id_ == content::RenderFrameHost::kNoFrameTreeNodeId)
        return nullptr;
    return content::WebContents::FromFrameTreeNodeId(frame_tree_node_id_);
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

    bool granted_special_access = false;
    auto navigationType = toQt(pageTransitionToNavigationType(ui::PageTransition(request_.transition_type)));
    switch (navigationType) {
    case QWebEngineUrlRequestInfo::NavigationTypeLink:
    case QWebEngineUrlRequestInfo::NavigationTypeTyped:
        if (blink::mojom::ResourceType(request_.resource_type) == blink::mojom::ResourceType::kMainFrame && request_.has_user_gesture)
            granted_special_access = true; // allow normal explicit navigation
        break;
    case QWebEngineUrlRequestInfo::NavigationTypeBackForward:
    case QWebEngineUrlRequestInfo::NavigationTypeReload:
        if (blink::mojom::ResourceType(request_.resource_type) == blink::mojom::ResourceType::kMainFrame)
            granted_special_access = true;
        break;
    default:
        break;
    }

    // Check if non-local access is allowed
    if (!allow_remote_ && remote_access_) {
        if (!granted_special_access) {
            target_client_->OnComplete(network::URLLoaderCompletionStatus(net::ERR_NETWORK_ACCESS_DENIED));
            delete this;
            return;
        }
    }

    // Check if local access is allowed
    if (!allow_local_ && local_access_) {
        // Check for specifically granted file access:
        if (auto *frame_tree = content::FrameTreeNode::GloballyFindByID(frame_tree_node_id_)) {
            const int renderer_id = frame_tree->current_frame_host()->GetProcess()->GetID();
            base::FilePath file_path;
            if (net::FileURLToFilePath(request_.url, &file_path)) {
                if (content::ChildProcessSecurityPolicy::GetInstance()->CanReadFile(renderer_id, file_path))
                    granted_special_access = true;
            }
        }
        if (!granted_special_access) {
            target_client_->OnComplete(network::URLLoaderCompletionStatus(net::ERR_ACCESS_DENIED));
            delete this;
            return;
        }
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
    const QUrl originalUrl = toQt(request_.url);
    const QUrl initiator = request_.request_initiator.has_value() ? toQt(request_.request_initiator->GetURL()) : QUrl();

    auto wc = webContents();
    GURL top_document_url = wc ? wc->GetVisibleURL() : GURL();
    QUrl firstPartyUrl;
    if (!top_document_url.is_empty())
        firstPartyUrl = toQt(top_document_url);
    else
        firstPartyUrl = toQt(request_.site_for_cookies.first_party_url()); // m_topDocumentUrl can be empty for the main-frame.

    QHash<QByteArray, QByteArray> headers = toQt(request_.headers);

    if (!request_.referrer.is_empty())
        headers.insert("Referer", toQt(request_.referrer).toEncoded());

    auto info = new QWebEngineUrlRequestInfoPrivate(
            resourceType, navigationType, originalUrl, firstPartyUrl, initiator,
            QByteArray::fromStdString(request_.method), headers);
    Q_ASSERT(!request_info_);
    request_info_.reset(new QWebEngineUrlRequestInfo(info));

    InterceptOnUIThread();
    ContinueAfterIntercept();
}

void InterceptedRequest::InterceptOnUIThread()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (auto interceptor = getProfileInterceptor())
        interceptor->interceptRequest(*request_info_);

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
        const auto scoped_request_info = std::move(request_info_);
        QWebEngineUrlRequestInfoPrivate &info = *scoped_request_info->d_ptr;

        for (auto header = info.extraHeaders.constBegin(); header != info.extraHeaders.constEnd(); ++header) {
            std::string h = header.key().toStdString();
            if (base::EqualsCaseInsensitiveASCII(h, "referer"))
                request_.referrer = GURL(header.value().toStdString());
            else
                request_.headers.SetHeader(h, header.value().toStdString());
        }

        if (info.changed) {
            if (info.shouldBlockRequest)
                return SendErrorAndCompleteImmediately(net::ERR_BLOCKED_BY_CLIENT);

            if (info.shouldRedirectRequest) {
                net::RedirectInfo::FirstPartyURLPolicy first_party_url_policy =
                        request_.update_first_party_url_on_redirect ? net::RedirectInfo::FirstPartyURLPolicy::UPDATE_URL_ON_REDIRECT
                                                                    : net::RedirectInfo::FirstPartyURLPolicy::NEVER_CHANGE_URL;
                net::RedirectInfo redirectInfo = net::RedirectInfo::ComputeRedirectInfo(
                        request_.method, request_.url, request_.site_for_cookies,
                        first_party_url_policy, request_.referrer_policy, request_.referrer.spec(),
                        net::HTTP_TEMPORARY_REDIRECT, toGurl(info.url), absl::nullopt,
                        false /*insecure_scheme_was_upgraded*/);
                request_.method = redirectInfo.new_method;
                request_.url = redirectInfo.new_url;
                request_.site_for_cookies = redirectInfo.new_site_for_cookies;
                request_.referrer = GURL(redirectInfo.new_referrer);
                request_.referrer_policy = redirectInfo.new_referrer_policy;
                if (request_.method == net::HttpRequestHeaders::kGetMethod)
                    request_.request_body = nullptr;
                // In case of multiple sequential rediredts, current_response_ has previously been moved to target_client_
                // so we create a new one using the redirect url.
                if (!current_response_)
                    current_response_ = createResponse(request_);
                current_response_->encoded_data_length = 0;
                target_client_->OnReceiveRedirect(redirectInfo, std::move(current_response_));
                return;
            }
        }
    }

    if (!target_loader_ && target_factory_) {
        loader_error_seen_ = false;
        target_factory_->CreateLoaderAndStart(target_loader_.BindNewPipeAndPassReceiver(), request_id_,
                                              options_, request_, proxied_client_receiver_.BindNewPipeAndPassRemote(),
                                              traffic_annotation_);
    }
}

// URLLoaderClient methods.

void InterceptedRequest::OnReceiveResponse(network::mojom::URLResponseHeadPtr head, mojo::ScopedDataPipeConsumerHandle handle, absl::optional<mojo_base::BigBuffer> buffer)
{
    current_response_ = head.Clone();

    target_client_->OnReceiveResponse(std::move(head), std::move(handle), std::move(buffer));
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

void InterceptedRequest::OnTransferSizeUpdated(int32_t transfer_size_diff)
{
    target_client_->OnTransferSizeUpdated(transfer_size_diff);
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
                                        const absl::optional<GURL> &new_url)
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
    else
        loader_error_seen_ = true;
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

    if (proxied_loader_receiver_.is_bound() && wait_for_loader_error && !loader_error_seen_) {
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

ProxyingURLLoaderFactoryQt::ProxyingURLLoaderFactoryQt(ProfileAdapter *adapter, int frame_tree_node_id,
                                                       mojo::PendingReceiver<network::mojom::URLLoaderFactory> loader_receiver,
                                                       mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory_info)
    : m_profileAdapter(adapter), m_frameTreeNodeId(frame_tree_node_id), m_weakFactory(this)
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

void ProxyingURLLoaderFactoryQt::CreateLoaderAndStart(mojo::PendingReceiver<network::mojom::URLLoader> loader, int32_t request_id,
                                                      uint32_t options, const network::ResourceRequest &request,
                                                      mojo::PendingRemote<network::mojom::URLLoaderClient> url_loader_client,
                                                      const net::MutableNetworkTrafficAnnotationTag &traffic_annotation)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    mojo::PendingRemote<network::mojom::URLLoaderFactory> target_factory_clone;
    if (m_targetFactory)
        m_targetFactory->Clone(target_factory_clone.InitWithNewPipeAndPassReceiver());

    // Will manage its own lifetime
    InterceptedRequest *req = new InterceptedRequest(m_profileAdapter, m_frameTreeNodeId, request_id, options,
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
