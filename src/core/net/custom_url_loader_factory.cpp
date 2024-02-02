// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "custom_url_loader_factory.h"

#include "base/strings/stringprintf.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/system/simple_watcher.h"
#include "net/base/net_errors.h"
#include "net/http/http_status_code.h"
#include "net/http/http_util.h"
#include "services/network/public/cpp/cors/cors.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "url/url_util.h"
#include "url/url_util_qt.h"

#include "api/qwebengineurlscheme.h"
#include "net/url_request_custom_job_proxy.h"
#include "profile_adapter.h"
#include "type_conversion.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qmimedatabase.h>
#include <QtCore/qmimedata.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {

namespace {

class CustomURLLoader : public network::mojom::URLLoader
                      , private URLRequestCustomJobProxy::Client
{
public:
    static void CreateAndStart(const network::ResourceRequest &request,
                               mojo::PendingReceiver<network::mojom::URLLoader> loader,
                               mojo::PendingRemote<network::mojom::URLLoaderClient> client_remote,
                               QPointer<ProfileAdapter> profileAdapter)
    {
        // CustomURLLoader will handle its own life-cycle, and delete when
        // the client lets go.
        auto *customUrlLoader = new CustomURLLoader(request, std::move(loader), std::move(client_remote), profileAdapter);
        customUrlLoader->Start();
    }

    // network::mojom::URLLoader:
    void FollowRedirect(const std::vector<std::string> &removed_headers,
                        const net::HttpRequestHeaders &modified_headers,
                        const net::HttpRequestHeaders &modified_cors_exempt_headers, // FIXME: do something with this?
                        const absl::optional<GURL> &new_url) override
    {
        // We can be asked for follow our own redirect
        scoped_refptr<URLRequestCustomJobProxy> proxy = new URLRequestCustomJobProxy(this, m_proxy->m_scheme, m_proxy->m_profileAdapter);
        m_proxy->m_client = nullptr;
//        m_taskRunner->PostTask(FROM_HERE, base::BindOnce(&URLRequestCustomJobProxy::release, m_proxy));
        content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
                       base::BindOnce(&URLRequestCustomJobProxy::release, m_proxy));
        m_proxy = std::move(proxy);
        if (new_url)
            m_request.url = *new_url;
        else
            m_request.url = m_redirect;
        m_redirect = GURL();
        for (const std::string &header: removed_headers)
            m_request.headers.RemoveHeader(header);
        m_request.headers.MergeFrom(modified_headers);
        Start();
    }
    void SetPriority(net::RequestPriority priority, int32_t intra_priority_value) override { }
    void PauseReadingBodyFromNet() override { }
    void ResumeReadingBodyFromNet() override { }

private:
    CustomURLLoader(const network::ResourceRequest &request,
                    mojo::PendingReceiver<network::mojom::URLLoader> loader,
                    mojo::PendingRemote<network::mojom::URLLoaderClient> client_remote,
                    QPointer<ProfileAdapter> profileAdapter)
        // ### We can opt to run the url-loader on the UI thread instead
        : m_taskRunner(content::GetIOThreadTaskRunner({}))
        , m_proxy(new URLRequestCustomJobProxy(this, request.url.scheme(), profileAdapter))
        , m_receiver(this, std::move(loader))
        , m_client(std::move(client_remote))
        , m_request(request)
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        m_receiver.set_disconnect_handler(
                    base::BindOnce(&CustomURLLoader::OnConnectionError, m_weakPtrFactory.GetWeakPtr()));
        m_firstBytePosition = 0;
        m_device = nullptr;
        m_error = 0;
        QWebEngineUrlScheme scheme = QWebEngineUrlScheme::schemeByName(QByteArray::fromStdString(request.url.scheme()));
        m_corsEnabled = scheme.flags().testFlag(QWebEngineUrlScheme::CorsEnabled);
        m_isLocal = scheme.flags().testFlag(QWebEngineUrlScheme::LocalScheme);
    }

    ~CustomURLLoader() override = default;

    void Start()
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());

        if (network::cors::IsCorsEnabledRequestMode(m_request.mode)) {
            // CORS mode requires a valid request_initiator.
            if (!m_request.request_initiator)
                return CompleteWithFailure(net::ERR_INVALID_ARGUMENT);

            if (m_isLocal) {
                std::string fromScheme = m_request.request_initiator->GetTupleOrPrecursorTupleIfOpaque().scheme();
                const std::vector<std::string> &localSchemes = url::GetLocalSchemes();
                bool fromLocal = base::Contains(localSchemes, fromScheme);
                bool hasLocalAccess = fromLocal;
                if (const url::CustomScheme *cs = url::CustomScheme::FindScheme(fromScheme))
                    hasLocalAccess = cs->flags & (url::CustomScheme::LocalAccessAllowed | url::CustomScheme::Local);
                if (!hasLocalAccess)
                    return CompleteWithFailure(net::ERR_ACCESS_DENIED);
            } else if (!m_corsEnabled && !m_request.request_initiator->IsSameOriginWith(url::Origin::Create(m_request.url))) {
                // Custom schemes are not covered by CorsURLLoader, so we need to reject CORS requests manually.
                 return CompleteWithFailure(network::CorsErrorStatus(network::mojom::CorsError::kCorsDisabledScheme));
            }
        }

        if (mojo::CreateDataPipe(nullptr, m_pipeProducerHandle, m_pipeConsumerHandle) != MOJO_RESULT_OK)
            return CompleteWithFailure(net::ERR_FAILED);

        m_head = network::mojom::URLResponseHead::New();
        m_head->request_start = base::TimeTicks::Now();

        if (!m_pipeConsumerHandle.is_valid())
            return CompleteWithFailure(net::ERR_FAILED);

        std::map<std::string, std::string> headers;
        net::HttpRequestHeaders::Iterator it(m_request.headers);
        while (it.GetNext())
            headers.emplace(it.name(), it.value());
        if (!m_request.referrer.is_empty())
            headers.emplace("Referer", m_request.referrer.spec());

        std::string rangeHeader;
        if (ParseRange(m_request.headers))
            m_firstBytePosition = m_byteRange.first_byte_position();

