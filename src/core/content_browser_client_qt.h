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

#ifndef CONTENT_BROWSER_CLIENT_QT_H
#define CONTENT_BROWSER_CLIENT_QT_H

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "third_party/WebKit/public/platform/WebNotificationPermission.h"

#include <QtCore/qcompilerdetection.h> // Needed for Q_DECL_OVERRIDE

namespace net {
class URLRequestContextGetter;
}

namespace content {
class BrowserContext;
class BrowserMainParts;
class DevToolsManagerDelegate;
class RenderProcessHost;
class RenderViewHostDelegateView;
class ResourceContext;
class WebContentsViewPort;
class WebContents;
struct MainFunctionParams;
}

namespace gfx {
class GLShareGroup;
}

namespace QtWebEngineCore {
class BrowserContextQt;
class BrowserMainPartsQt;
class ResourceDispatcherHostDelegateQt;
class ShareGroupQtQuick;

class ContentBrowserClientQt : public content::ContentBrowserClient {

public:
    ContentBrowserClientQt();
    ~ContentBrowserClientQt();
    static ContentBrowserClientQt* Get();
    virtual content::BrowserMainParts* CreateBrowserMainParts(const content::MainFunctionParams&) Q_DECL_OVERRIDE;
    virtual void RenderProcessWillLaunch(content::RenderProcessHost* host) Q_DECL_OVERRIDE;
    virtual void ResourceDispatcherHostCreated() Q_DECL_OVERRIDE;
    virtual gfx::GLShareGroup* GetInProcessGpuShareGroup() Q_DECL_OVERRIDE;
    virtual content::MediaObserver* GetMediaObserver() Q_DECL_OVERRIDE;
    virtual content::AccessTokenStore* CreateAccessTokenStore() Q_DECL_OVERRIDE;
    virtual content::QuotaPermissionContext *CreateQuotaPermissionContext() Q_DECL_OVERRIDE;
    virtual void OverrideWebkitPrefs(content::RenderViewHost *, const GURL &, content::WebPreferences *) Q_DECL_OVERRIDE;
    virtual void AllowCertificateError(
        int render_process_id,
        int render_frame_id,
        int cert_error,
        const net::SSLInfo& ssl_info,
        const GURL& request_url,
        content::ResourceType resource_type,
        bool overridable,
        bool strict_enforcement,
        bool expired_previous_decision,
        const base::Callback<void(bool)>& callback,
        content::CertificateRequestResultType* result) Q_DECL_OVERRIDE;
    virtual void RequestPermission(
        content::PermissionType permission,
        content::WebContents* web_contents,
        int bridge_id,
        const GURL& requesting_frame,
        bool user_gesture,
        const base::Callback<void(bool)>& result_callback) Q_DECL_OVERRIDE;
    virtual void CancelPermissionRequest(content::PermissionType permission,
                                         content::WebContents* web_contents,
                                         int bridge_id,
                                         const GURL& requesting_frame) Q_DECL_OVERRIDE;
    content::LocationProvider* OverrideSystemLocationProvider() Q_DECL_OVERRIDE;
    content::DevToolsManagerDelegate *GetDevToolsManagerDelegate() Q_DECL_OVERRIDE;
    virtual net::URLRequestContextGetter *CreateRequestContext(content::BrowserContext *browser_context, content::ProtocolHandlerMap *protocol_handlers, content::URLRequestInterceptorScopedVector request_interceptorss) Q_DECL_OVERRIDE;

    virtual blink::WebNotificationPermission CheckDesktopNotificationPermission(const GURL& source_origin, content::ResourceContext* context, int render_process_id)  Q_DECL_OVERRIDE;

    virtual std::string GetApplicationLocale() Q_DECL_OVERRIDE;
    virtual void AppendExtraCommandLineSwitches(base::CommandLine* command_line, int child_process_id) Q_DECL_OVERRIDE;

private:
    BrowserMainPartsQt* m_browserMainParts;
    scoped_ptr<ResourceDispatcherHostDelegateQt> m_resourceDispatcherHostDelegate;
    scoped_refptr<ShareGroupQtQuick> m_shareGroupQtQuick;
};

} // namespace QtWebEngineCore

#endif // CONTENT_BROWSER_CLIENT_QT_H
