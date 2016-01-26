/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "url_request_custom_job.h"
#include "url_request_custom_job_delegate.h"

#include "api/qwebengineurlrequestjob.h"
#include "api/qwebengineurlschemehandler.h"
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

URLRequestCustomJob::URLRequestCustomJob(URLRequest *request, NetworkDelegate *networkDelegate, QWebEngineUrlSchemeHandler *schemeHandler)
    : URLRequestJob(request, networkDelegate)
    , m_device(0)
    , m_schemeHandler(schemeHandler)
    , m_error(0)
    , m_started(false)
    , m_weakFactoryIO(this)
    , m_weakFactoryUI(this)
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
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE, base::Bind(&URLRequestCustomJob::startAsync, m_weakFactoryIO.GetWeakPtr()));
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
    m_weakFactoryIO.InvalidateWeakPtrs();
    m_weakFactoryUI.InvalidateWeakPtrs();

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
        content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyStarted, m_weakFactoryUI.GetWeakPtr()));
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
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyStarted, m_weakFactoryUI.GetWeakPtr()));
}

void URLRequestCustomJob::abort()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    QMutexLocker lock(&m_mutex);
    if (m_device && m_device->isOpen())
        m_device->close();
    m_device = 0;
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyCanceled, m_weakFactoryUI.GetWeakPtr()));
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
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyFailure, m_weakFactoryUI.GetWeakPtr()));
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
    QWebEngineUrlRequestJob *requestJob = new QWebEngineUrlRequestJob(m_delegate);
    m_schemeHandler->requestStarted(requestJob);
}

} // namespace
