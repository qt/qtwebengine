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
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browser_context_qt.h"

#include "browser_context_adapter.h"
#include "download_manager_delegate_qt.h"
#include "permission_manager_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "resource_context_qt.h"
#include "ssl_host_state_delegate_qt.h"
#include "type_conversion.h"
#include "url_request_context_getter_qt.h"

#include "base/time/time.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "net/proxy/proxy_config_service.h"

namespace QtWebEngineCore {

BrowserContextQt::BrowserContextQt(BrowserContextAdapter *adapter)
    : m_adapter(adapter)
{
}

BrowserContextQt::~BrowserContextQt()
{
    if (resourceContext)
        content::BrowserThread::DeleteSoon(content::BrowserThread::IO, FROM_HERE, resourceContext.release());
}

base::FilePath BrowserContextQt::GetPath() const
{
    return toFilePath(m_adapter->dataPath());
}

base::FilePath BrowserContextQt::GetCachePath() const
{
    return toFilePath(m_adapter->cachePath());
}

bool BrowserContextQt::IsOffTheRecord() const
{
    return m_adapter->isOffTheRecord();
}

net::URLRequestContextGetter *BrowserContextQt::GetRequestContext()
{
    return url_request_getter_.get();
}

net::URLRequestContextGetter *BrowserContextQt::GetRequestContextForRenderProcess(int)
{
    return GetRequestContext();
}

net::URLRequestContextGetter *BrowserContextQt::GetMediaRequestContext()
{
    return GetRequestContext();
}

net::URLRequestContextGetter *BrowserContextQt::GetMediaRequestContextForRenderProcess(int)
{
    return GetRequestContext();
}

net::URLRequestContextGetter *BrowserContextQt::GetMediaRequestContextForStoragePartition(const base::FilePath&, bool)
{
    return GetRequestContext();
}

content::ResourceContext *BrowserContextQt::GetResourceContext()
{
    if (!resourceContext)
        resourceContext.reset(new ResourceContextQt(this));
    return resourceContext.get();
}

content::DownloadManagerDelegate *BrowserContextQt::GetDownloadManagerDelegate()
{
    return m_adapter->downloadManagerDelegate();
}

content::BrowserPluginGuestManager *BrowserContextQt::GetGuestManager()
{
    return 0;
}

storage::SpecialStoragePolicy *BrowserContextQt::GetSpecialStoragePolicy()
{
    QT_NOT_YET_IMPLEMENTED
    return 0;
}

content::PushMessagingService *BrowserContextQt::GetPushMessagingService()
{
    return 0;
}

content::SSLHostStateDelegate* BrowserContextQt::GetSSLHostStateDelegate()
{
    if (!sslHostStateDelegate)
        sslHostStateDelegate.reset(new SSLHostStateDelegateQt());
    return sslHostStateDelegate.get();
}

scoped_ptr<content::ZoomLevelDelegate> BrowserContextQt::CreateZoomLevelDelegate(const base::FilePath&)
{
    return nullptr;
}

content::PermissionManager *BrowserContextQt::GetPermissionManager()
{
    if (!permissionManager)
        permissionManager.reset(new PermissionManagerQt());
    return permissionManager.get();
}

net::URLRequestContextGetter *BrowserContextQt::CreateRequestContext(content::ProtocolHandlerMap *protocol_handlers, content::URLRequestInterceptorScopedVector request_interceptors)
{
    url_request_getter_ = new URLRequestContextGetterQt(m_adapter->sharedFromThis(), protocol_handlers, request_interceptors.Pass());
    return url_request_getter_.get();
}

} // namespace QtWebEngineCore
