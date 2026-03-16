// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "web_engine_context.h"

#include <math.h>

#include "base/base_switches.h"
#include "base/functional/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/metrics/field_trial.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/run_loop.h"
#include "base/strings/string_split.h"
#include "base/task/sequence_manager/thread_controller_with_message_pump_impl.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#include "chrome/common/chrome_switches.h"
#include "content/common/features.h"
#include "content/common/process_visibility_tracker.h"
#include "content/browser/memory_coordinator/browser_memory_consumer_registry.h"
#include "content/browser/compositor/surface_utils.h"
#include "content/browser/compositor/viz_process_transport_factory.h"
#include "components/viz/host/host_frame_sink_manager.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "chrome/browser/printing/print_job_manager.h"
#endif
#include "components/discardable_memory/service/discardable_shared_memory_manager.h"
#include "components/download/public/common/download_task_runner.h"
#include "components/input/switches.h"
#include "components/power_monitor/make_power_monitor_device_source.h"
#include "components/viz/common/features.h"
#include "components/variations/variations_ids_provider.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/app/mojo_ipc_support.h"
#include "content/browser/devtools/devtools_http_handler.h"
#include "content/browser/gpu/gpu_main_thread_factory.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/scheduler/browser_task_executor.h"
#include "content/browser/startup_data_impl.h"
#include "content/browser/startup_helper.h"
#include "content/browser/service_host/utility_process_host.h"
#include "content/gpu/in_process_gpu_thread.h"
#include "content/browser/tracing/memory_instrumentation_util.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_features.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/renderer/in_process_renderer_thread.h"
#include "content/utility/in_process_utility_thread.h"
#include "gpu/command_buffer/service/gpu_switches.h"
#include "gpu/config/gpu_finch_features.h"
#include "media/audio/audio_manager.h"
#include "media/base/media_switches.h"
#include "mojo/core/embedder/embedder.h"
#include "net/base/port_util.h"
#include "sandbox/policy/switches.h"
#include "services/network/public/cpp/features.h"
#include "services/network/public/cpp/network_switches.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/tracing/public/cpp/perfetto/perfetto_traced_process.h"
#include "services/tracing/public/cpp/trace_startup.h"
#include "services/tracing/public/cpp/tracing_features.h"
#include "third_party/blink/public/common/features.h"
#include "ui/base/ui_base_features.h"
#include "ui/events/event_switches.h"
#include "ui/native_theme/features/native_theme_features.h"
#include "ui/gl/gl_utils.h"
#include "ui/gl/gl_switches.h"
#include "url/url_features.h"
#if defined(Q_OS_WIN)
#include "sandbox/win/src/sandbox_types.h"
#include "content/public/app/sandbox_helper_win.h"
#endif // Q_OS_WIN

#if defined(Q_OS_MACOS)
#include "base/apple/foundation_util.h"
#endif

#if QT_CONFIG(accessibility)
#include "accessibility_activation_observer.h"
#endif
#include "api/qwebengineurlscheme.h"
#include "content_browser_client_qt.h"
#include "content_client_qt.h"
#include "content_main_delegate_qt.h"
#include "devtools_manager_delegate_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "net/webui_controller_factory_qt.h"
#include "profile_adapter.h"
#include "rhi_gpu_info.h"
#include "type_conversion.h"
#include "web_engine_library_info.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QMutex>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QStringList>
#include <QNetworkProxy>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QLoggingCategory>

#if QT_CONFIG(opengl) && BUILDFLAG(IS_OZONE)
#include "ozone/ozone_util_qt.h"
#endif // QT_CONFIG(opengl) && BUILDFLAG(IS_OZONE)

