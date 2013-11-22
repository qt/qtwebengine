/*
 *  Copyright (C) 2013 BlackBerry Limited. All rights reserved.
 */

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
void ResourceDispatcherHostLoginDelegateQt::PlatformCreateDialog(const string16& message)
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
                    bool ok = client->authenticationDialog(toQt(message), username, passwd);
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

    string16 explanation = ASCIIToUTF16("The server ") + host
            + ASCIIToUTF16(" requires a username and password.");
    explanation += ASCIIToUTF16(" The server says: ");
    explanation += realm;
    explanation += ASCIIToUTF16(".");

    PlatformCreateDialog(explanation);
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
