/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "browser_main_parts_qt.h"

#include "api/qwebenginemessagepumpscheduler_p.h"

#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_impl.h"
#include "base/message_loop/message_loop_current.h"
#include "base/process/process.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/common/service_manager_connection.h"
#include "extensions/buildflags/buildflags.h"
#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/common/constants.h"
#include "extensions/common/extensions_client.h"
#include "extensions/extensions_browser_client_qt.h"
#include "extensions/extension_system_factory_qt.h"
#include "common/extensions/extensions_client_qt.h"
#endif //BUILDFLAG(ENABLE_EXTENSIONS)
#include "services/resource_coordinator/public/cpp/process_resource_coordinator.h"
#include "services/resource_coordinator/public/cpp/resource_coordinator_features.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/service_manager/public/cpp/service.h"
#include "ui/display/screen.h"

#include "service/service_qt.h"
#include "web_engine_context.h"

#include <QtGui/qtgui-config.h>

#if QT_CONFIG(opengl)
#include "ui/gl/gl_context.h"
#include <QOpenGLContext>
#endif

#if defined(OS_MACOSX)
#include "ui/base/idle/idle.h"
#endif

#if defined(Q_OS_WIN)
#include "ui/display/win/screen_win.h"
#else
#include "desktop_screen_qt.h"
#endif


namespace QtWebEngineCore {

namespace {

// Return a timeout suitable for the glib loop, -1 to block forever,
// 0 to return right away, or a timeout in milliseconds from now.
int GetTimeIntervalMilliseconds(const base::TimeTicks &from)
{
    if (from.is_null())
        return -1;

    // Be careful here.  TimeDelta has a precision of microseconds, but we want a
    // value in milliseconds.  If there are 5.5ms left, should the delay be 5 or
    // 6?  It should be 6 to avoid executing delayed work too early.
    int delay = static_cast<int>(std::ceil((from - base::TimeTicks::Now()).InMillisecondsF()));

    // If this value is negative, then we need to run delayed work soon.
    return delay < 0 ? 0 : delay;
}

}  // anonymous namespace

class MessagePumpForUIQt : public base::MessagePump
{
public:
    MessagePumpForUIQt()
        : m_scheduler([this]() { handleScheduledWork(); })
    {}

    void setDelegate(Delegate *delegate)
    {
        m_delegate = delegate;
    }

    void Run(Delegate *delegate) override
    {
        // This is used only when MessagePumpForUIQt is used outside of the GUI thread.
        NOTIMPLEMENTED();
    }

    void Quit() override
    {
        // This is used only when MessagePumpForUIQt is used outside of the GUI thread.
        NOTIMPLEMENTED();
    }

    void ScheduleWork() override
    {
        // NOTE: This method may called from any thread at any time.
        m_scheduler.scheduleWork();
    }

    void ScheduleDelayedWork(const base::TimeTicks &delayed_work_time) override
    {
        m_scheduler.scheduleDelayedWork(GetTimeIntervalMilliseconds(delayed_work_time));
    }

private:
    // Both Qt and Chromium keep track of the current GL context by using
    // thread-local variables, and naturally they are completely unaware of each
    // other. As a result, when a QOpenGLContext is made current, the previous
    // gl::GLContext is not released, and vice-versa. This is fine as long as
    // each thread uses exclusively either Qt or Chromium GL bindings, which is
    // usually the case.
    //
    // The only exception is when the GL driver is considered thread-unsafe
    // (QOpenGLContext::supportsThreadedOpenGL() is false), in which case we
    // have to run all GL operations, including Chromium's GPU service, on the
    // UI thread. Now the bindings are being mixed and both Qt and Chromium get
    // quite confused regarding the current state of the surface.
    //
    // To get this to work we have to release the current QOpenGLContext before
    // executing any tasks from Chromium's GPU service and the gl::GLContext
    // afterwards. Since GPU service just posts tasks to the UI thread task
    // runner, we'll have to instrument the entire UI thread message pump.
    class ScopedGLContextChecker
    {
#if QT_CONFIG(opengl)
    public:
        ScopedGLContextChecker()
        {
            if (!m_enabled)
                return;

            if (QOpenGLContext *context = QOpenGLContext::currentContext())
                context->doneCurrent();
        }

