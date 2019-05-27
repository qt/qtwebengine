/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "url_request_notification.h"

#include "base/supports_user_data.h"
#include "base/task/post_task.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "net/url_request/url_request.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"
#include "profile_io_data_qt.h"
#include "qwebengineurlrequestinfo_p.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

// Calls cancel() when the URLRequest is destroyed.
class UserData : public base::SupportsUserData::Data {
public:
    UserData(URLRequestNotification *ptr) : m_ptr(ptr) {}
    ~UserData() { m_ptr->cancel(); }
    static const char key[];
private:
    URLRequestNotification *m_ptr;
};

const char UserData::key[] = "QtWebEngineCore::URLRequestNotification";

static content::ResourceType fromQt(QWebEngineUrlRequestInfo::ResourceType resourceType)
{
    return static_cast<content::ResourceType>(resourceType);
}

URLRequestNotification::URLRequestNotification(net::URLRequest *request,
                       bool isMainFrameRequest,
                       GURL *newUrl,
                       QWebEngineUrlRequestInfo &&requestInfo,
                       content::ResourceRequestInfo::WebContentsGetter webContentsGetter,
                       net::CompletionOnceCallback callback,
                       QPointer<ProfileAdapter> adapter)
    : m_request(request)
    , m_isMainFrameRequest(isMainFrameRequest)
    , m_newUrl(newUrl)
    , m_originalUrl(requestInfo.requestUrl())
    , m_requestInfo(std::move(requestInfo))
    , m_webContentsGetter(webContentsGetter)
    , m_callback(std::move(callback))
    , m_profileAdapter(adapter)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    m_request->SetUserData(UserData::key, std::make_unique<UserData>(this));

    base::PostTaskWithTraits(
        FROM_HERE,
        {content::BrowserThread::UI},
        base::BindOnce(&URLRequestNotification::notify, base::Unretained(this)));
}


void URLRequestNotification::notify()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    // May run concurrently with cancel() so no peeking at m_request here.

    int result = net::OK;
    content::WebContents *webContents = m_webContentsGetter.Run();

    if (webContents) {

        if (m_profileAdapter) {
            QWebEngineUrlRequestInterceptor* interceptor = m_profileAdapter->requestInterceptor();
            if (!interceptor->property("deprecated").toBool())
                interceptor->interceptRequest(m_requestInfo);
        }

        WebContentsAdapterClient *client =
            WebContentsViewQt::from(static_cast<content::WebContentsImpl*>(webContents)->GetView())->client();

        if (!m_requestInfo.changed()) {
            client->interceptRequest(m_requestInfo);
        }

        if (m_requestInfo.changed()) {
            result = m_requestInfo.d_ptr->shouldBlockRequest ? net::ERR_BLOCKED_BY_CLIENT : net::OK;
            // We handle the rest of the changes later when we are back in I/O thread
        }

        // Only do navigationRequested on MAIN_FRAME and SUB_FRAME resources
        if (result == net::OK && content::IsResourceTypeFrame(fromQt(m_requestInfo.resourceType()))) {
            int navigationRequestAction = WebContentsAdapterClient::AcceptRequest;
            client->navigationRequested(m_requestInfo.navigationType(),
                                        m_requestInfo.requestUrl(),
                                        navigationRequestAction,
                                        m_isMainFrameRequest);
            result = net::ERR_FAILED;
            switch (static_cast<WebContentsAdapterClient::NavigationRequestAction>(navigationRequestAction)) {
            case WebContentsAdapterClient::AcceptRequest:
                result = net::OK;
                break;
            case WebContentsAdapterClient::IgnoreRequest:
                result = net::ERR_ABORTED;
                break;
            }
            DCHECK(result != net::ERR_FAILED);
        }
    }

    // Run the callback on the IO thread.
    base::PostTaskWithTraits(
        FROM_HERE,
        {content::BrowserThread::IO},
        base::BindOnce(&URLRequestNotification::complete, base::Unretained(this), result));
}

void  URLRequestNotification::cancel()
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    // May run concurrently with notify() but we only touch m_request here.

    m_request = nullptr;
}

void URLRequestNotification::complete(int error)
{
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (m_request) {
        if (m_requestInfo.changed()) {
            if (m_originalUrl != m_requestInfo.d_ptr->url)
                *m_newUrl = toGurl(m_requestInfo.d_ptr->url);

            if (!m_requestInfo.d_ptr->extraHeaders.isEmpty()) {
                auto end = m_requestInfo.d_ptr->extraHeaders.constEnd();
                for (auto header = m_requestInfo.d_ptr->extraHeaders.constBegin(); header != end; ++header) {
                    std::string h = header.key().toStdString();
                    if (base::LowerCaseEqualsASCII(h, "referer")) {
                        m_request->SetReferrer(header.value().toStdString());
                    } else {
                        m_request->SetExtraRequestHeaderByName(h, header.value().toStdString(), /* overwrite */ true);
                    }
                }
            }
        }

        if (m_request->status().status() != net::URLRequestStatus::CANCELED)
            std::move(m_callback).Run(error);
        m_request->RemoveUserData(UserData::key);
    }

    delete this;
}

}