#define STRINGIFY_LITERAL(x) #x
#define STRINGIFY_EXPANDED(x) STRINGIFY_LITERAL(x)

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {

Q_WEBENGINE_LOGGING_CATEGORY(webEngineContextLog, "qt.webenginecontext")

static bool isFeatureEnabled(const std::string &feature, const base::CommandLine &commandLine)
{
    auto isInFeatureList = [&feature, commandLine](const std::string_view featuresSwitch) -> bool {
        if (!commandLine.HasSwitch(featuresSwitch))
            return false;

        std::string featuresString = commandLine.GetSwitchValueASCII(featuresSwitch);
        std::vector<std::string> features = base::SplitString(
                featuresString, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
        return std::find(features.begin(), features.end(), feature) != features.end();
    };

    return !isInFeatureList(switches::kDisableFeatures)
            && isInFeatureList(switches::kEnableFeatures);
}

static bool usingSupportedSGBackend()
{
    if (QQuickWindow::graphicsApi() != QSGRendererInterface::OpenGL
        && QQuickWindow::graphicsApi() != QSGRendererInterface::Vulkan
        && QQuickWindow::graphicsApi() != QSGRendererInterface::Metal
        && QQuickWindow::graphicsApi() != QSGRendererInterface::Direct3D11)
        return false;

    const QStringList args = QGuiApplication::arguments();

    // follow the logic from contextFactory in src/quick/scenegraph/qsgcontextplugin.cpp
    QString device = QQuickWindow::sceneGraphBackend();

    for (int index = 0; index < args.count(); ++index) {
        if (args.at(index).startsWith("--device="_L1)) {
            device = args.at(index).mid(9);
            break;
        }
    }

    if (device.isEmpty())
        device = qEnvironmentVariable("QT_QUICK_BACKEND");
    if (device.isEmpty())
        device = qEnvironmentVariable("QMLSCENE_DEVICE");

    return device.isEmpty() || device == "rhi"_L1;
}

static std::string getGLType(const base::CommandLine &cmd)
{
    if (cmd.HasSwitch(switches::kUseGL))
        return cmd.GetSwitchValueASCII(switches::kUseGL);

    if (!usingSupportedSGBackend() || cmd.HasSwitch(switches::kDisableGpu))
        return gl::kGLImplementationDisabledName;

    return gl::kGLImplementationANGLEName;
}

static bool isGLTypeSupported(const std::string &glType, bool usingVulkan = false)
{
#if BUILDFLAG(IS_OZONE)
    if (glType == gl::kGLImplementationEGLName)
        return true;

    if (glType == gl::kGLImplementationStubName)
        return usingVulkan;
#else
    Q_UNUSED(usingVulkan);
#endif

    if (glType == gl::kGLImplementationANGLEName || glType == gl::kGLImplementationDisabledName)
        return true;

    return false;
}

static std::string getVulkanType(const base::CommandLine &cmd)
{
#if QT_CONFIG(webengine_vulkan)
    if (isFeatureEnabled(features::kVulkan.name, cmd)) {
        if (cmd.HasSwitch(switches::kUseVulkan))
            return cmd.GetSwitchValueASCII(switches::kUseVulkan);
        return switches::kVulkanImplementationNameNative;
    }
#endif

    return "disabled";
}

static std::string getANGLEType(const base::CommandLine &cmd)
{
    if (getGLType(cmd) == gl::kGLImplementationANGLEName) {
        if (cmd.HasSwitch(switches::kUseANGLE))
            return cmd.GetSwitchValueASCII(switches::kUseANGLE);

        return gl::kANGLEImplementationDefaultName;
    }

    return "disabled";
}

static void logContext(const base::CommandLine &cmd)
{
    if (Q_UNLIKELY(webEngineContextLog().isDebugEnabled())) {
        QString log;
        log += u'\n';

        log += "Chromium GL Backend: "_L1 + QLatin1StringView(getGLType(cmd)) + u'\n';
        log += "Chromium ANGLE Backend: "_L1 + QLatin1StringView(getANGLEType(cmd)) + u'\n';
        log += "Chromium Vulkan Backend: "_L1 + QLatin1StringView(getVulkanType(cmd)) + u'\n';
        log += u'\n';

        log += "QSG RHI Backend: "_L1 + RhiGpuInfo::instance()->backendName() + u'\n';
        log += "QSG RHI Backend Supported: "_L1 + (usingSupportedSGBackend() ? "yes"_L1 : "no"_L1)
                + u'\n';
        log += "QSG RHI Device: "_L1 + RhiGpuInfo::instance()->deviceName() + u'\n';
        log += "QSG RHI GPU Vendor: "_L1 + RhiGpuInfo::instance()->vendorName() + u'\n';
        log += u'\n';

#if QT_CONFIG(opengl)
#if BUILDFLAG(IS_OZONE)
        log += "Using GLX: "_L1 + (OzoneUtilQt::usingGLX() ? "yes"_L1 : "no"_L1) + u'\n';
        log += "Using EGL: "_L1 + (OzoneUtilQt::usingEGL() ? "yes"_L1 : "no"_L1) + u'\n';
        log += "Using GBM: "_L1 + (RhiGpuInfo::instance()->isGbmSupported() ? "yes"_L1 : "no"_L1)
                + u'\n';
#endif // BUILDFLAG(IS_OZONE)
        log += "Using Shared GL: "_L1 + (QOpenGLContext::globalShareContext() ? "yes"_L1 : "no"_L1)
                + u'\n';
        log += u'\n';
#endif // QT_CONFIG(opengl)

        log += "Init Parameters:\n"_L1;
        const base::CommandLine::SwitchMap switchMap = cmd.GetSwitches();
        for (const auto &pair : switchMap)
            log += " *  "_L1 + toQt(pair.first) + u' ' + toQt(pair.second) + u'\n';

        qCDebug(webEngineContextLog, "%ls", qUtf16Printable(log));
    }
}

extern std::unique_ptr<base::MessagePump> messagePumpFactory();

static void setupProxyPac(base::CommandLine &commandLine)
{
    if (commandLine.HasSwitch(switches::kProxyPacUrl)) {
        QUrl pac_url(toQt(commandLine.GetSwitchValueASCII(switches::kProxyPacUrl)));
        if (pac_url.isValid()
            && (pac_url.isLocalFile()
                || !pac_url.scheme().compare("qrc"_L1, Qt::CaseInsensitive))) {
            QFile file;
            if (pac_url.isLocalFile())
                file.setFileName(pac_url.toLocalFile());
            else
                file.setFileName(pac_url.path().prepend(QLatin1Char(':')));
            if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QByteArray ba = file.readAll();
                commandLine.RemoveSwitch(switches::kProxyPacUrl);
                commandLine.AppendSwitchASCII(
                        switches::kProxyPacUrl,
                        ba.toBase64()
                                .prepend("data:application/x-javascript-config;base64,")
                                .toStdString());
            }
        }
    }
}

static QStringList parseEnvCommandLine(const QString &cmdLine)
{
    QString arg;
    QStringList arguments;
    enum { Parse, Quoted, Unquoted } state = Parse;
    for (const QChar c : cmdLine) {
        switch (state) {
        case Parse:
            if (c == QLatin1Char('"')) {
                state = Quoted;
            } else if (c != QLatin1Char(' ')) {
                arg += c;
                state = Unquoted;
            }
            // skips spaces
            break;
        case Quoted:
            if (c == QLatin1Char('"')) {
                DCHECK(!arg.isEmpty());
                state = Unquoted;
            } else {
                // includes spaces
                arg += c;
            }
            break;
        case Unquoted:
            if (c == QLatin1Char('"')) {
                // skips quotes
                state = Quoted;
            } else if (c == QLatin1Char(' ')) {
                arguments.append(arg);
                arg.clear();
                state = Parse;
            } else {
                arg += c;
            }
            break;
        }
    }
    // last arg
    if (!arg.isEmpty()) {
        arguments.append(arg);
    }
    return arguments;
}

scoped_refptr<QtWebEngineCore::WebEngineContext> WebEngineContext::m_handle;
bool WebEngineContext::m_destroyed = false;
bool WebEngineContext::m_closingDown = false;
void WebEngineContext::destroyProfileAdapter()
{
    if (content::RenderProcessHost::run_renderer_in_process()) {
        Q_ASSERT(m_profileAdapters.count() == 1);
        // this is a default profile
        m_defaultProfileAdapter.reset();
        Q_ASSERT(m_profileAdapters.isEmpty());
    }
}

bool WebEngineContext::profileExistOnPath(const QString &dataPath)
{
    for (auto existingProfileAdapter : m_profileAdapters) {
        if (existingProfileAdapter->dataPath() == dataPath)
            return true;
    }
    return false;
}

void WebEngineContext::addProfileAdapter(ProfileAdapter *profileAdapter)
{
    Q_ASSERT(!m_profileAdapters.contains(profileAdapter));
    const QString path = profileAdapter->dataPath();
    if (!profileAdapter->isOffTheRecord() && !profileAdapter->storageName().isEmpty()) {
        for (auto existingProfileAdapter : m_profileAdapters) {
            if (existingProfileAdapter->dataPath() == path) {
                // QTBUG-66068
                qWarning("Using the same data path for profile, may corrupt the data.");
                break;
            }
        }
    }

    if (content::RenderProcessHost::run_renderer_in_process()){
        if (!m_profileAdapters.isEmpty())
            qFatal("Single mode supports only single profile.");
        // there is only one profile therefore make it 'default'
        m_defaultProfileAdapter.reset(profileAdapter);
    }
    m_profileAdapters.append(profileAdapter);
}

void WebEngineContext::removeProfileAdapter(ProfileAdapter *profileAdapter)
{
    m_profileAdapters.removeAll(profileAdapter);
}

void WebEngineContext::flushMessages()
{
    if (!m_destroyed) {
        base::MessagePump::Delegate *delegate = static_cast<
                base::sequence_manager::internal::ThreadControllerWithMessagePumpImpl *>(
                WebEngineContext::current()->m_runLoop->delegate_);
        while (delegate->DoWork().is_immediate()) { }
    }
}
void WebEngineContext::destroy()
{
    if (m_devtoolsServer)
        m_devtoolsServer->stop();

    // Normally the GPU thread is shut down when the GpuProcessHost is destroyed
    // on IO thread (triggered by ~BrowserMainRunner). But by that time the UI
    // task runner is not working anymore so we need to clean Viz up earlier.
    if (m_mainDelegate->gpuClient() && m_mainDelegate->gpuClient()->gpuTaskRunner()) {
        content::GetHostFrameSinkManager()->SetConnectionLostCallback(base::DoNothing());
        auto factory = static_cast<content::VizProcessTransportFactory *>(
                content::ImageTransportFactory::GetInstance());
        factory->PrepareForShutDown();

        // Wait for viz destroy tasks to be completed on the GPU thread.
        base::WaitableEvent event(base::WaitableEvent::ResetPolicy::MANUAL,
                                  base::WaitableEvent::InitialState::NOT_SIGNALED);
        m_mainDelegate->gpuClient()->gpuTaskRunner()->PostTask(
                FROM_HERE,
                base::BindOnce([](base::WaitableEvent *event) { event->Signal(); }, &event));
        event.Wait();
    }

    // Flush the UI message loop before quitting.
    flushMessages();

#if QT_CONFIG(webengine_printing_and_pdf)
    // Kill print job manager early as it has a content::NotificationRegistrar
    m_printJobManager.reset();
#endif

    // Delete the global object and thus custom profiles
    // In case of single process ~RenderProcessHostImpl (there is only one instance)
    // is called explicitly by BrowserMainLoop::ShutdownThreadsAndCleanUp and requires browser context.
    // therefore delete browser context on PostMainMessageLoopRun.
    if (!content::RenderProcessHost::run_renderer_in_process()) {
        m_defaultProfileAdapter.reset();
        m_globalQObject.reset();
        while (m_profileAdapters.count())
            delete m_profileAdapters.first();
    } else {
        m_globalQObject.reset();
    }

    // Handle any events posted by browser-context shutdown.
    // This should deliver all necessary calls of DeleteSoon from PostTask
    flushMessages();

    m_devtoolsServer.reset();
    m_runLoop->AfterRun();

    // Destroy the main runner, this stops main message loop
    m_browserRunner.reset();

    // These would normally be in the content-runner, but we allocated them separately:
    m_mojoIpcSupport.reset();
    m_discardableSharedMemoryManager.reset();

    // Destroying content-runner will force Chromium at_exit calls to run, and
    // reap child processes.
    m_contentRunner.reset();

    // Drop the false reference.
    m_handle->Release();
}

WebEngineContext::~WebEngineContext()
{
    // WebEngineContext::destroy() must be called before we are deleted
    Q_ASSERT(!m_globalQObject);
    Q_ASSERT(!m_devtoolsServer);
    Q_ASSERT(!m_browserRunner);
    Q_ASSERT(m_profileAdapters.isEmpty());
}

WebEngineContext *WebEngineContext::current()
{
    if (m_destroyed)
        return nullptr;
    if (!m_handle.get()) {
        m_handle = new WebEngineContext();
        // Make sure that we ramp down Chromium before QApplication destroys its X connection, etc.
        qAddPostRoutine(WebEngineContext::destroyContextPostRoutine);
        // Add a false reference so there is no race between unreferencing m_handle and a global QApplication.
        m_handle->AddRef();
    }
    return m_handle.get();
}

ProfileAdapter *WebEngineContext::createDefaultProfileAdapter()
{
    Q_ASSERT(!m_destroyed);
    if (!m_defaultProfileAdapter) {
        ProfileAdapter *profile = new ProfileAdapter();
        // profile when added to m_profileAdapters might be set default
        // profile in case of single-process
        if (!m_defaultProfileAdapter)
            m_defaultProfileAdapter.reset(profile);
    }
    return m_defaultProfileAdapter.get();
}

ProfileAdapter *WebEngineContext::defaultProfileAdapter()
{
    return m_defaultProfileAdapter.get();
}

QObject *WebEngineContext::globalQObject()
{
    return m_globalQObject.get();
}

void WebEngineContext::destroyContextPostRoutine()
{
    // Destroy WebEngineContext before its static pointer is zeroed and destructor called.
    // Before destroying MessageLoop via destroying BrowserMainRunner destructor
    // WebEngineContext's pointer is used.
    m_closingDown = true;
    m_handle->destroy();
#if !defined(NDEBUG)
    if (!m_handle->HasOneRef())
        qWarning("WebEngineContext leaked on exit, likely due to leaked WebEngine View or Page");
#endif
    m_handle = nullptr;
    m_destroyed = true;
}

ProxyAuthentication WebEngineContext::qProxyNetworkAuthentication(QString host, int port)
{
    if (!QNetworkProxyFactory::usesSystemConfiguration()) {
        QNetworkProxy proxy = QNetworkProxy::applicationProxy();
        if (host == proxy.hostName() && port == proxy.port() && !proxy.user().isEmpty()
            && !proxy.password().isEmpty()) {
            return std::make_tuple(true, proxy.user(), proxy.password());
        }
    }
    return std::make_tuple(false, QString(), QString());
}

#ifndef CHROMIUM_VERSION
#error Chromium version should be defined at gyp-time. Something must have gone wrong
#define CHROMIUM_VERSION // This is solely to keep Qt Creator happy.
#endif

const static char kChromiumFlagsEnv[] = "QTWEBENGINE_CHROMIUM_FLAGS";
const static char kDisableSandboxEnv[] = "QTWEBENGINE_DISABLE_SANDBOX";

bool ShouldAllowSystemTracingConsumer() {
    return false;
}

static void initializeFeatureList(base::CommandLine &commandLine,
                                  std::vector<std::string> enableFeatures,
                                  std::vector<std::string> disableFeatures)
{
    std::string enableFeaturesString = base::JoinString(enableFeatures, ",");
    if (commandLine.HasSwitch(switches::kEnableFeatures)) {
        std::string commandLineEnableFeatures =
                commandLine.GetSwitchValueASCII(switches::kEnableFeatures);

        for (const std::string &enableFeature : base::SplitString(commandLineEnableFeatures, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY)) {
            auto it = std::find(disableFeatures.begin(), disableFeatures.end(), enableFeature);
            if (it == disableFeatures.end())
                continue;

            qWarning("An unsupported feature has been enabled from command line: %s\n"
                     "The feature is enabled but there is no guarantee that it will work or not break QtWebEngine.", enableFeature.c_str());

            // If a feature is disabled and enabled at the same time, then it will be disabled.
            // Remove feature from the disable list to make it possible to override from command line.
            disableFeatures.erase(it);
        }

        enableFeaturesString = enableFeaturesString + "," + commandLineEnableFeatures;
    }

    std::string disableFeaturesString = base::JoinString(disableFeatures, ",");
    if (commandLine.HasSwitch(switches::kDisableFeatures)) {
        std::string commandLineDisableFeatures =
                commandLine.GetSwitchValueASCII(switches::kDisableFeatures);

        for (const std::string &disableFeature : base::SplitString(commandLineDisableFeatures, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY)) {
            auto it = std::find(enableFeatures.begin(), enableFeatures.end(), disableFeature);
            if (it == enableFeatures.end())
                continue;

            qWarning("An essential feature has been disabled from command line: %s\n"
                     "The feature is disabled but there is no guarantee that it will not break QtWebEngine.", disableFeature.c_str());
        }

        disableFeaturesString = disableFeaturesString + "," + commandLineDisableFeatures;
    }

    commandLine.AppendSwitchASCII(switches::kEnableFeatures, enableFeaturesString);
    commandLine.AppendSwitchASCII(switches::kDisableFeatures, disableFeaturesString);
    base::FeatureList::InitInstance(enableFeaturesString, disableFeaturesString);
}

WebEngineContext::WebEngineContext()
    : m_mainDelegate(new ContentMainDelegateQt)
    , m_globalQObject(new QObject())
{
#if defined(Q_OS_MACOS)
    // The bundled handling is currently both completely broken in Chromium,
    // and unnecessary for us.
    base::apple::SetOverrideAmIBundled(false);
#endif

#ifdef Q_OS_LINUX
    // Call qputenv before BrowserMainRunnerImpl::Initialize is called.
    // http://crbug.com/245466
    qputenv("force_s3tc_enable", "true");
#endif

    if (QWebEngineUrlScheme::schemeByName(QByteArrayLiteral("qrc")) == QWebEngineUrlScheme()) {
        // User might have registered "qrc" already with different options.
        QWebEngineUrlScheme qrcScheme(QByteArrayLiteral("qrc"));
        qrcScheme.setFlags(QWebEngineUrlScheme::SecureScheme
                           | QWebEngineUrlScheme::ViewSourceAllowed);
        QWebEngineUrlScheme::registerScheme(qrcScheme);
    }

    QWebEngineUrlScheme::lockSchemes();

    // Allow us to inject javascript like any webview toolkit.
    content::RenderFrameHost::AllowInjectingJavaScript();

    bool useEmbeddedSwitches;
    base::CommandLine &parsedCommandLine = *initCommandLine(&useEmbeddedSwitches);

    setupProxyPac(parsedCommandLine);
    parsedCommandLine.AppendSwitchPath(switches::kBrowserSubprocessPath,
                                       WebEngineLibraryInfo::getPath(content::CHILD_PROCESS_EXE));

    parsedCommandLine.AppendSwitchASCII(
            switches::kApplicationName,
            QCoreApplication::applicationName().toUtf8().toPercentEncoding().toStdString());

    // Enable sandboxing on OS X and Linux (Desktop / Embedded) by default.
    bool disable_sandbox = qEnvironmentVariableIsSet(kDisableSandboxEnv);
    if (!disable_sandbox) {
#if defined(Q_OS_LINUX)
        parsedCommandLine.AppendSwitch(sandbox::policy::switches::kDisableSetuidSandbox);
#endif
    } else {
        parsedCommandLine.AppendSwitch(sandbox::policy::switches::kNoSandbox);
        qInfo("Sandboxing disabled by user.");
    }

    // Do not advertise a feature we have removed at compile time
    parsedCommandLine.AppendSwitch(switches::kDisableSpeechAPI);

    std::vector<std::string> disableFeatures;
    std::vector<std::string> enableFeatures;

    enableFeatures.push_back(features::kNetworkServiceInProcess.name);
    enableFeatures.push_back(features::kTracingServiceInProcess.name);
#if defined(Q_OS_MACOS) && BUILDFLAG(USE_SCK)
    // The feature name should match the definition of kScreenCaptureKitMacScreen.
    enableFeatures.push_back("ScreenCaptureKitMacScreen");
#endif // defined(Q_OS_MACOS)

    // By default the Touch Events API support (presence of 'ontouchstart' in 'window' object)
    // will be determined based on the availability of touch screen devices.
    if (!parsedCommandLine.HasSwitch(switches::kTouchEventFeatureDetection))
        parsedCommandLine.AppendSwitchASCII(switches::kTouchEventFeatureDetection,
                                            switches::kTouchEventFeatureDetectionAuto);

    // Not implemented but it overrides the devtools eyedropper
    // Should be sync with kEyeDropper base::Feature
    parsedCommandLine.AppendSwitchASCII(switches::kDisableBlinkFeatures, "EyeDropperAPI");
    disableFeatures.push_back(features::kEyeDropper.name);

    // Explicitly tell Chromium about default-on features we do not support
    disableFeatures.push_back(features::kBackgroundFetch.name);
    disableFeatures.push_back(features::kInstalledApp.name);
    parsedCommandLine.AppendSwitchASCII(switches::kDisableBlinkFeatures, "WebOTP");
    disableFeatures.push_back(features::kWebOTP.name);
    disableFeatures.push_back(features::kWebPayments.name);
    disableFeatures.push_back(features::kWebUsb.name);
#if defined(Q_OS_MACOS)
    // Skia Graphite is enabled by default on macOS, but we do not yet support Dawn
    // (or any) Graphite backend. This currently breaks hardware rendering.
    // TODO: Re-enable once we support Dawn on macOS.
    disableFeatures.push_back(features::kSkiaGraphite.name);
#endif

    if (useEmbeddedSwitches) {
        // embedded switches are based on the switches for Android, see content/browser/android/content_startup_flags.cc
        enableFeatures.push_back(features::kOverlayScrollbar.name);
        parsedCommandLine.AppendSwitch(switches::kEnableViewport);
        parsedCommandLine.AppendSwitch(input::switches::kValidateInputEventStream);
        parsedCommandLine.AppendSwitch(switches::kDisableCompositedAntialiasing);
    }

#if BUILDFLAG(IS_OZONE)
    if (!RhiGpuInfo::instance()->isGbmSupported()) {
        disableFeatures.push_back(media::kAcceleratedVideoDecodeLinux.name);
        parsedCommandLine.AppendSwitch(switches::kDisableGpuMemoryBufferVideoFrames);
    }
#endif

    // Init GPU switches.
    parsedCommandLine.AppendSwitch(switches::kInProcessGPU);

    std::string glType = getGLType(parsedCommandLine);
    // Always set --use-gl.
    if (!parsedCommandLine.HasSwitch(switches::kUseGL))
        parsedCommandLine.AppendSwitchASCII(switches::kUseGL, glType);

    if (glType == gl::kGLImplementationDisabledName) {
        // Always set --disable-gpu to avoid unexpected GL contexts, see QTBUG-128784.
        if (!parsedCommandLine.HasSwitch(switches::kDisableGpu))
            parsedCommandLine.AppendSwitch(switches::kDisableGpu);
    } else {
        // Warn on custom --use-gl if hardware rendering is disabled.
        if (!usingSupportedSGBackend())
            qWarning("--use-gl=%s is set with unsupported SceneGraph Backend. Expect troubles!",
                     glType.c_str());

        if (parsedCommandLine.HasSwitch(switches::kDisableGpu))
            qWarning("--use-gl=%s is set with --disable-gpu. Expect troubles!", glType.c_str());
    }

#if BUILDFLAG(IS_OZONE)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::OpenGL && usingSupportedSGBackend()) {
        const bool disableGpu = parsedCommandLine.HasSwitch(switches::kDisableGpu);
        const bool usingVulkan = isFeatureEnabled(features::kVulkan.name, parsedCommandLine);
        if (!disableGpu && !usingVulkan && !RhiGpuInfo::instance()->isGbmSupported()) {
#if QT_CONFIG(webengine_vulkan)
            if (RhiGpuInfo::isVulkanSupported()) {
                qWarning("GBM is not supported with the current configuration. "
                         "Fallback to Vulkan rendering in Chromium.");
                parsedCommandLine.AppendSwitchASCII(switches::kUseVulkan,
                                                    switches::kVulkanImplementationNameNative);
                enableFeatures.push_back(features::kVulkan.name);
            } else
#endif
            {
                qWarning("GBM is not supported with the current configuration and Vulkan is not "
                         "available. Fallback to software rendering.");
                parsedCommandLine.AppendSwitch(switches::kDisableGpu);
            }
        }
    }
#if QT_CONFIG(webengine_vulkan)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Vulkan && usingSupportedSGBackend()) {
        // TODO: Try not to force Chromium's Vulkan backend on Linux.
        //       Currently we force it because OzoneImageBackingFactory does not support to create
        //       SharedImage in RGBA8888 format under GLX.
        parsedCommandLine.AppendSwitchASCII(switches::kUseVulkan,
                                            switches::kVulkanImplementationNameNative);
        enableFeatures.push_back(features::kVulkan.name);

        const char deviceExtensionsVar[] = "QT_VULKAN_DEVICE_EXTENSIONS";
        QByteArrayList requiredDeviceExtensions = { "VK_KHR_external_memory_fd",
                                                    "VK_EXT_external_memory_dma_buf",
                                                    "VK_EXT_image_drm_format_modifier" };
        if (qEnvironmentVariableIsSet(deviceExtensionsVar)) {
            QByteArrayList envExtList = qgetenv(deviceExtensionsVar).split(';');
            int found = 0;
            for (const QByteArray &ext : requiredDeviceExtensions) {
                if (envExtList.contains(ext))
                    found++;
            }
            if (found != requiredDeviceExtensions.size()) {
                qWarning("Vulkan rendering may fail because %s environment variable is already "
                         "set but it doesn't contain some of the required Vulkan device "
                         "extensions:\n%s",
                         deviceExtensionsVar, requiredDeviceExtensions.join('\n').constData());
            }
        } else {
            qputenv(deviceExtensionsVar, requiredDeviceExtensions.join(';'));
        }
    }
