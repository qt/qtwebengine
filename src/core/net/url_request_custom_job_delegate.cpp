// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "url_request_custom_job_delegate.h"
#include "url_request_custom_job_proxy.h"

#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/net_errors.h"

#include "type_conversion.h"

#include <QByteArray>

namespace QtWebEngineCore {

URLRequestCustomJobDelegate::URLRequestCustomJobDelegate(URLRequestCustomJobProxy *proxy,
                                                         const QUrl &url,
                                                         const QByteArray &method,
                                                         const QUrl &initiatorOrigin,
                                                         const QMap<QByteArray, QByteArray> &headers)
    : m_proxy(proxy),
      m_request(url),
      m_method(method),
      m_initiatorOrigin(initiatorOrigin),
      m_requestHeaders(headers)
{
}

URLRequestCustomJobDelegate::~URLRequestCustomJobDelegate()
{
}

QUrl URLRequestCustomJobDelegate::url() const
{
    return m_request;
}

QByteArray URLRequestCustomJobDelegate::method() const
{
    return m_method;
}

QUrl URLRequestCustomJobDelegate::initiator() const
{
    return m_initiatorOrigin;
}

QMap<QByteArray, QByteArray> URLRequestCustomJobDelegate::requestHeaders() const
{
    return m_requestHeaders;
}

void URLRequestCustomJobDelegate::setAdditionalResponseHeaders(
        const QMultiMap<QByteArray, QByteArray> &additionalResponseHeaders)
{
    m_additionalResponseHeaders = additionalResponseHeaders;
}

void URLRequestCustomJobDelegate::reply(const QByteArray &contentType, QIODevice *device)
{
    if (device)
        QObject::connect(device, &QIODevice::readyRead, this, &URLRequestCustomJobDelegate::slotReadyRead);
    m_proxy->m_ioTaskRunner->PostTask(FROM_HERE,
                                      base::BindOnce(&URLRequestCustomJobProxy::reply, m_proxy,
                                                     contentType.toStdString(), device,
                                                     std::move(m_additionalResponseHeaders)));
}

void URLRequestCustomJobDelegate::slotReadyRead()
{
    m_proxy->m_ioTaskRunner->PostTask(FROM_HERE,
                                      base::BindOnce(&URLRequestCustomJobProxy::readyRead, m_proxy));
}

void URLRequestCustomJobDelegate::abort()
{
    m_proxy->m_ioTaskRunner->PostTask(FROM_HERE,
                                      base::BindOnce(&URLRequestCustomJobProxy::abort, m_proxy));
}

void URLRequestCustomJobDelegate::redirect(const QUrl &url)
{
    m_proxy->m_ioTaskRunner->PostTask(FROM_HERE,
                                      base::BindOnce(&URLRequestCustomJobProxy::redirect, m_proxy, toGurl(url)));
}

void URLRequestCustomJobDelegate::fail(Error error)
{
    int net_error = 0;
    switch (error) {
    case NoError:
        break;
    case UrlInvalid:
        net_error = net::ERR_INVALID_URL;
        break;
    case UrlNotFound:
        net_error = net::ERR_FILE_NOT_FOUND;
        break;
    case RequestAborted:
        net_error = net::ERR_ABORTED;
        break;
    case RequestDenied:
        net_error = net::ERR_ACCESS_DENIED;
        break;
    case RequestFailed:
        net_error = net::ERR_FAILED;
        break;
    }
    if (net_error) {
        m_proxy->m_ioTaskRunner->PostTask(FROM_HERE,
                                          base::BindOnce(&URLRequestCustomJobProxy::fail, m_proxy, net_error));
    }
}

} // namespace
