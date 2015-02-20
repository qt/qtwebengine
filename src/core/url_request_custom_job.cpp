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
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
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

URLRequestCustomJob::URLRequestCustomJob(URLRequest *request, NetworkDelegate *networkDelegate, CustomUrlSchemeHandler *schemeHandler)
    : URLRequestJob(request, networkDelegate)
    , m_device(0)
    , m_schemeHandler(schemeHandler)
    , m_weakFactory(this)
{
}

URLRequestCustomJob::~URLRequestCustomJob()
{
    if (m_device && m_device->isOpen())
        m_device->close();
}

void URLRequestCustomJob::Start()
{
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE, base::Bind(&URLRequestCustomJob::startAsync, m_weakFactory.GetWeakPtr()));
}

void URLRequestCustomJob::Kill()
{
    if (m_device && m_device->isOpen())
        m_device->close();
    m_weakFactory.InvalidateWeakPtrs();

    URLRequestJob::Kill();
}

bool URLRequestCustomJob::GetMimeType(std::string *mimeType) const
{
    if (m_mimeType.size() > 0) {
        *mimeType = m_mimeType;
        return true;
    }
    return false;
}

bool URLRequestCustomJob::GetCharset(std::string* charset)
{
    if (m_charset.size() > 0) {
        *charset = m_charset;
        return true;
    }
    return false;
}

void URLRequestCustomJob::setReplyMimeType(const std::string &mimeType)
{
    m_mimeType = mimeType;
}

void URLRequestCustomJob::setReplyCharset(const std::string &charset)
{
    m_charset = charset;
}

void URLRequestCustomJob::setReplyDevice(QIODevice *device)
{
    m_device = device;
    if (m_device && !m_device->isReadable())
        m_device->open(QIODevice::ReadOnly);
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE, base::Bind(&URLRequestCustomJob::notifyStarted, m_weakFactory.GetWeakPtr()));
}

bool URLRequestCustomJob::ReadRawData(IOBuffer *buf, int bufSize, int *bytesRead)
{
    Q_ASSERT(bytesRead);
    qint64 rv = m_device ? m_device->read(buf->data(), bufSize) : -1;
    if (rv >= 0) {
        *bytesRead = static_cast<int>(rv);
        return true;
    } else {
        NotifyDone(URLRequestStatus(URLRequestStatus::FAILED, ERR_FAILED));
    }
    return false;
}

void URLRequestCustomJob::notifyStarted()
{
    if (m_device && m_device->isReadable())
        NotifyHeadersComplete();
    else
        NotifyStartError(URLRequestStatus(URLRequestStatus::FAILED, ERR_INVALID_URL));
}

void URLRequestCustomJob::startAsync()
{
    m_delegate.reset(new URLRequestCustomJobDelegate(this));
    m_schemeHandler->handleJob(m_delegate.get());
}