#endif // QT_CONFIG(webengine_vulkan)
#endif // BUILDFLAG(IS_OZONE)

#if defined(Q_OS_WIN)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11
        || QQuickWindow::graphicsApi() == QSGRendererInterface::Vulkan) {
        const QString luid = RhiGpuInfo::instance()->getAdapterLuid();
        if (!luid.isEmpty())
            parsedCommandLine.AppendSwitchASCII(switches::kUseAdapterLuid, luid.toStdString());
    }
#endif
    // We need the FieldTrialList to make sure Chromium features are provided to child processes
    if (!base::FieldTrialList::GetInstance()) {
        m_fieldTrialList.reset(new base::FieldTrialList());
    }

    initializeFeatureList(parsedCommandLine, enableFeatures, disableFeatures);

    logContext(parsedCommandLine);

    // Early error on unsupported --use-gl settings.
    if (!isGLTypeSupported(glType, isFeatureEnabled(features::kVulkan.name, parsedCommandLine)))
        qFatal("--use-gl=%s is not supported with the current configuration.", glType.c_str());

    registerMainThreadFactories();

    content::ContentMainParams contentMainParams(m_mainDelegate.get());
    contentMainParams.setup_signal_handlers = false;
#if defined(Q_OS_WIN)
    contentMainParams.sandbox_info = QtWebEngineSandbox::staticSandboxInterfaceInfo();
    sandbox::SandboxInterfaceInfo sandbox_info = {nullptr};
    if (!contentMainParams.sandbox_info) {
        content::InitializeSandboxInfo(&sandbox_info);
        contentMainParams.sandbox_info = &sandbox_info;
    }
