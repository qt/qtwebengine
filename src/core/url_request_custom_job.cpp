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

#include "url_request_custom_job.h"
#include "url_request_custom_job_delegate.h"

#include "custom_url_scheme_handler.h"
#include "type_conversion.h"

#include "content/public/browser/browser_thread.h"
#include "net/base/net_errors.h"
#include "net/base/io_buffer.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QUrl>

using namespace net;

namespace QtWebEngineCore {

URLRequestCustomJob::URLRequestCustomJob(URLRequest *request, NetworkDelegate *networkDelegate, CustomUrlSchemeHandler *schemeHandler)
    : URLRequestJob(request, networkDelegate)
    , m_device(0)
    , m_schemeHandler(schemeHandler)
    , m_error(0)
    , m_started(false)
    , m_weakFactory(this)
{
}

URLRequestCustomJob::~URLRequestCustomJob()
{
    QMutexLocker lock(&m_mutex);
    if (m_delegate) {
        m_delegate->m_job = 0;
        m_delegate->deleteLater();
    }
    if (m_device && m_device->isOpen())
        m_device->close();
}

void URLRequestCustomJob::Start()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE, base::Bind(&URLRequestCustomJob::startAsync, m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJob::Kill()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    QMutexLocker lock(&m_mutex);
    if (m_delegate) {
        m_delegate->m_job = 0;
        m_delegate->deleteLater();
    }
    m_delegate = 0;
    if (m_device && m_device->isOpen())
        m_device->close();
    m_device = 0;
    m_weakFactory.InvalidateWeakPtrs();

    URLRequestJob::Kill();
}

bool URLRequestCustomJob::GetMimeType(std::string *mimeType) const
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_mimeType.size() > 0) {
        *mimeType = m_mimeType;
        return true;
    }
    return false;
}

bool URLRequestCustomJob::GetCharset(std::string* charset)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_charset.size() > 0) {
        *charset = m_charset;
        return true;
    }
    return false;
}

bool URLRequestCustomJob::IsRedirectResponse(GURL* location, int* http_status_code)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    QMutexLocker lock(&m_mutex);
    if (m_redirect.is_valid()) {
        *location = m_redirect;
        *http_status_code = 303;
        return true;
    }
    return false;
}

void URLRequestCustomJob::setReplyMimeType(const std::string &mimeType)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    m_mimeType = mimeType;
}

void URLRequestCustomJob::setReplyCharset(const std::string &charset)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    m_charset = charset;
}

void URLRequestCustomJob::setReplyDevice(QIODevice *device)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    m_device = device;
    if (m_device && !m_device->isReadable())
        m_device->open(QIODevice::ReadOnly);

    if (m_device && m_device->isReadable())
        content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyStarted, m_weakFactory.GetWeakPtr()));
    else
        fail(ERR_INVALID_URL);
}

bool URLRequestCustomJob::ReadRawData(IOBuffer *buf, int bufSize, int *bytesRead)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    Q_ASSERT(bytesRead);
    QMutexLocker lock(&m_mutex);
    qint64 rv = m_device ? m_device->read(buf->data(), bufSize) : -1;
    if (rv >= 0) {
        *bytesRead = static_cast<int>(rv);
        return true;
    } else {
        NotifyDone(URLRequestStatus(URLRequestStatus::FAILED, ERR_FAILED));
    }
    return false;
}

void URLRequestCustomJob::redirect(const GURL &url)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    if (m_device || m_error)
        return;

    QMutexLocker lock(&m_mutex);
    m_redirect = url;
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyStarted, m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJob::abort()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    if (m_device && m_device->isOpen())
        m_device->close();
    m_device = 0;
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyCanceled, m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJob::notifyCanceled()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (m_started)
        NotifyDone(URLRequestStatus(URLRequestStatus::CANCELED, ERR_ABORTED));
    else
        NotifyStartError(URLRequestStatus(URLRequestStatus::CANCELED, ERR_ABORTED));
}

void URLRequestCustomJob::notifyStarted()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    Q_ASSERT(!m_started);
    m_started = true;
    NotifyHeadersComplete();
}

void URLRequestCustomJob::fail(int error)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    m_error = error;
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyFailure, m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJob::notifyFailure()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    QMutexLocker lock(&m_mutex);
    if (m_device)
        m_device->close();
    if (m_started)
        NotifyDone(URLRequestStatus(URLRequestStatus::FAILED, m_error));
    else
        NotifyStartError(URLRequestStatus(URLRequestStatus::FAILED, m_error));
}

void URLRequestCustomJob::startAsync()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    Q_ASSERT(!m_started);
    Q_ASSERT(!m_delegate);
    QMutexLocker lock(&m_mutex);
    m_delegate = new URLRequestCustomJobDelegate(this);
    lock.unlock();
    m_schemeHandler->handleJob(m_delegate);
}

} // namespace
