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
#include "url_request_custom_job_proxy.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/io_buffer.h"

#include <QIODevice>

using namespace net;

namespace QtWebEngineCore {

URLRequestCustomJob::URLRequestCustomJob(URLRequest *request,
                                         NetworkDelegate *networkDelegate,
                                         const std::string &scheme,
                                         QWeakPointer<const BrowserContextAdapter> adapter)
    : URLRequestJob(request, networkDelegate)
    , m_scheme(scheme)
    , m_adapter(adapter)
    , m_proxy(new URLRequestCustomJobProxy(this))
{
}

URLRequestCustomJob::~URLRequestCustomJob()
{
    if (m_proxy)
        m_proxy->killJob();
}

static void startAsync(URLRequestCustomJobProxy *proxy)
{
    proxy->startAsync();
}

void URLRequestCustomJob::Start()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
                                     base::Bind(&startAsync, m_proxy));
}

void URLRequestCustomJob::Kill()
{
    if (m_proxy)
        m_proxy->killJob();
    m_proxy = 0;

    URLRequestJob::Kill();
}

bool URLRequestCustomJob::GetMimeType(std::string *mimeType) const
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_proxy)
        return false;
    QMutexLocker lock(&m_proxy->m_mutex);
    if (m_proxy->m_mimeType.size() > 0) {
        *mimeType = m_proxy->m_mimeType;
        return true;
    }
    return false;
}

bool URLRequestCustomJob::GetCharset(std::string* charset)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_proxy)
        return false;
    QMutexLocker lock(&m_proxy->m_mutex);
    if (m_proxy->m_charset.size() > 0) {
        *charset = m_proxy->m_charset;
        return true;
    }
    return false;
}

bool URLRequestCustomJob::IsRedirectResponse(GURL* location, int* http_status_code)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    if (!m_proxy)
        return false;
    QMutexLocker lock(&m_proxy->m_mutex);
    if (m_proxy->m_redirect.is_valid()) {
        *location = m_proxy->m_redirect;
        *http_status_code = 303;
        return true;
    }
    return false;
}

int URLRequestCustomJob::ReadRawData(IOBuffer *buf, int bufSize)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
    Q_ASSERT(m_proxy);
    QMutexLocker lock(&m_proxy->m_mutex);
    if (m_proxy->m_error)
        return m_proxy->m_error;
    qint64 rv = m_proxy->m_device ? m_proxy->m_device->read(buf->data(), bufSize) : -1;
    if (rv >= 0)
        return static_cast<int>(rv);
    else {
        // QIODevice::read might have called fail on us.
        if (m_proxy->m_error)
            return m_proxy->m_error;
        return ERR_FAILED;
    }
}
} // namespace
