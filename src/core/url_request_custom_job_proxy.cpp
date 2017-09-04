/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "url_request_custom_job_proxy.h"
#include "url_request_custom_job.h"
#include "url_request_custom_job_delegate.h"
#include "api/qwebengineurlrequestjob.h"
#include "browser_context_adapter.h"
#include "type_conversion.h"
#include "content/public/browser/browser_thread.h"

using namespace net;

namespace QtWebEngineCore {

URLRequestCustomJobProxy::URLRequestCustomJobProxy(URLRequestCustomJob *job)
    : m_mutex(QMutex::Recursive)
    , m_job(job)
    , m_delegate(0)
    , m_error(0)
    , m_started(false)
    , m_asyncInitialized(false)
    , m_weakFactory(this)
{
}

URLRequestCustomJobProxy::~URLRequestCustomJobProxy()
{
    Q_ASSERT(!m_job);
    Q_ASSERT(!m_delegate);
}

void URLRequestCustomJobProxy::killJob()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    QMutexLocker lock(&m_mutex);
    m_job = 0;
    bool doDelete = false;
    if (m_delegate) {
        m_delegate->deleteLater();
    } else {
        // Do not delete yet if startAsync has not yet run.
        doDelete = m_asyncInitialized;
    }
    if (m_device && m_device->isOpen())
        m_device->close();
    m_device = 0;
    m_weakFactory.InvalidateWeakPtrs();
    lock.unlock();
    if (doDelete)
        delete this;
}

void URLRequestCustomJobProxy::unsetJobDelegate()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    m_delegate = 0;
    bool doDelete = false;
    if (m_job)
        abort();
    else
        doDelete = true;
    lock.unlock();
    if (doDelete)
        delete this;
}

void URLRequestCustomJobProxy::setReplyMimeType(const std::string &mimeType)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    m_mimeType = mimeType;
}

void URLRequestCustomJobProxy::setReplyCharset(const std::string &charset)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    m_charset = charset;
}

void URLRequestCustomJobProxy::setReplyDevice(QIODevice *device)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    if (!m_job)
        return;
    m_device = device;
    if (m_device && !m_device->isReadable())
        m_device->open(QIODevice::ReadOnly);

    qint64 size = m_device ? m_device->size() : -1;
    if (size > 0)
        m_job->set_expected_content_size(size);
    if (m_device && m_device->isReadable()) {
        content::BrowserThread::PostTask(
                    content::BrowserThread::IO, FROM_HERE,
                    base::Bind(&URLRequestCustomJobProxy::notifyStarted,
                               m_weakFactory.GetWeakPtr()));
    } else {
        fail(ERR_INVALID_URL);
    }
}

void URLRequestCustomJobProxy::redirect(const GURL &url)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    QMutexLocker lock(&m_mutex);
    if (m_device || m_error)
        return;
    if (!m_job)
        return;
    m_redirect = url;
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::Bind(&URLRequestCustomJobProxy::notifyStarted,
                                                m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJobProxy::abort()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    if (m_device && m_device->isOpen())
        m_device->close();
    m_device = 0;
    if (!m_job)
        return;
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::Bind(&URLRequestCustomJobProxy::notifyCanceled,
                                                m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJobProxy::notifyCanceled()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    QMutexLocker lock(&m_mutex);
    if (!m_job)
        return;
    if (m_started)
        m_job->NotifyCanceled();
    else
        m_job->NotifyStartError(URLRequestStatus(URLRequestStatus::CANCELED, ERR_ABORTED));
}

void URLRequestCustomJobProxy::notifyStarted()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    QMutexLocker lock(&m_mutex);
    if (!m_job)
        return;
    Q_ASSERT(!m_started);
    m_started = true;
    m_job->NotifyHeadersComplete();
}

void URLRequestCustomJobProxy::fail(int error)
{
    QMutexLocker lock(&m_mutex);
    m_error = error;
    if (content::BrowserThread::CurrentlyOn(content::BrowserThread::IO))
        return;
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (!m_job)
        return;
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::Bind(&URLRequestCustomJobProxy::notifyFailure,
                                                m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJobProxy::notifyFailure()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    QMutexLocker lock(&m_mutex);
    if (!m_job)
        return;
    if (m_device)
        m_device->close();
    if (!m_started)
        m_job->NotifyStartError(URLRequestStatus::FromError(m_error));
    // else we fail on the next read, or the read that might already be in progress
}

GURL URLRequestCustomJobProxy::requestUrl()
{
    QMutexLocker lock(&m_mutex);
    if (!m_job)
        return GURL();
    return m_job->request()->url();
}

std::string URLRequestCustomJobProxy::requestMethod()
{
    QMutexLocker lock(&m_mutex);
    if (!m_job)
        return std::string();
    return m_job->request()->method();
}

void URLRequestCustomJobProxy::startAsync()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    Q_ASSERT(!m_started);
    Q_ASSERT(!m_delegate);
    QMutexLocker lock(&m_mutex);
    if (!m_job) {
        lock.unlock();
        delete this;
        return;
    }

    QWebEngineUrlSchemeHandler *schemeHandler = 0;
    QSharedPointer<const BrowserContextAdapter> browserContext = m_job->m_adapter.toStrongRef();
    if (browserContext)
        schemeHandler = browserContext->customUrlSchemeHandlers()[toQByteArray(m_job->m_scheme)];
    if (schemeHandler) {
        m_delegate = new URLRequestCustomJobDelegate(this);
        m_asyncInitialized = true;
        QWebEngineUrlRequestJob *requestJob = new QWebEngineUrlRequestJob(m_delegate);
        schemeHandler->requestStarted(requestJob);
    } else {
        lock.unlock();
        abort();
        delete this;
        return;
    }
}

} // namespace