//        m_taskRunner->PostTask(FROM_HERE,
        content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
                       base::BindOnce(&URLRequestCustomJobProxy::initialize, m_proxy,
                                      m_request.url, m_request.method, m_request.request_initiator, std::move(headers)));
    }

    void CompleteWithFailure(network::CorsErrorStatus cors_error)
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        m_client->OnComplete(network::URLLoaderCompletionStatus(cors_error));
        ClearProxyAndClient(false);
    }

    void CompleteWithFailure(net::Error net_error)
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        m_client->OnComplete(network::URLLoaderCompletionStatus(net_error));
        ClearProxyAndClient(false);
    }

    void OnConnectionError()
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        m_receiver.reset();
        if (m_client.is_bound())
            ClearProxyAndClient(false);
        else
            delete this;
    }

    void OnTransferComplete(MojoResult result)
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        if (result == MOJO_RESULT_OK) {
            network::URLLoaderCompletionStatus status(net::OK);
            status.encoded_data_length = m_totalBytesRead + m_headerBytesRead;
            status.encoded_body_length = m_totalBytesRead;
            status.decoded_body_length = m_totalBytesRead;
            m_client->OnComplete(status);
        } else {
            m_client->OnComplete(network::URLLoaderCompletionStatus(net::ERR_FAILED));
        }
        ClearProxyAndClient(false /* result == MOJO_RESULT_OK */);
    }

    void ClearProxyAndClient(bool wait_for_loader_error = false)
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        m_proxy->m_client = nullptr;
        m_client.reset();
        if (m_device && m_device->isOpen())
            m_device->close();
        m_device = nullptr;
