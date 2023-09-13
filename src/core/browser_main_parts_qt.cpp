// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "browser_main_parts_qt.h"

#include "api/qwebenginemessagepumpscheduler_p.h"

#include "base/message_loop/message_pump.h"
#include "base/message_loop/message_pump_for_ui.h"
#include "base/process/process.h"
#include "base/task/current_thread.h"
#include "base/task/sequence_manager/sequence_manager_impl.h"
#include "base/task/sequence_manager/thread_controller_with_message_pump_impl.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/threading/thread_restrictions.h"
#include "chrome/browser/tab_contents/form_interaction_tab_helper.h"
#include "components/device_event_log/device_event_log.h"
#include "components/performance_manager/embedder/graph_features.h"
#include "components/performance_manager/embedder/performance_manager_lifetime.h"
#include "components/performance_manager/public/graph/graph.h"
#include "components/performance_manager/public/performance_manager.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_security_policy.h"
#include "content/public/common/content_features.h"
#include "content/public/common/result_codes.h"
#include "extensions/buildflags/buildflags.h"
#include "ppapi/buildflags/buildflags.h"
#include "select_file_dialog_factory_qt.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/service_manager/public/cpp/service.h"
#include "type_conversion.h"
#include "ui/display/screen.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/common/constants.h"
#include "extensions/common/extensions_client.h"
#include "extensions/extensions_browser_client_qt.h"
#include "extensions/extension_system_factory_qt.h"
#include "common/extensions/extensions_client_qt.h"
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

#if BUILDFLAG(ENABLE_PLUGINS)
#include "content/public/browser/plugin_service.h"
#include "extensions/plugin_service_filter_qt.h"
#endif // BUILDFLAG(ENABLE_PLUGINS)

#include "web_engine_context.h"
#include "web_usb_detector_qt.h"

#include <QDeadlineTimer>
#include <QtGui/qtgui-config.h>
#include <QStandardPaths>

#if QT_CONFIG(opengl)
#include "ui/gl/gl_context.h"
#include <QOpenGLContext>
#endif

#if BUILDFLAG(IS_MAC)
#include "base/message_loop/message_pump_mac.h"
#include "services/device/public/cpp/geolocation/geolocation_manager.h"
#include "services/device/public/cpp/geolocation/system_geolocation_source.h"
#include "ui/base/idle/idle.h"
#endif

#if defined(Q_OS_WIN)
#include "ui/display/win/screen_win.h"
#else
#include "desktop_screen_qt.h"
#endif

#if defined(Q_OS_LINUX)
#include "components/os_crypt/key_storage_config_linux.h"
#include "components/os_crypt/os_crypt.h"
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

    void Run(Delegate *) override
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
        ensureDelegate();
        m_scheduler.scheduleImmediateWork();
    }

    void ScheduleDelayedWork(const Delegate::NextWorkInfo &next_work_info) override
    {
        // NOTE: This method may called from any thread at any time.
        ScheduleDelayedWork(next_work_info.delayed_run_time);
    }

    void ScheduleDelayedWork(const base::TimeTicks &delayed_work_time)
    {
        ensureDelegate();
        m_scheduler.scheduleDelayedWork(GetTimeIntervalMilliseconds(delayed_work_time));
    }

private:
    void ensureDelegate()
    {
        if (!m_delegate) {
            auto seqMan = base::CurrentThread::Get()->GetCurrentSequenceManagerImpl();
            m_delegate = static_cast<base::sequence_manager::internal::ThreadControllerWithMessagePumpImpl *>(
                             seqMan->controller_.get());
        }
    }
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
        bool m_enabled = WebEngineContext::isGpuServiceOnUIThread();
#endif // QT_CONFIG(opengl)
    };


    void handleScheduledWork()
    {
        ScopedGLContextChecker glContextChecker;

        QDeadlineTimer timer(std::chrono::milliseconds(2));
        base::MessagePump::Delegate::NextWorkInfo more_work_info = m_delegate->DoWork();
        while (more_work_info.is_immediate() && !timer.hasExpired())
            more_work_info = m_delegate->DoWork();

        if (more_work_info.is_immediate())
            return m_scheduler.scheduleImmediateWork();

        if (m_delegate->DoIdleWork())
            return m_scheduler.scheduleIdleWork();

        ScheduleDelayedWork(more_work_info.delayed_run_time);
    }

    Delegate *m_delegate = nullptr;
    QWebEngineMessagePumpScheduler m_scheduler;
};

#if BUILDFLAG(IS_MAC)
class FakeGeolocationSource : public device::SystemGeolocationSource
{
public:
    FakeGeolocationSource() = default;
    ~FakeGeolocationSource() override = default;