        ~ScopedGLContextChecker()
        {
            if (!m_enabled)
                return;

            if (gl::GLContext *context = gl::GLContext::GetCurrent())
                context->ReleaseCurrent(nullptr);
        }

    private:
        bool m_enabled = !QOpenGLContext::supportsThreadedOpenGL();
#endif // QT_CONFIG(opengl)
    };


    void handleScheduledWork()
    {
        Q_ASSERT(m_delegate);

        ScopedGLContextChecker glContextChecker;

        bool more_work_is_plausible = m_delegate->DoWork();

        base::TimeTicks delayed_work_time;
        more_work_is_plausible |= m_delegate->DoDelayedWork(&delayed_work_time);

        if (more_work_is_plausible)
            return ScheduleWork();

        more_work_is_plausible |= m_delegate->DoIdleWork();
        if (more_work_is_plausible)
            return ScheduleWork();

        ScheduleDelayedWork(delayed_work_time);
    }

    Delegate *m_delegate = nullptr;
    QWebEngineMessagePumpScheduler m_scheduler;
};

// Needed to access protected constructor from MessageLoop.
class MessageLoopForUIQt : public base::MessageLoop {
public:
    MessageLoopForUIQt() : MessageLoop(TYPE_UI, base::BindOnce(&messagePumpFactory))
    {
        BindToCurrentThread();

        auto pump = static_cast<MessagePumpForUIQt *>(pump_);
        auto backend = static_cast<base::MessageLoopImpl *>(backend_.get());
        pump->setDelegate(backend);
    }
private:
    static std::unique_ptr<base::MessagePump> messagePumpFactory()
    {
        return base::WrapUnique(new MessagePumpForUIQt);
    }
};

BrowserMainPartsQt::BrowserMainPartsQt() : content::BrowserMainParts()
{ }

BrowserMainPartsQt::~BrowserMainPartsQt() = default;


int BrowserMainPartsQt::PreEarlyInitialization()
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    content::ChildProcessSecurityPolicy::GetInstance()->RegisterWebSafeScheme(extensions::kExtensionScheme);
#endif //ENABLE_EXTENSIONS
    return 0;
}

void BrowserMainPartsQt::PreMainMessageLoopStart()
{
    // Overrides message loop creation in BrowserMainLoop::MainMessageLoopStart().
    m_mainMessageLoop.reset(new MessageLoopForUIQt);
}

void BrowserMainPartsQt::PreMainMessageLoopRun()
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::ExtensionsClient::Set(new extensions::ExtensionsClientQt());
    extensions::ExtensionsBrowserClient::Set(new extensions::ExtensionsBrowserClientQt());
    extensions::ExtensionSystemFactoryQt::GetInstance();
#endif //ENABLE_EXTENSIONS
}

void BrowserMainPartsQt::PostMainMessageLoopRun()
{
    // The ProfileQt's destructor uses the MessageLoop so it should be deleted
    // right before the RenderProcessHostImpl's destructor destroys it.
    WebEngineContext::current()->destroyProfileAdapter();
}

int BrowserMainPartsQt::PreCreateThreads()
{
    base::ThreadRestrictions::SetIOAllowed(true);

#if defined(OS_MACOSX)
    ui::InitIdleMonitor();
#endif

    // Like ChromeBrowserMainExtraPartsViews::PreCreateThreads does.
#if defined(Q_OS_WIN)
    display::Screen::SetScreenInstance(new display::win::ScreenWin);
#else
    display::Screen::SetScreenInstance(new DesktopScreenQt);
#endif
    return 0;
}

void BrowserMainPartsQt::ServiceManagerConnectionStarted(content::ServiceManagerConnection *connection)
{
    ServiceQt::GetInstance()->InitConnector();
    connection->GetConnector()->WarmService(service_manager::ServiceFilter::ByName("qtwebengine"));
    m_processResourceCoordinator = std::make_unique<resource_coordinator::ProcessResourceCoordinator>(connection->GetConnector());
    m_processResourceCoordinator->OnProcessLaunched(base::Process::Current());
}

} // namespace QtWebEngineCore