//        m_taskRunner->PostTask(FROM_HERE, base::BindOnce(&URLRequestCustomJobProxy::release, m_proxy));
        content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
                       base::BindOnce(&URLRequestCustomJobProxy::release, m_proxy));
        if (!wait_for_loader_error || !m_receiver.is_bound())
            delete this;
    }

    // URLRequestCustomJobProxy::Client:
    void notifyExpectedContentSize(qint64 size) override
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        m_totalSize = size;
        if (m_byteRange.IsValid()) {
            if (!m_byteRange.ComputeBounds(size)) {
                CompleteWithFailure(net::ERR_REQUEST_RANGE_NOT_SATISFIABLE);
            } else {
                m_maxBytesToRead = m_byteRange.last_byte_position() - m_byteRange.first_byte_position() + 1;
                m_head->content_length = m_maxBytesToRead;
            }
        } else {
            m_head->content_length = size;
        }
    }
    void notifyHeadersComplete() override
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        DCHECK(!m_error);
        m_head->response_start = base::TimeTicks::Now();

        std::string headers;
        if (!m_redirect.is_empty()) {
            headers += "HTTP/1.1 303 See Other\n";
            headers += base::StringPrintf("Location: %s\n", m_redirect.spec().c_str());
        } else {
            if (m_byteRange.IsValid() && m_totalSize > 0) {
                headers += "HTTP/1.1 206 Partial Content\n";
                headers += net::HttpResponseHeaders::kContentRange;
                headers += base::StringPrintf(": bytes %lld-%lld/%lld",
                                              qlonglong{m_byteRange.first_byte_position()},
                                              qlonglong{m_byteRange.last_byte_position()},
                                              qlonglong{m_totalSize});
                headers += "\n";
            } else {
                headers += "HTTP/1.1 200 OK\n";
            }
            if (m_mimeType.size() > 0) {
                headers += net::HttpRequestHeaders::kContentType;
                headers += base::StringPrintf(": %s", m_mimeType.c_str());
                if (m_charset.size() > 0)
                    headers += base::StringPrintf("; charset=%s", m_charset.c_str());
                headers += "\n";
            }
        }
        if (m_corsEnabled) {
            std::string origin;
            if (m_request.headers.GetHeader("Origin", &origin)) {
                headers += base::StringPrintf("Access-Control-Allow-Origin: %s\n", origin.c_str());
                headers += "Access-Control-Allow-Credentials: true\n";
            }
        }
        for (auto it = m_additionalResponseHeaders.cbegin();
             it != m_additionalResponseHeaders.cend(); ++it) {
            headers += it.key().toLower().toStdString() + ": " + it.value().toLower().toStdString()
                    + "\n";
        }
        m_head->headers = base::MakeRefCounted<net::HttpResponseHeaders>(net::HttpUtil::AssembleRawHeaders(headers));
        m_head->encoded_data_length = m_head->headers->raw_headers().length();

        if (!m_redirect.is_empty()) {
            m_head->content_length = {};
            m_head->encoded_body_length = {};
            net::RedirectInfo::FirstPartyURLPolicy first_party_url_policy =
                    m_request.update_first_party_url_on_redirect ? net::RedirectInfo::FirstPartyURLPolicy::UPDATE_URL_ON_REDIRECT
                                                                 : net::RedirectInfo::FirstPartyURLPolicy::NEVER_CHANGE_URL;
            net::RedirectInfo redirectInfo = net::RedirectInfo::ComputeRedirectInfo(
                        m_request.method, m_request.url,
                        m_request.site_for_cookies,
                        first_party_url_policy, m_request.referrer_policy,
                        m_request.referrer.spec(), net::HTTP_SEE_OTHER,
                        m_redirect, absl::nullopt, false /*insecure_scheme_was_upgraded*/);
            m_client->OnReceiveRedirect(redirectInfo, std::move(m_head));
            m_head = nullptr;
            // ### should m_request be updated with RedirectInfo? (see FollowRedirect)
            return;
        }
        DCHECK(m_device);
        m_head->mime_type = m_mimeType;
        m_head->charset = m_charset;
        m_headerBytesRead = m_head->headers->raw_headers().length();
        m_client->OnReceiveResponse(std::move(m_head), std::move(m_pipeConsumerHandle), absl::nullopt);
        m_head = nullptr;

        m_watcher = std::make_unique<mojo::SimpleWatcher>(
                FROM_HERE, mojo::SimpleWatcher::ArmingPolicy::MANUAL, m_taskRunner);
        m_watcher->Watch(m_pipeProducerHandle.get(), MOJO_HANDLE_SIGNAL_WRITABLE,
                         MOJO_WATCH_CONDITION_SATISFIED,
                         base::BindRepeating(&CustomURLLoader::notifyReadyWrite,
                                             m_weakPtrFactory.GetWeakPtr()));

        readAvailableData(); // May delete this
    }
    void notifyCanceled() override
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        OnTransferComplete(MOJO_RESULT_CANCELLED);
    }
    void notifyAborted() override
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        notifyStartFailure(net::ERR_ABORTED);
    }
    void notifyStartFailure(int error) override
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        m_head->response_start = base::TimeTicks::Now();
        std::string headers;
        switch (error) {
        case net::ERR_INVALID_URL:
            headers = "HTTP/1.1 400 Bad Request\n";
            break;
        case net::ERR_FILE_NOT_FOUND:
            headers = "HTTP/1.1 404 Not Found\n";
            break;
        case net::ERR_ABORTED:
            headers = "HTTP/1.1 503 Request Aborted\n";
            break;
        case net::ERR_ACCESS_DENIED:
            headers = "HTTP/1.1 403 Forbidden\n";
            break;
        case net::ERR_FAILED:
            headers = "HTTP/1.1 400 Request Failed\n";
            break;
        default:
            headers = "HTTP/1.1 500 Internal Error\n";
            break;
        }
        m_head->headers = base::MakeRefCounted<net::HttpResponseHeaders>(net::HttpUtil::AssembleRawHeaders(headers));
        m_head->encoded_data_length = m_head->headers->raw_headers().length();
        m_head->content_length = {};
        m_head->encoded_body_length = {};
        m_client->OnReceiveResponse(std::move(m_head), mojo::ScopedDataPipeConsumerHandle(), absl::nullopt);
        CompleteWithFailure(net::Error(error));
    }
    void notifyReadyRead() override
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        readAvailableData();
    }
    void notifyReadyWrite(MojoResult result, const mojo::HandleSignalsState &state)
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        if (result != MOJO_RESULT_OK) {
            CompleteWithFailure(net::ERR_FAILED);
            return;
        }
        readAvailableData();
    }
    bool readAvailableData()
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        for (;;) {
            if (m_error || !m_device)
                break;

            void *buffer = nullptr;
            uint32_t bufferSize = 0;
            MojoResult beginResult = m_pipeProducerHandle->BeginWriteData(
                    &buffer, &bufferSize, MOJO_BEGIN_WRITE_DATA_FLAG_NONE);
            if (beginResult == MOJO_RESULT_SHOULD_WAIT) {
                m_watcher->ArmOrNotify();
                return false; // Wait for pipe watcher
            }
            if (beginResult != MOJO_RESULT_OK)
                break;
            if (m_maxBytesToRead > 0 && m_maxBytesToRead <= int64_t{std::numeric_limits<uint32_t>::max()})
                bufferSize = std::min(bufferSize, uint32_t(m_maxBytesToRead));

            int readResult = m_device->read(static_cast<char *>(buffer), bufferSize);
            uint32_t bytesRead = std::max(readResult, 0);
            m_pipeProducerHandle->EndWriteData(bytesRead);
            m_totalBytesRead += bytesRead;
            m_client->OnTransferSizeUpdated(m_totalBytesRead);

            const bool deviceAtEnd = m_device->atEnd();
            if ((deviceAtEnd && !m_device->isSequential())
                || (m_maxBytesToRead > 0 && m_totalBytesRead >= m_maxBytesToRead)) {
                OnTransferComplete(MOJO_RESULT_OK);
                return true; // Done with reading
            }

            if (readResult == 0)
                return false; // Wait for readyRead
            if (readResult < 0 && deviceAtEnd && m_device->isSequential()) {
                // Failure on read, and sequential device claiming to be at end, so treat it as a successful end-of-data.
                OnTransferComplete(MOJO_RESULT_OK);
                return true; // Done with reading
            }
            if (readResult < 0)
                break;
        }

        CompleteWithFailure(m_error ? net::Error(m_error) : net::ERR_FAILED);
        return true; // Done with reading
    }
    bool ParseRange(const net::HttpRequestHeaders &headers)
    {
        std::string range_header;
        if (headers.GetHeader(net::HttpRequestHeaders::kRange, &range_header)) {
            std::vector<net::HttpByteRange> ranges;
            if (net::HttpUtil::ParseRangeHeader(range_header, &ranges)) {
                // Chromium doesn't support multirange requests.
                if (ranges.size() == 1) {
                    m_byteRange = ranges[0];
                    return true;
                }
            }
        }
        return false;
    }
    base::SequencedTaskRunner *taskRunner() override
    {
        DCHECK(m_taskRunner->RunsTasksInCurrentSequence());
        return m_taskRunner.get();
    }

    scoped_refptr<base::SequencedTaskRunner> m_taskRunner;
    scoped_refptr<URLRequestCustomJobProxy> m_proxy;

    mojo::Receiver<network::mojom::URLLoader> m_receiver;
    mojo::Remote<network::mojom::URLLoaderClient> m_client;
    mojo::ScopedDataPipeProducerHandle m_pipeProducerHandle;
    mojo::ScopedDataPipeConsumerHandle m_pipeConsumerHandle;
    std::unique_ptr<mojo::SimpleWatcher> m_watcher;

    net::HttpByteRange m_byteRange;
    int64_t m_totalSize = 0;
    int64_t m_maxBytesToRead = -1;
    network::ResourceRequest m_request;
    network::mojom::URLResponseHeadPtr m_head;
    qint64 m_headerBytesRead = 0;
    qint64 m_totalBytesRead = 0;
    bool m_corsEnabled;
    bool m_isLocal;

    base::WeakPtrFactory<CustomURLLoader> m_weakPtrFactory{this};
};

