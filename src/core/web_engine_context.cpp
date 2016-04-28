/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "web_engine_context.h"

#include <math.h>

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#if defined(ENABLE_BASIC_PRINTING)
#include "chrome/browser/printing/print_job_manager.h"
#endif // defined(ENABLE_BASIC_PRINTING)
#include "content/browser/gpu/gpu_process_host.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/utility_process_host_impl.h"
#include "content/gpu/in_process_gpu_thread.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/plugin_service.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/renderer/in_process_renderer_thread.h"
#include "content/utility/in_process_utility_thread.h"
#include "gpu/command_buffer/service/gpu_switches.h"
#include "ui/events/event_switches.h"
#include "ui/native_theme/native_theme_switches.h"
#include "ui/gl/gl_switches.h"
#if defined(OS_WIN)
#include "sandbox/win/src/sandbox_types.h"
#include "content/public/app/sandbox_helper_win.h"
#endif // OS_WIN

#include "browser_context_adapter.h"
#include "content_browser_client_qt.h"
#include "content_client_qt.h"
#include "content_main_delegate_qt.h"
#include "dev_tools_http_handler_delegate_qt.h"
#include "gl_context_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "type_conversion.h"
#include "surface_factory_qt.h"
#include "web_engine_library_info.h"
#include <QFileInfo>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QStringList>
#include <QVector>
#include <qpa/qplatformnativeinterface.h>

using namespace QtWebEngineCore;

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE

namespace {

scoped_refptr<QtWebEngineCore::WebEngineContext> sContext;

void destroyContext()
{
    // Destroy WebEngineContext before its static pointer is zeroed and destructor called.
    // Before destroying MessageLoop via destroying BrowserMainRunner destructor
    // WebEngineContext's pointer is used.
    sContext->destroy();
    sContext = 0;
}

bool usingANGLE()
{
#if defined(Q_OS_WIN)
    if (qt_gl_global_share_context())
        return qt_gl_global_share_context()->isOpenGLES();
    return QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES;
#else
    return false;
#endif
}

bool usingSoftwareDynamicGL()
{
#if defined(Q_OS_WIN)
    HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());
    wchar_t path[MAX_PATH];
    DWORD size = GetModuleFileName(handle, path, MAX_PATH);
    QFileInfo openGLModule(QString::fromWCharArray(path, size));
    return openGLModule.fileName() == QLatin1String("opengl32sw.dll");
#else
    return false;
#endif
}

bool usingQtQuick2DRenderer()
{
    const QStringList args = QGuiApplication::arguments();
    QString device;
    for (int index = 0; index < args.count(); ++index) {
        if (args.at(index).startsWith(QLatin1String("--device="))) {
            device = args.at(index).mid(9);
            break;
        }
    }

    if (device.isEmpty())
        device = QString::fromLocal8Bit(qgetenv("QMLSCENE_DEVICE"));

    // This assumes that the plugin is installed and is going to be used by QtQuick.
    return device == QLatin1String("softwarecontext");
}

#if defined(ENABLE_PLUGINS)
void dummyGetPluginCallback(const std::vector<content::WebPluginInfo>&)
{
}
#endif

} // namespace

