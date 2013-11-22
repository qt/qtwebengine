/*
 *  Copyright (C) 2013 BlackBerry Limited. All rights reserved.
 */

#include "resource_dispatcher_host_delegate_qt.h"
#include "resource_dispatcher_host_login_delegate_qt.h"
#include "web_contents_adapter_client.h"

#include "grit/net_resources.h"
#include "net/base/net_module.h"

ResourceDispatcherHostDelegateQt::ResourceDispatcherHostDelegateQt()
{
}

ResourceDispatcherHostDelegateQt::~ResourceDispatcherHostDelegateQt()
{
}

bool ResourceDispatcherHostDelegateQt::AcceptAuthRequest(net::URLRequest* request, net::AuthChallengeInfo* auth_info)
{
    return true;
}

content::ResourceDispatcherHostLoginDelegate* ResourceDispatcherHostDelegateQt::CreateLoginDelegate(net::AuthChallengeInfo* auth_info, net::URLRequest* request)
{
    return new ResourceDispatcherHostLoginDelegateQt(auth_info, request);
}