#endif
    m_contentRunner = content::ContentMainRunner::Create();
    m_contentRunner->Initialize(std::move(contentMainParams));

    mojo::core::Configuration mojoConfiguration;
    mojoConfiguration.is_broker_process = true;
    mojo::core::Init(mojoConfiguration);

    // This block mirrors ContentMainRunnerImpl::RunBrowser():

    m_browserMemoryConsumerRegistry =
            std::make_unique<base::ScopedMemoryConsumerRegistry<content::BrowserMemoryConsumerRegistry>>();
    m_mainDelegate->PreBrowserMain();
    base::MessagePump::OverrideMessagePumpForUIFactory(messagePumpFactory);
    content::BrowserTaskExecutor::Create();
    auto* provider = m_mainDelegate->CreateVariationsIdsProvider();
    if (!provider) {
        variations::VariationsIdsProvider::Create(
                variations::VariationsIdsProvider::Mode::kUseSignedInState);
    }
    m_mainDelegate->PostEarlyInitialization({});
    content::StartBrowserThreadPool();
    tracing::InitTracingPostFeatureList(/*enable_consumer=*/true, /*will_trace_thread_restart=*/false);
    tracing::PerfettoTracedProcess::Get().SetAllowSystemTracingConsumerCallback(
            base::BindRepeating(&ShouldAllowSystemTracingConsumer));
    base::PowerMonitor::GetInstance()->Initialize(MakePowerMonitorDeviceSource());
    content::ProcessVisibilityTracker::GetInstance();
    m_discardableSharedMemoryManager = std::make_unique<discardable_memory::DiscardableSharedMemoryManager>();

    m_mojoIpcSupport = std::make_unique<content::MojoIpcSupport>(content::BrowserTaskExecutor::CreateIOThread());
    download::SetIOTaskRunner(m_mojoIpcSupport->io_thread()->task_runner());
    content::InitializeBrowserMemoryInstrumentationClient();
    std::unique_ptr<content::StartupData> startupData = m_mojoIpcSupport->CreateBrowserStartupData();

    // Once the MessageLoop has been created, attach a top-level RunLoop.
    m_runLoop.reset(new base::RunLoop);
    m_runLoop->BeforeRun();

    content::MainFunctionParams mainParams(base::CommandLine::ForCurrentProcess());
    mainParams.startup_data = std::move(startupData);
    m_browserRunner = content::BrowserMainRunner::Create();
    m_browserRunner->Initialize(std::move(mainParams));

    m_devtoolsServer.reset(new DevToolsServerQt());
    m_devtoolsServer->start();
    // Force the initialization of MediaCaptureDevicesDispatcher on the UI
    // thread to avoid a thread check assertion in its constructor when it
    // first gets referenced on the IO thread.
    MediaCaptureDevicesDispatcher::GetInstance();

    // Initialize WebCacheManager here to ensure its subscription to render process creation events.
    web_cache::WebCacheManager::GetInstance();

