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

#include "content_browser_client_qt.h"

#include "base/message_loop/message_loop.h"
#include "base/threading/thread_restrictions.h"
#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/quota_permission_context.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "ui/base/ui_base_switches.h"
#include "ui/gfx/screen.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"

#include "access_token_store_qt.h"
#include "browser_context_adapter.h"
#include "browser_context_qt.h"
#include "certificate_error_controller.h"
#include "certificate_error_controller_p.h"
#include "desktop_screen_qt.h"
#include "dev_tools_http_handler_delegate_qt.h"
#ifdef QT_USE_POSITIONING
#include "location_provider_qt.h"
#endif
#include "media_capture_devices_dispatcher.h"
#include "resource_dispatcher_host_delegate_qt.h"
#include "user_script_controller_host.h"
#include "web_contents_delegate_qt.h"
#include "web_engine_context.h"
#include "web_engine_library_info.h"

#include <QGuiApplication>
#include <QLocale>
#include <QOpenGLContext>
#include <qpa/qplatformnativeinterface.h>

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE

namespace QtWebEngineCore {

namespace {

ContentBrowserClientQt* gBrowserClient = 0; // Owned by ContentMainDelegateQt.

// Return a timeout suitable for the glib loop, -1 to block forever,
// 0 to return right away, or a timeout in milliseconds from now.
int GetTimeIntervalMilliseconds(const base::TimeTicks& from) {
  if (from.is_null())
    return -1;

  // Be careful here.  TimeDelta has a precision of microseconds, but we want a
  // value in milliseconds.  If there are 5.5ms left, should the delay be 5 or
  // 6?  It should be 6 to avoid executing delayed work too early.
  int delay = static_cast<int>(
      ceil((from - base::TimeTicks::Now()).InMillisecondsF()));

  // If this value is negative, then we need to run delayed work soon.
  return delay < 0 ? 0 : delay;
}

class MessagePumpForUIQt : public QObject,
                           public base::MessagePump
{
public:
    MessagePumpForUIQt()
        // Usually this gets passed through Run, but since we have
        // our own event loop, attach it explicitly ourselves.
        : m_delegate(base::MessageLoopForUI::current())
        , m_explicitLoop(0)
        , m_timerId(0)
    {
    }

    virtual void Run(Delegate *delegate) Q_DECL_OVERRIDE
    {
        Q_ASSERT(delegate == m_delegate);
        // This is used only when MessagePumpForUIQt is used outside of the GUI thread.
        QEventLoop loop;
        m_explicitLoop = &loop;
        loop.exec();
        m_explicitLoop = 0;
    }

    virtual void Quit() Q_DECL_OVERRIDE
    {
        Q_ASSERT(m_explicitLoop);
        m_explicitLoop->quit();
    }

    virtual void ScheduleWork() Q_DECL_OVERRIDE
    {
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

    virtual void ScheduleDelayedWork(const base::TimeTicks &delayed_work_time) Q_DECL_OVERRIDE
    {
        if (delayed_work_time.is_null()) {
            killTimer(m_timerId);
            m_timerId = 0;
            m_timerScheduledTime = base::TimeTicks();
        } else if (!m_timerId || delayed_work_time < m_timerScheduledTime) {
            killTimer(m_timerId);
            m_timerId = startTimer(GetTimeIntervalMilliseconds(delayed_work_time));
            m_timerScheduledTime = delayed_work_time;
        }
    }

protected:
    virtual void customEvent(QEvent *ev) Q_DECL_OVERRIDE
    {
        if (handleScheduledWork())
            QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

    virtual void timerEvent(QTimerEvent *ev) Q_DECL_OVERRIDE
    {
        Q_ASSERT(m_timerId == ev->timerId());
        killTimer(m_timerId);
        m_timerId = 0;
        m_timerScheduledTime = base::TimeTicks();

        base::TimeTicks next_delayed_work_time;
        m_delegate->DoDelayedWork(&next_delayed_work_time);
        ScheduleDelayedWork(next_delayed_work_time);
    }

private:
    bool handleScheduledWork() {
        bool more_work_is_plausible = m_delegate->DoWork();

        base::TimeTicks delayed_work_time;
        more_work_is_plausible |= m_delegate->DoDelayedWork(&delayed_work_time);

        if (more_work_is_plausible)
            return true;

        more_work_is_plausible |= m_delegate->DoIdleWork();
        if (!more_work_is_plausible)
            ScheduleDelayedWork(delayed_work_time);

        return more_work_is_plausible;
    }

    Delegate *m_delegate;
    QEventLoop *m_explicitLoop;
    int m_timerId;
    base::TimeTicks m_timerScheduledTime;
};

scoped_ptr<base::MessagePump> messagePumpFactory()
{
    return scoped_ptr<base::MessagePump>(new MessagePumpForUIQt);
}

} // namespace

class BrowserMainPartsQt : public content::BrowserMainParts
{
public:
    BrowserMainPartsQt()
        : content::BrowserMainParts()
    { }

    void PreMainMessageLoopStart() Q_DECL_OVERRIDE
    {
        base::MessageLoop::InitMessagePumpForUIFactory(messagePumpFactory);
    }

    void PreMainMessageLoopRun() Q_DECL_OVERRIDE
    {
    }

    void PostMainMessageLoopRun()
    {
        // The BrowserContext's destructor uses the MessageLoop so it should be deleted
        // right before the RenderProcessHostImpl's destructor destroys it.
        WebEngineContext::current()->destroyBrowserContext();
    }

    int PreCreateThreads() Q_DECL_OVERRIDE
    {
        base::ThreadRestrictions::SetIOAllowed(true);
        // Like ChromeBrowserMainExtraPartsAura::PreCreateThreads does.
        gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, new DesktopScreenQt);
        return 0;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(BrowserMainPartsQt);
};

class QtShareGLContext : public gfx::GLContext {
public:
    QtShareGLContext(QOpenGLContext *qtContext)
        : gfx::GLContext(0)
        , m_handle(0)
    {
        QString platform = qApp->platformName().toLower();
        QPlatformNativeInterface *pni = QGuiApplication::platformNativeInterface();
        if (platform == QLatin1String("xcb")) {
            if (gfx::GetGLImplementation() == gfx::kGLImplementationEGLGLES2)
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
            else
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("glxcontext"), qtContext);
        } else if (platform == QLatin1String("cocoa"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("cglcontextobj"), qtContext);
        else if (platform == QLatin1String("qnx"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else if (platform == QLatin1String("eglfs") || platform == QLatin1String("wayland"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else if (platform == QLatin1String("windows")) {
            if (gfx::GetGLImplementation() == gfx::kGLImplementationEGLGLES2)
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglContext"), qtContext);
            else
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("renderingcontext"), qtContext);
        } else {
            qFatal("%s platform not yet supported", platform.toLatin1().constData());
            // Add missing platforms once they work.
            Q_UNREACHABLE();
        }
    }

    virtual void* GetHandle() Q_DECL_OVERRIDE { return m_handle; }
    // Qt currently never creates contexts using robustness attributes.
    virtual bool WasAllocatedUsingRobustnessExtension() { return false; }

    // We don't care about the rest, this context shouldn't be used except for its handle.
    virtual bool Initialize(gfx::GLSurface *, gfx::GpuPreference) Q_DECL_OVERRIDE { Q_UNREACHABLE(); return false; }
    virtual void Destroy() Q_DECL_OVERRIDE { Q_UNREACHABLE(); }
    virtual bool MakeCurrent(gfx::GLSurface *) Q_DECL_OVERRIDE { Q_UNREACHABLE(); return false; }
    virtual void ReleaseCurrent(gfx::GLSurface *) Q_DECL_OVERRIDE { Q_UNREACHABLE(); }
    virtual bool IsCurrent(gfx::GLSurface *) Q_DECL_OVERRIDE { Q_UNREACHABLE(); return false; }
    virtual void SetSwapInterval(int) Q_DECL_OVERRIDE { Q_UNREACHABLE(); }

private:
    void *m_handle;
};

class ShareGroupQtQuick : public gfx::GLShareGroup {
public:
    virtual gfx::GLContext* GetContext() Q_DECL_OVERRIDE { return m_shareContextQtQuick.get(); }
    virtual void AboutToAddFirstContext() Q_DECL_OVERRIDE;

private:
    scoped_refptr<QtShareGLContext> m_shareContextQtQuick;
};

void ShareGroupQtQuick::AboutToAddFirstContext()
{
    // This currently has to be setup by ::main in all applications using QQuickWebEngineView with delegated rendering.
    QOpenGLContext *shareContext = qt_gl_global_share_context();
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure to call QtWebEngine::initialize() in your main() function.");
    }
    m_shareContextQtQuick = make_scoped_refptr(new QtShareGLContext(shareContext));
}

class QuotaPermissionContextQt : public content::QuotaPermissionContext {
public:
    virtual void RequestQuotaPermission(const content::StorageQuotaParams &params, int render_process_id, const PermissionCallback &callback) Q_DECL_OVERRIDE
    {
        Q_UNUSED(params);
        Q_UNUSED(render_process_id);
        callback.Run(QUOTA_PERMISSION_RESPONSE_DISALLOW);
    }
};

ContentBrowserClientQt::ContentBrowserClientQt()
    : m_browserMainParts(0)
{
    Q_ASSERT(!gBrowserClient);
    gBrowserClient = this;
}

ContentBrowserClientQt::~ContentBrowserClientQt()
{
    gBrowserClient = 0;
}

ContentBrowserClientQt *ContentBrowserClientQt::Get()
{
    return gBrowserClient;
}

content::BrowserMainParts *ContentBrowserClientQt::CreateBrowserMainParts(const content::MainFunctionParams&)
{
    m_browserMainParts = new BrowserMainPartsQt;
    return m_browserMainParts;
}

void ContentBrowserClientQt::RenderProcessWillLaunch(content::RenderProcessHost* host)
{
    // FIXME: Add a settings variable to enable/disable the file scheme.
    content::ChildProcessSecurityPolicy::GetInstance()->GrantScheme(host->GetID(), url::kFileScheme);
    static_cast<BrowserContextQt*>(host->GetBrowserContext())->m_adapter->userScriptController()->renderProcessStartedWithHost(host);
}

void ContentBrowserClientQt::ResourceDispatcherHostCreated()
{
    m_resourceDispatcherHostDelegate.reset(new ResourceDispatcherHostDelegateQt);
    content::ResourceDispatcherHost::Get()->SetDelegate(m_resourceDispatcherHostDelegate.get());
}

gfx::GLShareGroup *ContentBrowserClientQt::GetInProcessGpuShareGroup()
{
    if (!m_shareGroupQtQuick.get())
        m_shareGroupQtQuick = new ShareGroupQtQuick;
    return m_shareGroupQtQuick.get();
}

content::MediaObserver *ContentBrowserClientQt::GetMediaObserver()
{
    return MediaCaptureDevicesDispatcher::GetInstance();
}

void ContentBrowserClientQt::OverrideWebkitPrefs(content::RenderViewHost *rvh, const GURL &url, content::WebPreferences *web_prefs)
{
    Q_UNUSED(url);
    if (content::WebContents *webContents = rvh->GetDelegate()->GetAsWebContents())
        static_cast<WebContentsDelegateQt*>(webContents->GetDelegate())->overrideWebPreferences(webContents, web_prefs);
}

content::AccessTokenStore *ContentBrowserClientQt::CreateAccessTokenStore()
{
    return new AccessTokenStoreQt;
}

net::URLRequestContextGetter* ContentBrowserClientQt::CreateRequestContext(content::BrowserContext* browser_context, content::ProtocolHandlerMap* protocol_handlers, content::URLRequestInterceptorScopedVector request_interceptors)
{
    return static_cast<BrowserContextQt*>(browser_context)->CreateRequestContext(protocol_handlers);
}

content::QuotaPermissionContext *ContentBrowserClientQt::CreateQuotaPermissionContext()
{
    return new QuotaPermissionContextQt;
}

void ContentBrowserClientQt::AllowCertificateError(int render_process_id, int render_frame_id, int cert_error,
                                                   const net::SSLInfo& ssl_info, const GURL& request_url,
                                                   content::ResourceType resource_type,
                                                   bool overridable, bool strict_enforcement,
                                                   bool expired_previous_decision,
                                                   const base::Callback<void(bool)>& callback,
                                                   content::CertificateRequestResultType* result)
{
    // We leave the result with its default value.
    Q_UNUSED(result);

    content::RenderFrameHost *frameHost = content::RenderFrameHost::FromID(render_process_id, render_frame_id);
    WebContentsDelegateQt* contentsDelegate = 0;
    if (content::WebContents *webContents = frameHost->GetRenderViewHost()->GetDelegate()->GetAsWebContents())
        contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());

    QSharedPointer<CertificateErrorController> errorController(new CertificateErrorController(new CertificateErrorControllerPrivate(cert_error, ssl_info, request_url, resource_type, overridable, strict_enforcement, callback)));
    contentsDelegate->allowCertificateError(errorController);
}

void ContentBrowserClientQt::RequestPermission(content::PermissionType permission,
                                                content::WebContents* web_contents,
                                                int bridge_id,
                                                const GURL& requesting_frame,
                                                bool user_gesture,
                                                const base::Callback<void(bool)>& result_callback)
{
    Q_UNUSED(bridge_id);
    Q_UNUSED(user_gesture);
    WebContentsDelegateQt* contentsDelegate = static_cast<WebContentsDelegateQt*>(web_contents->GetDelegate());
    Q_ASSERT(contentsDelegate);
    if (permission == content::PERMISSION_GEOLOCATION)
        contentsDelegate->requestGeolocationPermission(requesting_frame, result_callback);
    else
        result_callback.Run(false);
}


void ContentBrowserClientQt::CancelPermissionRequest(content::PermissionType permission,
                                                     content::WebContents* web_contents,
                                                     int bridge_id,
                                                     const GURL& requesting_frame)
{
    Q_UNUSED(bridge_id);
    WebContentsDelegateQt* contentsDelegate = static_cast<WebContentsDelegateQt*>(web_contents->GetDelegate());
    Q_ASSERT(contentsDelegate);
    if (permission == content::PERMISSION_GEOLOCATION)
        contentsDelegate->cancelGeolocationPermissionRequest(requesting_frame);
}

blink::WebNotificationPermission ContentBrowserClientQt::CheckDesktopNotificationPermission(const GURL&, content::ResourceContext *, int )
{
    return blink::WebNotificationPermission::WebNotificationPermissionDenied;
}

content::LocationProvider *ContentBrowserClientQt::OverrideSystemLocationProvider()
{
#ifdef QT_USE_POSITIONING
    return new LocationProviderQt;
#else
    return 0; // Leave it up to Chromium to figure something out.
#endif
}

std::string ContentBrowserClientQt::GetApplicationLocale()
{
    return WebEngineLibraryInfo::getApplicationLocale();
}

void ContentBrowserClientQt::AppendExtraCommandLineSwitches(base::CommandLine* command_line, int child_process_id)
{
    Q_UNUSED(child_process_id);

    std::string processType = command_line->GetSwitchValueASCII(switches::kProcessType);
    if (processType == switches::kZygoteProcess)
        command_line->AppendSwitchASCII(switches::kLang, GetApplicationLocale());
}

content::DevToolsManagerDelegate* ContentBrowserClientQt::GetDevToolsManagerDelegate()
{
    return new DevToolsManagerDelegateQt;
}

} // namespace QtWebEngineCore
