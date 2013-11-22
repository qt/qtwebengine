/*
 *  Copyright (C) 2013 BlackBerry Limited. All rights reserved.
 */

#ifndef RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_QT_H_
#define RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_QT_H_

#include "base/compiler_specific.h"
#include "base/strings/string16.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"

#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE


namespace net
{
    class AuthChallengeInfo;
    class URLRequest;
}

class WebContentsAdapterClient;

// This class provides a dialog box to ask the user for credentials. Useful in
// ResourceDispatcherHostDelegate::CreateLoginDelegate.
class ResourceDispatcherHostLoginDelegateQt: public content::ResourceDispatcherHostLoginDelegate
{
public:
    // Threading: IO thread.
    ResourceDispatcherHostLoginDelegateQt(net::AuthChallengeInfo* auth_info, net::URLRequest* request);

    // ResourceDispatcherHostLoginDelegate implementation:
    // Threading: IO thread.
    virtual void OnRequestCancelled() Q_DECL_OVERRIDE;

    // Called by the platform specific code when the user responds. Public because
    // the aforementioned platform specific code may not have access to private
    // members. Not to be called from client code.
    // Threading: UI thread.
    void UserAcceptedAuth(const string16& username, const string16& password);

    void UserCancelledAuth();

protected:
    // Threading: any
    virtual ~ResourceDispatcherHostLoginDelegateQt();

private:
    // All the methods that begin with Platform need to be implemented by the
    // platform specific LoginDialog implementation.
    // Creates the dialog.
    // Threading: UI thread.
    void PlatformCreateDialog(const string16& message);

    // Called from the destructor to let each platform do any necessary cleanup.
    // Threading: UI thread.
    void PlatformCleanUp();

    // Called from OnRequestCancelled if the request was cancelled.
    // Threading: UI thread.
    void PlatformRequestCancelled();

    // Sets up dialog creation.
    // Threading: UI thread.
    void PrepDialog(const string16& host, const string16& realm);

    // Sends the authentication to the requester.
    // Threading: IO thread.
    void SendAuthToRequester(bool success, const string16& username, const string16& password);

private:
    // The request that wants login data.
    // Threading: IO thread.
    net::URLRequest* m_urlRequest;

    WebContentsAdapterClient *m_webContentsAdapterClient;
};


#endif  // RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_QT_H_
