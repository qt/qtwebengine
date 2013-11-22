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
#include "resource_dispatcher_host_login_delegate_qt.h"
#include "web_contents_adapter_client.h"
#include "web_contents_view_qt.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "net/base/auth.h"
#include "net/url_request/url_request.h"
#include "type_conversion.h"


ResourceDispatcherHostLoginDelegateQt::ResourceDispatcherHostLoginDelegateQt(net::AuthChallengeInfo* auth_info, net::URLRequest* request)
    : m_urlRequest(request)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    content::BrowserThread::PostTask(
            content::BrowserThread::UI, FROM_HERE,
            base::Bind(&ResourceDispatcherHostLoginDelegateQt::PrepDialog, this,
                    ASCIIToUTF16(auth_info->challenger.ToString()),
                    UTF8ToUTF16(auth_info->realm)));
}

void ResourceDispatcherHostLoginDelegateQt::OnRequestCancelled()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
            base::Bind(&ResourceDispatcherHostLoginDelegateQt::PlatformRequestCancelled, this));
}

void ResourceDispatcherHostLoginDelegateQt::UserAcceptedAuth(const string16& username, const string16& password)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
            base::Bind(&ResourceDispatcherHostLoginDelegateQt::SendAuthToRequester, this, true,
                    username, password));
}

void ResourceDispatcherHostLoginDelegateQt::UserCancelledAuth()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
            base::Bind(&ResourceDispatcherHostLoginDelegateQt::SendAuthToRequester, this, false,
                    string16(), string16()));

    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
            base::Bind(&ResourceDispatcherHostLoginDelegateQt::PlatformCleanUp, this));
}

ResourceDispatcherHostLoginDelegateQt::~ResourceDispatcherHostLoginDelegateQt()
{
    // Cannot post any tasks here; this object is going away and cannot be
    // referenced/dereferenced.
}

// Bogus implementations for linking. They are never called because
// ResourceDispatcherHostDelegate::CreateLoginDelegate returns NULL.
// TODO: implement ResourceDispatcherHostLoginDelegateQt for other platforms, drop this #if
void ResourceDispatcherHostLoginDelegateQt::PlatformCreateDialog(const string16& host, const string16& realm)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    int render_process_id;
    int render_view_id;
    content::ResourceRequestInfo::ForRequest(m_urlRequest)->GetAssociatedRenderView(&render_process_id, &render_view_id);

    content::WebContents* web_contents = NULL;
    content::RenderViewHost* render_view_host = content::RenderViewHost::FromID(render_process_id, render_view_id);
    if (render_view_host) {
        web_contents = content::WebContents::FromRenderViewHost(render_view_host);
        if (web_contents) {
            WebContentsViewQt *webcontentViewQt = WebContentsViewQt::from(web_contents->GetView());
            if (webcontentViewQt) {
                WebContentsAdapterClient *client = webcontentViewQt->client();
                if (client) {
                    QString username, passwd;
                    bool ok = client->authenticationDialog(toQt(host), toQt(realm), username, passwd);
                    if (ok) {
                        UserAcceptedAuth(toString16(username), toString16(passwd));
                    } else {
                        UserCancelledAuth();
                    }
                }
            }
        }
    }
}
void ResourceDispatcherHostLoginDelegateQt::PlatformCleanUp()
{
}
void ResourceDispatcherHostLoginDelegateQt::PlatformRequestCancelled()
{
}

void ResourceDispatcherHostLoginDelegateQt::PrepDialog(const string16& host, const string16& realm)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    PlatformCreateDialog(host, realm);
}

void ResourceDispatcherHostLoginDelegateQt::SendAuthToRequester(bool success, const string16& username, const string16& password)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    if (success) {
        m_urlRequest->SetAuth(net::AuthCredentials(username, password));
    } else {
        m_urlRequest->CancelAuth();
    }
    content::ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(m_urlRequest);

    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
            base::Bind(&ResourceDispatcherHostLoginDelegateQt::PlatformCleanUp, this));
}