namespace QtWebEngineCore {

void WebEngineContext::destroyBrowserContext()
{
    m_defaultBrowserContext.reset();
}

void WebEngineContext::destroy()
{
    delete m_globalQObject;
    m_globalQObject = 0;
    base::MessagePump::Delegate *delegate = m_runLoop->loop_;
    // Flush the UI message loop before quitting.
    while (delegate->DoWork()) { }
    GLContextHelper::destroy();
    m_devtools.reset(0);
    m_runLoop->AfterRun();

    // Force to destroy RenderProcessHostImpl by destroying BrowserMainRunner.
    // RenderProcessHostImpl should be destroyed before WebEngineContext since
    // default BrowserContext might be used by the RenderprocessHostImpl's destructor.
    m_browserRunner.reset(0);
}

WebEngineContext::~WebEngineContext()
{
}

scoped_refptr<WebEngineContext> WebEngineContext::current()
{
    if (!sContext.get()) {
        sContext = new WebEngineContext();
        // Make sure that we ramp down Chromium before QApplication destroys its X connection, etc.
        qAddPostRoutine(destroyContext);
    }
    return sContext;
}

QSharedPointer<BrowserContextAdapter> WebEngineContext::defaultBrowserContext()
{
    if (!m_defaultBrowserContext)
        m_defaultBrowserContext = QSharedPointer<BrowserContextAdapter>::create(QStringLiteral("Default"));
    return m_defaultBrowserContext;
}

QObject *WebEngineContext::globalQObject()
{
    return m_globalQObject;
}

#ifndef CHROMIUM_VERSION
#error Chromium version should be defined at gyp-time. Something must have gone wrong
#define CHROMIUM_VERSION // This is solely to keep Qt Creator happy.
#endif

WebEngineContext::WebEngineContext()
    : m_mainDelegate(new ContentMainDelegateQt)
    , m_contentRunner(content::ContentMainRunner::Create())
    , m_browserRunner(content::BrowserMainRunner::Create())
    , m_globalQObject(new QObject())
{
    QStringList appArgs = QCoreApplication::arguments();
    bool useEmbeddedSwitches = appArgs.removeAll(QStringLiteral("--enable-embedded-switches"));
#if defined(QTWEBENGINE_EMBEDDED_SWITCHES)
    useEmbeddedSwitches = !appArgs.removeAll(QStringLiteral("--disable-embedded-switches"));
#endif

#ifdef Q_OS_LINUX
    // Call qputenv before BrowserMainRunnerImpl::Initialize is called.
    // http://crbug.com/245466
    qputenv("force_s3tc_enable", "true");
#endif

    // Allow us to inject javascript like any webview toolkit.
    content::RenderFrameHost::AllowInjectingJavaScriptForAndroidWebView();

#if defined(Q_OS_WIN)
    // We must initialize the command line with the UTF-16 arguments vector we got from
    // QCoreApplication. CommandLine::Init ignores its arguments on Windows and calls
    // GetCommandLineW() instead.
    base::CommandLine::CreateEmpty();
    base::CommandLine* parsedCommandLine = base::CommandLine::ForCurrentProcess();
    base::CommandLine::StringVector argv;
    argv.resize(appArgs.size());
    std::transform(appArgs.constBegin(), appArgs.constEnd(), argv.begin(), &toString16);
    parsedCommandLine->InitFromArgv(argv);
#else
    QVector<QByteArray> args;
    Q_FOREACH (const QString& arg, appArgs)
        args << arg.toUtf8();

    QVector<const char*> argv(args.size());
    for (int i = 0; i < args.size(); ++i)
        argv[i] = args[i].constData();
    base::CommandLine::Init(argv.size(), argv.constData());
    base::CommandLine* parsedCommandLine = base::CommandLine::ForCurrentProcess();
#endif

    parsedCommandLine->AppendSwitchPath(switches::kBrowserSubprocessPath, WebEngineLibraryInfo::getPath(content::CHILD_PROCESS_EXE));

    // Enable sandboxing on OS X and Linux (Desktop / Embedded) by default.
    bool disable_sandbox = qEnvironmentVariableIsSet("QTWEBENGINE_DISABLE_SANDBOX");
    if (!disable_sandbox) {
#if defined(Q_OS_WIN)
        parsedCommandLine->AppendSwitch(switches::kNoSandbox);
#elif defined(Q_OS_LINUX)
        parsedCommandLine->AppendSwitch(switches::kDisableSetuidSandbox);
#endif
    } else {
        parsedCommandLine->AppendSwitch(switches::kNoSandbox);
        qInfo() << "Sandboxing disabled by user.";
    }

    parsedCommandLine->AppendSwitch(switches::kEnableThreadedCompositing);
    parsedCommandLine->AppendSwitch(switches::kInProcessGPU);
    // These are currently only default on OS X, and we don't support them:
    parsedCommandLine->AppendSwitch(switches::kDisableZeroCopy);
    parsedCommandLine->AppendSwitch(switches::kDisableGpuMemoryBufferCompositorResources);

    if (useEmbeddedSwitches) {
        // Inspired by the Android port's default switches
        parsedCommandLine->AppendSwitch(switches::kEnableOverlayScrollbar);
        parsedCommandLine->AppendSwitch(switches::kEnablePinch);
        parsedCommandLine->AppendSwitch(switches::kEnableViewport);
        parsedCommandLine->AppendSwitch(switches::kMainFrameResizesAreOrientationChanges);
        parsedCommandLine->AppendSwitch(switches::kDisableAcceleratedVideoDecode);
        parsedCommandLine->AppendSwitch(switches::kDisableGpuShaderDiskCache);
        parsedCommandLine->AppendSwitch(switches::kDisable2dCanvasAntialiasing);
        parsedCommandLine->AppendSwitch(cc::switches::kDisableCompositedAntialiasing);
        parsedCommandLine->AppendSwitchASCII(switches::kProfilerTiming, switches::kProfilerTimingDisabledValue);
    }

    GLContextHelper::initialize();

    if (usingANGLE() || usingSoftwareDynamicGL() || usingQtQuick2DRenderer()) {
        parsedCommandLine->AppendSwitch(switches::kDisableGpu);
    } else {
        const char *glType = 0;
        if (qt_gl_global_share_context()) {
            if (qt_gl_global_share_context()->isOpenGLES()) {
                glType = gfx::kGLImplementationEGLName;
            } else {
                glType = gfx::kGLImplementationDesktopName;
            }
        } else {
            qWarning("WebEngineContext used before QtWebEngine::initialize()");
            // We have to assume the default OpenGL module type will be used.
            switch (QOpenGLContext::openGLModuleType()) {
            case QOpenGLContext::LibGL:
                glType = gfx::kGLImplementationDesktopName;
                break;
            case QOpenGLContext::LibGLES:
                glType = gfx::kGLImplementationEGLName;
                break;
            }
        }

        parsedCommandLine->AppendSwitchASCII(switches::kUseGL, glType);
    }

    content::UtilityProcessHostImpl::RegisterUtilityMainThreadFactory(content::CreateInProcessUtilityThread);
    content::RenderProcessHostImpl::RegisterRendererMainThreadFactory(content::CreateInProcessRendererThread);
    content::GpuProcessHost::RegisterGpuMainThreadFactory(content::CreateInProcessGpuThread);

    content::ContentMainParams contentMainParams(m_mainDelegate.get());
    contentMainParams.setup_signal_handlers = false;
#if defined(OS_WIN)
    sandbox::SandboxInterfaceInfo sandbox_info = {0};
    content::InitializeSandboxInfo(&sandbox_info);
    contentMainParams.sandbox_info = &sandbox_info;
#endif
    m_contentRunner->Initialize(contentMainParams);
    m_browserRunner->Initialize(content::MainFunctionParams(*base::CommandLine::ForCurrentProcess()));

    // Once the MessageLoop has been created, attach a top-level RunLoop.
    m_runLoop.reset(new base::RunLoop);
    m_runLoop->BeforeRun();

    m_devtools = createDevToolsHttpHandler();
    // Force the initialization of MediaCaptureDevicesDispatcher on the UI
    // thread to avoid a thread check assertion in its constructor when it
    // first gets referenced on the IO thread.
    MediaCaptureDevicesDispatcher::GetInstance();

#if defined(ENABLE_PLUGINS)
    // Creating pepper plugins from the page (which calls PluginService::GetPluginInfoArray)
    // might fail unless the page queried the list of available plugins at least once
    // (which ends up calling PluginService::GetPlugins). Since the plugins list can only
    // be created from the FILE thread, and that GetPluginInfoArray is synchronous, it
    // can't loads plugins synchronously from the IO thread to serve the render process' request
    // and we need to make sure that it happened beforehand.
    content::PluginService::GetInstance()->GetPlugins(base::Bind(&dummyGetPluginCallback));
#endif

#if defined(ENABLE_BASIC_PRINTING)
    m_printJobManager.reset(new printing::PrintJobManager());
#endif // defined(ENABLE_BASIC_PRINTING)
}

#if defined(ENABLE_BASIC_PRINTING)
printing::PrintJobManager* WebEngineContext::getPrintJobManager()
{
    return m_printJobManager.get();
}
#endif // defined(ENABLE_BASIC_PRINTING)
} // namespace
