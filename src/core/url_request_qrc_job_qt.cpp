/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "url_request_qrc_job_qt.h"
#include "type_conversion.h"

#include "net/base/net_errors.h"

#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

using namespace net;

URLRequestQrcJobQt::URLRequestQrcJobQt(URLRequest* request, NetworkDelegate* network_delegate)
    : URLRequestJob(request, network_delegate)
    , data_offset_(0)
    , weak_factory_(this) {

}

URLRequestQrcJobQt::~URLRequestQrcJobQt() { }

void URLRequestQrcJobQt::Start() {
    // Start reading asynchronously so that all error reporting and data
    // callbacks happen as they would for network requests.
    base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&URLRequestQrcJobQt::StartAsync, weak_factory_.GetWeakPtr()));
}

void URLRequestQrcJobQt::Kill() {
    data_ = NULL;
    weak_factory_.InvalidateWeakPtrs();

    URLRequestJob::Kill();
}

bool URLRequestQrcJobQt::GetMimeType(std::string* mime_type) const {
    if (mime_type_.size() > 0) {
        *mime_type = mime_type_;
        return true;
    }
    return false;
}

bool URLRequestQrcJobQt::ReadRawData(IOBuffer* buf, int buf_size, int* bytes_read) {
    DCHECK(bytes_read);
    if (!data_.get()) {
        return false;
    }
    int remaining = data_->size() - data_offset_;
    if (buf_size > remaining)
        buf_size = remaining;
    memcpy(buf->data(), data_->data() + data_offset_, buf_size);
    data_offset_ += buf_size;
    *bytes_read = buf_size;
    return true;
}

void URLRequestQrcJobQt::StartAsync() {
    if (!request_)
        return;
    int result = OK;
    // Get qrc file path.
    QUrl url = toQt(request_->url());
    QString qrc_file_path = ":" + url.path();
    QFile qrc_file(qrc_file_path);
    QFileInfo qrc_file_info(qrc_file);
    // Get qrc file mime type.
    QMimeDatabase mime_database;
    QMimeType mime_type = mime_database.mimeTypeForFile(qrc_file_info);
    mime_type_ = mime_type.name().toLower().toStdString();
    // Read file data.
    if (!qrc_file.open(QIODevice::ReadOnly)) {
        result = ERR_INVALID_URL;
    } else {
        QByteArray data_array = qrc_file.readAll();
        data_ = new IOBufferWithSize(data_array.size());
        memcpy(data_->data(), data_array.data(), data_array.size());
    }
    OnGetDataCompleted(result);
}

void URLRequestQrcJobQt::OnGetDataCompleted(int result) {
    if (result == OK) {
        // Notify that the headers are complete
        NotifyHeadersComplete();
    } else {
        NotifyStartError(URLRequestStatus(URLRequestStatus::FAILED, result));
    }
}