class CustomURLLoaderFactory : public network::mojom::URLLoaderFactory {
public:
    CustomURLLoaderFactory(ProfileAdapter *profileAdapter, mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver)
        : m_taskRunner(content::GetIOThreadTaskRunner({}))
        , m_profileAdapter(profileAdapter)
    {
        m_receivers.set_disconnect_handler(base::BindRepeating(
            &CustomURLLoaderFactory::OnDisconnect, base::Unretained(this)));
        m_receivers.Add(this, std::move(receiver));
    }
    ~CustomURLLoaderFactory() override = default;

    // network::mojom::URLLoaderFactory:
    void CreateLoaderAndStart(mojo::PendingReceiver<network::mojom::URLLoader> loader,
                              int32_t request_id,
                              uint32_t options,
                              const network::ResourceRequest &request,
                              mojo::PendingRemote<network::mojom::URLLoaderClient> client,
                              const net::MutableNetworkTrafficAnnotationTag &traffic_annotation) override
    {
        DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
        Q_UNUSED(request_id);
        Q_UNUSED(options);
        Q_UNUSED(traffic_annotation);

        m_taskRunner->PostTask(FROM_HERE,
                               base::BindOnce(&CustomURLLoader::CreateAndStart, request,
                                              std::move(loader), std::move(client),
                                              m_profileAdapter));

    }

    void Clone(mojo::PendingReceiver<network::mojom::URLLoaderFactory> receiver) override
    {
        m_receivers.Add(this, std::move(receiver));
    }

    void OnDisconnect()
    {
        if (m_receivers.empty())
            delete this;
    }

    static mojo::PendingRemote<network::mojom::URLLoaderFactory> Create(ProfileAdapter *profileAdapter)
    {
        mojo::PendingRemote<network::mojom::URLLoaderFactory> pending_remote;
        new CustomURLLoaderFactory(profileAdapter, pending_remote.InitWithNewPipeAndPassReceiver());
        return pending_remote;
    }

    const scoped_refptr<base::SequencedTaskRunner> m_taskRunner;
    mojo::ReceiverSet<network::mojom::URLLoaderFactory> m_receivers;
    QPointer<ProfileAdapter> m_profileAdapter;
};

} // namespace

mojo::PendingRemote<network::mojom::URLLoaderFactory> CreateCustomURLLoaderFactory(ProfileAdapter *profileAdapter)
{
    return CustomURLLoaderFactory::Create(profileAdapter);
}

} // namespace QtWebEngineCore

