/*
 *  Copyright (C) 2013 BlackBerry Limited. All rights reserved.
 */

#ifndef RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H_
#define RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H_

#include "content/public/browser/resource_dispatcher_host_delegate.h"

#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE

namespace net
{
    class URLRequest;
    class AuthChallengeInfo;
}

class ResourceDispatcherHostDelegateQt: public content::ResourceDispatcherHostDelegate
{
public:
    ResourceDispatcherHostDelegateQt();
    virtual ~ResourceDispatcherHostDelegateQt();

    virtual bool AcceptAuthRequest(net::URLRequest* request, net::AuthChallengeInfo* auth_info) Q_DECL_OVERRIDE;

    virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(net::AuthChallengeInfo* auth_info, net::URLRequest* request) Q_DECL_OVERRIDE;

private:
    DISALLOW_COPY_AND_ASSIGN (ResourceDispatcherHostDelegateQt);
};


#endif  // RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H_
