// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "url_request_custom_job_proxy.h"
#include "url_request_custom_job_delegate.h"

#include "content/public/browser/browser_thread.h"
#include "net/base/net_errors.h"

#include "api/qwebengineurlrequestjob.h"
#include "profile_adapter.h"
#include "type_conversion.h"
#include "web_engine_context.h"

namespace QtWebEngineCore {

URLRequestCustomJobProxy::URLRequestCustomJobProxy(URLRequestCustomJobProxy::Client *client,
                                                   const std::string &scheme,
                                                   QPointer<ProfileAdapter> profileAdapter)
    : m_client(client)
    , m_started(false)
    , m_scheme(scheme)
    , m_delegate(nullptr)
    , m_profileAdapter(profileAdapter)
    , m_ioTaskRunner(m_client->taskRunner())
{
    DCHECK(m_ioTaskRunner && m_ioTaskRunner->RunsTasksInCurrentSequence());
}

URLRequestCustomJobProxy::~URLRequestCustomJobProxy()
{
}

void URLRequestCustomJobProxy::release()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (m_delegate) {
        m_delegate->deleteLater();
        m_delegate = nullptr;
    }
}

void URLRequestCustomJobProxy::reply(std::string contentType, QIODevice *device)
{
    if (!m_client)
        return;
    DCHECK (!m_ioTaskRunner || m_ioTaskRunner->RunsTasksInCurrentSequence());
    QByteArray qcontentType = QByteArray::fromStdString(contentType).toLower();
    const int sidx = qcontentType.indexOf(';');
    if (sidx > 0) {
        const int cidx = qcontentType.indexOf("charset=", sidx);
        if (cidx > 0) {
            m_client->m_charset = qcontentType.mid(cidx + 8).toStdString();
            qcontentType = qcontentType.first(sidx);
        } else {
            qWarning() << "QWebEngineUrlRequestJob::reply(): Unrecognized content-type format with ';'" << qcontentType;
        }
    }
    m_client->m_mimeType = qcontentType.toStdString();
    m_client->m_device = device;
    if (m_client->m_device && !m_client->m_device->isReadable())
        m_client->m_device->open(QIODevice::ReadOnly);

    if (m_client->m_device && m_client->m_firstBytePosition > 0)
        m_client->m_device->seek(m_client->m_firstBytePosition);

    qint64 deviceSize = m_client->m_device ? m_client->m_device->size() : -1;
    if (deviceSize > 0)
        m_client->notifyExpectedContentSize(deviceSize);

    if (m_client->m_device && m_client->m_device->isReadable()) {
        m_started = true;
        m_client->notifyHeadersComplete();
    } else {
        fail(net::ERR_INVALID_URL);
    }
}

void URLRequestCustomJobProxy::redirect(GURL url)
{
    if (!m_client)
        return;
    DCHECK (!m_ioTaskRunner || m_ioTaskRunner->RunsTasksInCurrentSequence());
    if (m_client->m_device || m_client->m_error)
        return;
    m_client->m_redirect = url;
    m_started = true;
    m_client->notifyHeadersComplete();
}

void URLRequestCustomJobProxy::abort()
{
    if (!m_client)
        return;
    DCHECK (!m_ioTaskRunner || m_ioTaskRunner->RunsTasksInCurrentSequence());
    if (m_client->m_device && m_client->m_device->isOpen())
        m_client->m_device->close();
    m_client->m_device = nullptr;
    if (m_started)
        m_client->notifyCanceled();
    else
        m_client->notifyAborted();
}

void URLRequestCustomJobProxy::fail(int error)
{
    if (!m_client)
        return;
    DCHECK (m_ioTaskRunner->RunsTasksInCurrentSequence());
    m_client->m_error = error;
    if (m_client->m_device)
        m_client->m_device->close();
    if (!m_started)
        m_client->notifyStartFailure(error);
    // else we fail on the next read, or the read that might already be in progress
}

void URLRequestCustomJobProxy::readyRead()
{
    DCHECK (m_ioTaskRunner->RunsTasksInCurrentSequence());
    if (m_client)
        m_client->notifyReadyRead();
}

void URLRequestCustomJobProxy::initialize(GURL url, std::string method,
                                          absl::optional<url::Origin> initiator,
                                          std::map<std::string, std::string> headers)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    Q_ASSERT(!m_delegate);

    QUrl initiatorOrigin;
    if (initiator.has_value())
        initiatorOrigin = QUrl::fromEncoded(QByteArray::fromStdString(initiator.value().Serialize()));

    QWebEngineUrlSchemeHandler *schemeHandler = nullptr;

    if (m_profileAdapter)
        schemeHandler = m_profileAdapter->urlSchemeHandler(toQByteArray(m_scheme));
    QMap<QByteArray, QByteArray> qHeaders;
    for (auto it = headers.cbegin(); it != headers.cend(); ++it)
        qHeaders.insert(toQByteArray(it->first), toQByteArray(it->second));

    if (schemeHandler) {
        m_delegate = new URLRequestCustomJobDelegate(this, toQt(url),
                                                     QByteArray::fromStdString(method),
                                                     initiatorOrigin,
                                                     qHeaders);
        QWebEngineUrlRequestJob *requestJob = new QWebEngineUrlRequestJob(m_delegate);
        schemeHandler->requestStarted(requestJob);
    }
}

} // namespace
