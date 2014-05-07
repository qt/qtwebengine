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

#include "content_browser_client_qt.h"

#include "base/message_loop/message_loop.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "ui/gfx/screen.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"

#include "browser_context_qt.h"
#include "desktop_screen_qt.h"
#include "dev_tools_http_handler_delegate_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "resource_dispatcher_host_delegate_qt.h"
#include "web_contents_view_qt.h"

#include <QGuiApplication>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtQuick/private/qsgcontext_p.h>
#include <qpa/qplatformnativeinterface.h>

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
    {
    }

    virtual void Run(Delegate *delegate) Q_DECL_OVERRIDE
    {
        // FIXME: This could be needed if we want to run Chromium tests.
        // We could run a QEventLoop here.
    }

    virtual void Quit() Q_DECL_OVERRIDE
    {
        Q_UNREACHABLE();
    }

    virtual void ScheduleWork() Q_DECL_OVERRIDE
    {
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

    virtual void ScheduleDelayedWork(const base::TimeTicks &delayed_work_time) Q_DECL_OVERRIDE
    {
        startTimer(GetTimeIntervalMilliseconds(delayed_work_time));
    }

protected:
    virtual void customEvent(QEvent *ev) Q_DECL_OVERRIDE
    {
        if (handleScheduledWork())
            QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

    virtual void timerEvent(QTimerEvent *ev) Q_DECL_OVERRIDE
    {
        killTimer(ev->timerId());

        base::TimeTicks next_delayed_work_time;
        m_delegate->DoDelayedWork(&next_delayed_work_time);

        if (!next_delayed_work_time.is_null())
            startTimer(GetTimeIntervalMilliseconds(next_delayed_work_time));
    }

private:
    bool handleScheduledWork() {
        bool more_work_is_plausible = m_delegate->DoWork();

        base::TimeTicks delayed_work_time;
        more_work_is_plausible |= m_delegate->DoDelayedWork(&delayed_work_time);

        if (more_work_is_plausible)
            return true;

        more_work_is_plausible |= m_delegate->DoIdleWork();
        if (!more_work_is_plausible && !delayed_work_time.is_null())
            startTimer(GetTimeIntervalMilliseconds(delayed_work_time));

        return more_work_is_plausible;
    }

    Delegate *m_delegate;
};

base::MessagePump* messagePumpFactory()
{
    return new MessagePumpForUIQt;
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
        base::MessageLoop::InitMessagePumpForUIFactory(::messagePumpFactory);
    }

    void PreMainMessageLoopRun() Q_DECL_OVERRIDE
    {
        m_browserContext.reset(new BrowserContextQt());
    }

    void PostMainMessageLoopRun()
    {
        m_browserContext.reset();
    }

    int PreCreateThreads() Q_DECL_OVERRIDE
    {
        base::ThreadRestrictions::SetIOAllowed(true);
        // Like ChromeBrowserMainExtraPartsAura::PreCreateThreads does.
        gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, new DesktopScreenQt);
        return 0;
    }

    BrowserContextQt* browser_context() const {
        return m_browserContext.get();
    }

private:
    scoped_ptr<BrowserContextQt> m_browserContext;

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
        if (platform == QStringLiteral("xcb")) {
            if (gfx::GetGLImplementation() == gfx::kGLImplementationEGLGLES2)
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
            else
                m_handle = pni->nativeResourceForContext(QByteArrayLiteral("glxcontext"), qtContext);
        } else if (platform == QStringLiteral("cocoa"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("cglcontextobj"), qtContext);
        else if (platform == QStringLiteral("qnx"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else if (platform == QStringLiteral("eglfs"))
            m_handle = pni->nativeResourceForContext(QByteArrayLiteral("eglcontext"), qtContext);
        else {
            qFatal("%s platform not yet supported", platform.toLatin1().constData());
            // Add missing platforms once they work.
            Q_UNREACHABLE();
        }
    }

    virtual void* GetHandle() Q_DECL_OVERRIDE { return m_handle; }

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
#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    QOpenGLContext *shareContext = QSGContext::sharedOpenGLContext();
#else
    QOpenGLContext *shareContext = QOpenGLContextPrivate::globalShareContext();
#endif
    if (!shareContext) {
        qFatal("QWebEngine: OpenGL resource sharing is not set up in QtQuick. Please make sure to call QWebEngine::initialize() or QWebEngineWidgets::initialize() in your main() function.");
    }
    m_shareContextQtQuick = make_scoped_refptr(new QtShareGLContext(shareContext));
}

content::WebContentsViewPort* ContentBrowserClientQt::OverrideCreateWebContentsView(content::WebContents* web_contents,
                                                                                    content::RenderViewHostDelegateView** render_view_host_delegate_view)
{
    WebContentsViewQt* rv = new WebContentsViewQt(web_contents);
    *render_view_host_delegate_view = rv;
    return rv;
}

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

void ContentBrowserClientQt::RenderProcessHostCreated(content::RenderProcessHost* host)
{
    // FIXME: Add a settings variable to enable/disable the file scheme.
    content::ChildProcessSecurityPolicy::GetInstance()->GrantScheme(host->GetID(), chrome::kFileScheme);
}

void ContentBrowserClientQt::ResourceDispatcherHostCreated()
{
    m_resourceDispatcherHostDelegate.reset(new ResourceDispatcherHostDelegateQt);
    content::ResourceDispatcherHost::Get()->SetDelegate(m_resourceDispatcherHostDelegate.get());
}

gfx::GLShareGroup *ContentBrowserClientQt::GetInProcessGpuShareGroup()
{
    if (!m_shareGroupQtQuick)
        m_shareGroupQtQuick = new ShareGroupQtQuick;
    return m_shareGroupQtQuick.get();
}

content::MediaObserver *ContentBrowserClientQt::GetMediaObserver()
{
    return MediaCaptureDevicesDispatcher::GetInstance();
}

BrowserContextQt* ContentBrowserClientQt::browser_context() {
    Q_ASSERT(m_browserMainParts);
    return static_cast<BrowserMainPartsQt*>(m_browserMainParts)->browser_context();
}

net::URLRequestContextGetter* ContentBrowserClientQt::CreateRequestContext(content::BrowserContext* content_browser_context, content::ProtocolHandlerMap* protocol_handlers)
{
    if (content_browser_context != browser_context())
        fprintf(stderr, "Warning: off the record browser context not implemented !\n");
    return static_cast<BrowserContextQt*>(browser_context())->CreateRequestContext(protocol_handlers);
}

void ContentBrowserClientQt::enableInspector(bool enable)
{
    if (enable && !m_devtools) {
        m_devtools.reset(new DevToolsHttpHandlerDelegateQt(browser_context()));
    } else if (!enable && m_devtools) {
        m_devtools.reset();
    }
}