#if defined(Q_OS_LINUX)
    media::AudioManager::SetGlobalAppName(QCoreApplication::applicationName().toStdString());
#endif

#if QT_CONFIG(webengine_printing_and_pdf)
    m_printJobManager.reset(new printing::PrintJobManager());
#endif

#if QT_CONFIG(accessibility)
    m_accessibilityActivationObserver.reset(new AccessibilityActivationObserver());
#endif

    content::WebUIControllerFactory::RegisterFactory(WebUIControllerFactoryQt::GetInstance());
}

#if QT_CONFIG(webengine_printing_and_pdf)
printing::PrintJobManager* WebEngineContext::getPrintJobManager()
{
    return m_printJobManager.get();
}
#endif

base::CommandLine *WebEngineContext::initCommandLine(bool *useEmbeddedSwitches)
{
    if (!base::CommandLine::CreateEmpty())
        qFatal("base::CommandLine has been initialized unexpectedly.");

    QStringList appArgs = QCoreApplication::arguments();
    if (appArgs.empty()) {
        qFatal("Argument list is empty, the program name is not passed to QCoreApplication. "
               "base::CommandLine cannot be properly initialized.");
    }

    base::CommandLine *commandLine = base::CommandLine::ForCurrentProcess();
    int index = appArgs.indexOf(QRegularExpression(u"--webEngineArgs"_s,
                                                   QRegularExpression::CaseInsensitiveOption));
    if (qEnvironmentVariableIsSet(kChromiumFlagsEnv)) {
        appArgs = appArgs.mid(0, 1); // Take application name and drop the rest
        appArgs.append(parseEnvCommandLine(qEnvironmentVariable(kChromiumFlagsEnv)));
        if (index > -1)
            qWarning("Note 'webEngineArgs' are overridden by QTWEBENGINE_CHROMIUM_FLAGS");
    } else {
        if (index > -1) {
            appArgs.erase(appArgs.begin() + 1, appArgs.begin() + index + 1);
        } else {
            appArgs = appArgs.mid(0, 1);
        }
    }
#if defined(QTWEBENGINE_EMBEDDED_SWITCHES)
    *useEmbeddedSwitches = !appArgs.contains("--disable-embedded-switches"_L1);
#else
    *useEmbeddedSwitches = appArgs.contains("--enable-embedded-switches"_L1);
#endif
    appArgs.removeAll("--disable-embedded-switches"_L1);
    appArgs.removeAll("--enable-embedded-switches"_L1);

    bool isRemoteDebugPort =
            (-1
             != appArgs.indexOf(QRegularExpression(u"--remote-debugging-port=.*"_s,
                                                   QRegularExpression::CaseInsensitiveOption)))
            || !qEnvironmentVariable("QTWEBENGINE_REMOTE_DEBUGGING").isEmpty();
    bool isRemoteAllowOrigins =
            (-1
             != appArgs.indexOf(QRegularExpression(u"--remote-allow-origins=.*"_s,
                                                   QRegularExpression::CaseInsensitiveOption)));

    if (isRemoteDebugPort && !isRemoteAllowOrigins) {
        appArgs.append(u"--remote-allow-origins=*"_s);
        qWarning("Added {--remote-allow-origins=*} to command-line arguments "
                 "to avoid web socket connection errors during remote debugging.");
    }

    base::CommandLine::StringVector argv;
    argv.resize(appArgs.size());
#if defined(Q_OS_WIN)
    for (int i = 0; i < appArgs.size(); ++i)
        argv[i] = appArgs[i].toStdWString();
#else
    for (int i = 0; i < appArgs.size(); ++i)
        argv[i] = appArgs[i].toStdString();
#endif
    commandLine->InitFromArgv(argv);

    return commandLine;
}

bool WebEngineContext::closingDown()
{
    return m_closingDown;
}

void WebEngineContext::registerMainThreadFactories()
{
    content::UtilityProcessHost::RegisterUtilityMainThreadFactory(content::CreateInProcessUtilityThread);
    content::RenderProcessHostImpl::RegisterRendererMainThreadFactory(content::CreateInProcessRendererThread);
    content::RegisterGpuMainThreadFactory(content::CreateInProcessGpuThread);
}

} // namespace

QT_BEGIN_NAMESPACE
const char *qWebEngineVersion() noexcept
{
    return STRINGIFY_EXPANDED(QTWEBENGINECORE_VERSION_STR);
}

const char *qWebEngineProcessName() noexcept
{
    return STRINGIFY_EXPANDED(QTWEBENGINEPROCESS_NAME);
}

const char *qWebEngineChromiumVersion() noexcept
{
    return STRINGIFY_EXPANDED(CHROMIUM_VERSION);
}

const char *qWebEngineChromiumSecurityPatchVersion() noexcept
{
    return "146.0.7680.80"; // FIXME: Remember to update
}

QT_END_NAMESPACE
