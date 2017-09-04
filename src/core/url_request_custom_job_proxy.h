/****************************************************************************
**
** Copyright (C) 2017 Company Ltd.
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

#ifndef URL_REQUEST_CUSTOM_JOB_PROXY_H_
#define URL_REQUEST_CUSTOM_JOB_PROXY_H_

#include "url/gurl.h"
#include "base/memory/weak_ptr.h"
#include <QtCore/QMutex>
#include <QtCore/QPointer>


QT_FORWARD_DECLARE_CLASS(QIODevice)

namespace QtWebEngineCore {

class URLRequestCustomJob;
class URLRequestCustomJobDelegate;

// Used to comunicate between URLRequestCustomJob living on the IO thread
// and URLRequestCustomJobDelegate living on the UI thread.
class URLRequestCustomJobProxy {
public:
    URLRequestCustomJobProxy(URLRequestCustomJob *job);
    ~URLRequestCustomJobProxy();

    void setReplyMimeType(const std::string &);
    void setReplyCharset(const std::string &);
    void setReplyDevice(QIODevice *);

    void redirect(const GURL &url);
    void fail(int);
    void abort();

    void killJob();
    void unsetJobDelegate();

    void startAsync();
    void notifyStarted();
    void notifyFailure();
    void notifyCanceled();

    GURL requestUrl();
    std::string requestMethod();

    QMutex m_mutex;
    QPointer<QIODevice> m_device;
    URLRequestCustomJob *m_job;
    URLRequestCustomJobDelegate *m_delegate;
    std::string m_mimeType;
    std::string m_charset;
    int m_error;
    GURL m_redirect;
    bool m_started;
    bool m_asyncInitialized;
    base::WeakPtrFactory<URLRequestCustomJobProxy> m_weakFactory;
};

} // namespace QtWebEngineCore

#endif // URL_REQUEST_CUSTOM_JOB_PROXY_H_