    // SystemGeolocationSource implementation:
    void StartWatchingPosition(bool) override {}
    void StopWatchingPosition() override {}
    void RegisterPermissionUpdateCallback(PermissionUpdateCallback callback)
    {
        callback.Run(device::LocationSystemPermissionStatus::kDenied);
    }
    void RegisterPositionUpdateCallback(PositionUpdateCallback callback) {}
};
#endif // BUILDFLAG(IS_MAC)

std::unique_ptr<base::MessagePump> messagePumpFactory()
{
    static bool madePrimaryPump = false;
    if (!madePrimaryPump) {
        madePrimaryPump = true;
        return std::make_unique<MessagePumpForUIQt>();
    }
#if BUILDFLAG(IS_MAC)
    return base::MessagePumpMac::Create();
#else
    return std::make_unique<base::MessagePumpForUI>();
#endif
}

int BrowserMainPartsQt::PreEarlyInitialization()
{
#if BUILDFLAG(ENABLE_EXTENSIONS)
    content::ChildProcessSecurityPolicy::GetInstance()->RegisterWebSafeScheme(extensions::kExtensionScheme);
#endif //ENABLE_EXTENSIONS
    return content::RESULT_CODE_NORMAL_EXIT;
}

void BrowserMainPartsQt::PreCreateMainMessageLoop()
{
#if BUILDFLAG(IS_MAC)
    m_geolocationManager = std::make_unique<device::GeolocationManager>(std::make_unique<FakeGeolocationSource>());
#endif
}

void BrowserMainPartsQt::PostCreateMainMessageLoop()
{
    if (!device_event_log::IsInitialized())
        device_event_log::Initialize(0 /* default max entries */);

#if defined(Q_OS_LINUX)
    std::unique_ptr<os_crypt::Config> config = std::make_unique<os_crypt::Config>();
    config->product_name = "Qt WebEngine";
    config->main_thread_runner = content::GetUIThreadTaskRunner({});
    config->should_use_preference = false;
    config->user_data_path = toFilePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    OSCrypt::SetConfig(std::move(config));
#endif
}

int BrowserMainPartsQt::PreMainMessageLoopRun()
{
    ui::SelectFileDialog::SetFactory(new SelectFileDialogFactoryQt());

#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::ExtensionsClient::Set(new extensions::ExtensionsClientQt());
    extensions::ExtensionsBrowserClient::Set(new extensions::ExtensionsBrowserClientQt());
    extensions::ExtensionSystemFactoryQt::GetInstance();

#if BUILDFLAG(ENABLE_PLUGINS)
    content::PluginService *plugin_service = content::PluginService::GetInstance();
    plugin_service->SetFilter(extensions::PluginServiceFilterQt::GetInstance());
#endif // BUILDFLAG(ENABLE_PLUGINS)
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

    if (base::FeatureList::IsEnabled(features::kWebUsb)) {
        m_webUsbDetector.reset(new WebUsbDetectorQt());
        content::GetUIThreadTaskRunner({ base::TaskPriority::BEST_EFFORT })
                ->PostTask(FROM_HERE,
                           base::BindOnce(&WebUsbDetectorQt::Initialize,
                                          base::Unretained(m_webUsbDetector.get())));
    }
    return content::RESULT_CODE_NORMAL_EXIT;
}

void BrowserMainPartsQt::PostMainMessageLoopRun()
{
    performance_manager_lifetime_.reset();

    m_webUsbDetector.reset();

    // The ProfileQt's destructor uses the MessageLoop so it should be deleted
    // right before the RenderProcessHostImpl's destructor destroys it.
    WebEngineContext::current()->destroyProfileAdapter();
}

int BrowserMainPartsQt::PreCreateThreads()
{
#if BUILDFLAG(IS_MAC)
    ui::InitIdleMonitor();
#endif

    // Like ChromeBrowserMainExtraPartsViews::PreCreateThreads does.
#if defined(Q_OS_WIN)
    display::Screen::SetScreenInstance(new display::win::ScreenWin);
#elif defined(Q_OS_DARWIN)
    display::Screen::SetScreenInstance(display::CreateNativeScreen());
#else
    display::Screen::SetScreenInstance(new DesktopScreenQt);
#endif
    return content::RESULT_CODE_NORMAL_EXIT;
}

static void CreatePoliciesAndDecorators(performance_manager::Graph *graph)
{
    graph->PassToGraph(FormInteractionTabHelper::CreateGraphObserver());
}

void BrowserMainPartsQt::PostCreateThreads()
{
    performance_manager_lifetime_ =
        std::make_unique<performance_manager::PerformanceManagerLifetime>(
            performance_manager::GraphFeatures::WithDefault(),
            base::BindOnce(&QtWebEngineCore::CreatePoliciesAndDecorators));
}

#if BUILDFLAG(IS_MAC)
device::GeolocationManager *BrowserMainPartsQt::GetGeolocationManager()
{
    return m_geolocationManager.get();
}
#endif

} // namespace QtWebEngineCore
