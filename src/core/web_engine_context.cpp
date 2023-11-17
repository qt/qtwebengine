// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "web_engine_context.h"

#include <math.h>
#include <QtGui/private/qrhi_p.h>

#include "base/base_switches.h"
#include "base/functional/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/run_loop.h"
#include "base/strings/string_split.h"
#include "base/task/sequence_manager/thread_controller_with_message_pump_impl.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
#include "chrome/browser/media/webrtc/webrtc_log_uploader.h"
#endif
#include "chrome/common/chrome_switches.h"
#include "content/common/process_visibility_tracker.h"
#include "content/gpu/gpu_child_thread.h"
#include "content/browser/compositor/surface_utils.h"
#include "content/browser/compositor/viz_process_transport_factory.h"
#include "components/viz/host/host_frame_sink_manager.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "chrome/browser/printing/print_job_manager.h"
#endif
#include "components/discardable_memory/service/discardable_shared_memory_manager.h"
#include "components/download/public/common/download_task_runner.h"
#include "components/power_scheduler/power_mode_arbiter.h"
#include "components/viz/common/features.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/app/mojo_ipc_support.h"
#include "content/browser/devtools/devtools_http_handler.h"
#include "content/browser/scheduler/browser_task_executor.h"
#include "content/browser/startup_data_impl.h"
#include "content/browser/startup_helper.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#if QT_CONFIG(webengine_pepper_plugins)
#include "content/public/browser/plugin_service.h"
#endif
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_features.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/network_service_util.h"
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
#include "services/tracing/public/cpp/trace_startup.h"
#include "services/tracing/public/cpp/tracing_features.h"
#include "third_party/blink/public/common/features.h"
#include "ui/base/ui_base_features.h"
#include "ui/events/event_switches.h"
#include "ui/native_theme/native_theme_features.h"
#include "ui/gl/gl_switches.h"
#if defined(Q_OS_WIN)
#include "sandbox/win/src/sandbox_types.h"
#include "content/public/app/sandbox_helper_win.h"
#endif // Q_OS_WIN

#if defined(Q_OS_MACOS)
#include "base/mac/foundation_util.h"
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
#include "ozone/gl_context_qt.h"
#include "profile_adapter.h"
#include "type_conversion.h"
#include "web_engine_library_info.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QMutex>
#include <QOffscreenSurface>
#if QT_CONFIG(opengl)
#include <QOpenGLContext>
#include <qopenglcontext_platform.h>
#endif
#include <QQuickWindow>
#include <QStringList>
#include <QSurfaceFormat>
#include <QNetworkProxy>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtQuick/private/qsgrhisupport_p.h>
#include <QLoggingCategory>

#if QT_CONFIG(opengl)
QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE
#endif

#define STRINGIFY_LITERAL(x) #x
#define STRINGIFY_EXPANDED(x) STRINGIFY_LITERAL(x)

namespace QtWebEngineCore {

Q_LOGGING_CATEGORY(webEngineContextLog, "qt.webenginecontext")

#if QT_CONFIG(opengl)

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
        if (args.at(index).startsWith(QLatin1String("--device="))) {
            device = args.at(index).mid(9);
            break;
        }
    }

    if (device.isEmpty())
        device = qEnvironmentVariable("QT_QUICK_BACKEND");
    if (device.isEmpty())
        device = qEnvironmentVariable("QMLSCENE_DEVICE");

    return device.isEmpty() || device == QLatin1String("rhi");
}

bool usingSoftwareDynamicGL()
{
    const char openGlVar[] = "QT_OPENGL";
    if (QCoreApplication::testAttribute(Qt::AA_UseSoftwareOpenGL))
        return true;

    if (qEnvironmentVariableIsSet(openGlVar)) {
        const QByteArray requested = qgetenv(openGlVar);
        if (requested == "software")
            return true;
    }
#if defined(Q_OS_WIN)
    HMODULE handle = QNativeInterface::QWGLContext::openGLModuleHandle();
    wchar_t path[MAX_PATH];
    DWORD size = GetModuleFileName(handle, path, MAX_PATH);
    QFileInfo openGLModule(QString::fromWCharArray(path, size));
    return openGLModule.fileName().contains(QLatin1String("opengl32sw"),Qt::CaseInsensitive);
#else
    return false;
#endif
}

static bool openGLPlatformSupport()
{
    return QGuiApplicationPrivate::platformIntegration()->hasCapability(
            QPlatformIntegration::OpenGL);
}

static const char *getGLType(bool enableGLSoftwareRendering, bool disableGpu)
{
    const char *glType = gl::kGLImplementationDisabledName;
    const bool tryGL =
            usingSupportedSGBackend() && !usingSoftwareDynamicGL() && openGLPlatformSupport();

    if (disableGpu || (!tryGL && !enableGLSoftwareRendering))
        return glType;

#if defined(Q_OS_MACOS)
    return gl::kGLImplementationANGLEName;
#else
#if defined(Q_OS_WIN)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11)
        return gl::kGLImplementationANGLEName;
#endif

    if (!qt_gl_global_share_context() || !qt_gl_global_share_context()->isValid()) {
        qWarning("WebEngineContext is used before QtWebEngineQuick::initialize() or OpenGL context "
                 "creation failed.");
        return glType;
    }

    const QSurfaceFormat sharedFormat = qt_gl_global_share_context()->format();

    switch (sharedFormat.renderableType()) {
    case QSurfaceFormat::OpenGL:
        if (sharedFormat.profile() == QSurfaceFormat::CoreProfile) {
            glType = gl::kGLImplementationDesktopName;
            qWarning("An OpenGL Core Profile was requested, but it is not supported "
                     "on the current platform. Falling back to a non-Core profile. "
                     "Note that this might cause rendering issues.");
        } else {
            glType = gl::kGLImplementationDesktopName;
        }
        break;
    case QSurfaceFormat::OpenGLES:
        glType = gl::kGLImplementationEGLName;
        break;
    case QSurfaceFormat::OpenVG:
    case QSurfaceFormat::DefaultRenderableType:
    default:
        // Shared contex created but no rederable type set.
        qWarning("Unsupported rendering surface format. Please open bug report at "
                 "https://bugreports.qt.io");
    }
    return glType;
#endif // defined(Q_OS_MACOS)
}
#else
static const char *getGLType(bool /*enableGLSoftwareRendering*/, bool disableGpu)
{
    if (disableGpu)
        return gl::kGLImplementationDisabledName;
#if defined(Q_OS_MACOS)
    return gl::kGLImplementationANGLEName;
#elif defined(Q_OS_WIN)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11)
        return gl::kGLImplementationANGLEName;
#endif
    return gl::kGLImplementationDisabledName;
}
#endif // QT_CONFIG(opengl)

#if defined(Q_OS_WIN)
static QString getAdapterLuid() {
    static const bool preferSoftwareDevice = qEnvironmentVariableIntValue("QSG_RHI_PREFER_SOFTWARE_RENDERER");
    QRhiD3D11InitParams rhiParams;
    QRhi::Flags flags;
    if (preferSoftwareDevice) {
        flags |= QRhi::PreferSoftwareRenderer;
    }
    QScopedPointer<QRhi> rhi(QRhi::create(QRhi::D3D11,&rhiParams,flags,nullptr));
    // mimic what QSGRhiSupport and QBackingStoreRhi does
    if (!rhi && !preferSoftwareDevice) {
        flags |= QRhi::PreferSoftwareRenderer;
        rhi.reset(QRhi::create(QRhi::D3D11, &rhiParams, flags));
    }
    if (rhi) {
        const QRhiD3D11NativeHandles *handles =
                static_cast<const QRhiD3D11NativeHandles *>(rhi->nativeHandles());
        Q_ASSERT(handles);
        return QString("%1,%2").arg(handles->adapterLuidHigh).arg(handles->adapterLuidLow);
    } else {
        return QString();
    }
}
#endif

#if QT_CONFIG(webengine_pepper_plugins)
void dummyGetPluginCallback(const std::vector<content::WebPluginInfo>&)
{
}
#endif

static void logContext(const char *glType, base::CommandLine *cmd)
{
    if (Q_UNLIKELY(webEngineContextLog().isDebugEnabled())) {
        const base::CommandLine::SwitchMap switch_map = cmd->GetSwitches();
        QStringList params;
        for (const auto &pair : switch_map)
            params << " * " << toQt(pair.first)
                   << toQt(pair.second) << "\n";
#if QT_CONFIG(opengl)
        const QSurfaceFormat sharedFormat = qt_gl_global_share_context() ? qt_gl_global_share_context()->format() : QSurfaceFormat::defaultFormat();
        const auto profile = QMetaEnum::fromType<QSurfaceFormat::OpenGLContextProfile>().valueToKey(
                sharedFormat.profile());
        const auto type = QMetaEnum::fromType<QSurfaceFormat::RenderableType>().valueToKey(
                sharedFormat.renderableType());
        qCDebug(webEngineContextLog,
                "\n\nChromium GL Backend: %s\n"
                "Surface Type: %s\n"
                "Surface Profile: %s\n"
                "Surface Version: %d.%d\n"
                "QSG RHI Backend: %s\n"
                "Using Supported QSG Backend: %s\n"
                "Using Software Dynamic GL: %s\n"
                "Using Shared GL: %s\n"
                "Using Multithreaded OpenGL: %s\n\n"
                "Init Parameters:\n %s",
                glType, type, profile, sharedFormat.majorVersion(), sharedFormat.minorVersion(),
                qUtf8Printable(QSGRhiSupport::instance()->rhiBackendName()),
                usingSupportedSGBackend() ? "yes" : "no", usingSoftwareDynamicGL() ? "yes" : "no",
                qt_gl_global_share_context() ? "yes" : "no",
                !WebEngineContext::isGpuServiceOnUIThread() ? "yes" : "no",
                qPrintable(params.join(" ")));
#else
        qCDebug(webEngineContextLog,
                "\n\nChromium GL Backend: %s\n"
                "QSG RHI Backend: %s\n\n"
                "Init Parameters:\n %s",
                glType, qUtf8Printable(QSGRhiSupport::instance()->rhiBackendName()),
                qPrintable(params.join(" ")));
#endif //QT_CONFIG(opengl)
    }
}

extern std::unique_ptr<base::MessagePump> messagePumpFactory();

static void setupProxyPac(base::CommandLine *commandLine)
{
    if (commandLine->HasSwitch(switches::kProxyPacUrl)) {
        QUrl pac_url(toQt(commandLine->GetSwitchValueASCII(switches::kProxyPacUrl)));
        if (pac_url.isValid() && (pac_url.isLocalFile() ||
                                  !pac_url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive))) {
            QFile file;
            if (pac_url.isLocalFile())
                file.setFileName(pac_url.toLocalFile());
            else
                file.setFileName(pac_url.path().prepend(QChar(':')));
            if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray ba = file.readAll();
                commandLine->RemoveSwitch(switches::kProxyPacUrl);
                commandLine->AppendSwitchASCII(switches::kProxyPacUrl,
                        ba.toBase64().prepend("data:application/x-javascript-config;base64,").toStdString());
            }
        }
    }
}

static void cleanupVizProcess()
{
    auto gpuChildThread = content::GpuChildThread::instance();
    if (!gpuChildThread)
        return;
    content::GetHostFrameSinkManager()->SetConnectionLostCallback(base::DoNothing());
    auto factory = static_cast<content::VizProcessTransportFactory*>(content::ImageTransportFactory::GetInstance());
    factory->PrepareForShutDown();
}

static QStringList parseEnvCommandLine(const QString &cmdLine)
{
    QString arg;
    QStringList arguments;
    enum { Parse, Quoted, Unquoted } state = Parse;
    for (const QChar c : cmdLine) {
        switch (state) {
        case Parse:
            if (c == '"') {
                state = Quoted;
            } else if (c != ' ' ) {
                arg += c;
                state = Unquoted;
            }
            // skips spaces
            break;
        case Quoted:
            if (c == '"') {
                DCHECK(!arg.isEmpty());
                state = Unquoted;
            } else {
                // includes spaces
                arg += c;
            }
            break;
        case Unquoted:
            if (c == '"') {
                // skips quotes
                state = Quoted;
            } else if (c == ' ') {
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

void WebEngineContext::addProfileAdapter(ProfileAdapter *profileAdapter)
{
    Q_ASSERT(!m_profileAdapters.contains(profileAdapter));
    const QString path = profileAdapter->dataPath();
    if (!profileAdapter->isOffTheRecord() && !profileAdapter->storageName().isEmpty()) {
        for (auto profileAdapter : m_profileAdapters) {
            if (profileAdapter->dataPath() == path) {
                // QTBUG-66068
                qWarning("Using the same data path for profile, may corrupt the data.");
                break;
            }
        }
    }

    if (content::RenderProcessHost::run_renderer_in_process()){
        if (!m_profileAdapters.isEmpty())
            qFatal("Single mode supports only single profile.");
        // there is only one profle therefore make it 'default'
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

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
    if (m_webrtcLogUploader)
        m_webrtcLogUploader->Shutdown();
#endif

    // Normally the GPU thread is shut down when the GpuProcessHost is destroyed
    // on IO thread (triggered by ~BrowserMainRunner). But by that time the UI
    // task runner is not working anymore so we need to do this earlier.
    cleanupVizProcess();
    destroyGpuProcess();
    // Flush the UI message loop before quitting.
    flushMessages();

#if QT_CONFIG(webengine_printing_and_pdf)
    // Kill print job manager early as it has a content::NotificationRegistrar
    m_printJobManager.reset();
#endif

    // Delete the global object and thus custom profiles
    // In case of single process ~RenderProcessHostImpl (there is only one instance)
    // is called expliclty by BrowserMainLoop::ShutdownThreadsAndCleanUp and requires browser context.
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
    // This should deliver all nessesery calls of DeleteSoon from PostTask
    flushMessages();

    m_devtoolsServer.reset();
    m_runLoop->AfterRun();

    // Destroy the main runner, this stops main message loop
    m_browserRunner.reset();
    // gpu thread is no longer around, so no more cotnext is used, remove the helper
    GLContextHelper::destroy();

    // These would normally be in the content-runner, but we allocated them separately:
    m_mojoIpcSupport.reset();
    m_discardableSharedMemoryManager.reset();

    // Destroying content-runner will force Chromium at_exit calls to run, and
    // reap child processes.
    m_contentRunner.reset();

    // Drop the false reference.
    m_handle->Release();

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
    m_webrtcLogUploader.reset();
#endif
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
const static char kDisableInProcGpuThread[] = "QTWEBENGINE_DISABLE_GPU_THREAD";

// static
bool WebEngineContext::isGpuServiceOnUIThread()
{
    static bool threadedGpu =
#if QT_CONFIG(opengl) && !defined(Q_OS_MACOS)
            QOpenGLContext::supportsThreadedOpenGL() &&
#endif
            !qEnvironmentVariableIsSet(kDisableInProcGpuThread);
    return !threadedGpu;
}

static void initializeFeatureList(base::CommandLine *commandLine, std::vector<std::string> enableFeatures, std::vector<std::string> disableFeatures)
{
    std::string enableFeaturesString = base::JoinString(enableFeatures, ",");
    if (commandLine->HasSwitch(switches::kEnableFeatures)) {
        std::string commandLineEnableFeatures = commandLine->GetSwitchValueASCII(switches::kEnableFeatures);

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
    if (commandLine->HasSwitch(switches::kDisableFeatures)) {
        std::string commandLineDisableFeatures = commandLine->GetSwitchValueASCII(switches::kDisableFeatures);

        for (const std::string &disableFeature : base::SplitString(commandLineDisableFeatures, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY)) {
            auto it = std::find(enableFeatures.begin(), enableFeatures.end(), disableFeature);
            if (it == enableFeatures.end())
                continue;

            qWarning("An essential feature has been disabled from command line: %s\n"
                     "The feature is disabled but there is no guarantee that it will not break QtWebEngine.", disableFeature.c_str());
        }

        disableFeaturesString = disableFeaturesString + "," + commandLineDisableFeatures;
    }

    commandLine->AppendSwitchASCII(switches::kEnableFeatures, enableFeaturesString);
    commandLine->AppendSwitchASCII(switches::kDisableFeatures, disableFeaturesString);
    base::FeatureList::InitializeInstance(enableFeaturesString, disableFeaturesString);
}

WebEngineContext::WebEngineContext()
    : m_mainDelegate(new ContentMainDelegateQt)
    , m_globalQObject(new QObject())
{
#if defined(Q_OS_MACOS)
    // The bundled handling is currently both completely broken in Chromium,
    // and unnecessary for us.
    base::mac::SetOverrideAmIBundled(false);
#endif

    base::ThreadPoolInstance::Create("Browser");
    m_contentRunner = content::ContentMainRunner::Create();
    m_browserRunner = content::BrowserMainRunner::Create();

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

    bool useEmbeddedSwitches = false;
    bool enableGLSoftwareRendering = false;
    base::CommandLine *parsedCommandLine =
            initCommandLine(useEmbeddedSwitches, enableGLSoftwareRendering);

    setupProxyPac(parsedCommandLine);
    parsedCommandLine->AppendSwitchPath(switches::kBrowserSubprocessPath, WebEngineLibraryInfo::getPath(content::CHILD_PROCESS_EXE));

    parsedCommandLine->AppendSwitchASCII(switches::kApplicationName, QCoreApplication::applicationName().toUtf8().toPercentEncoding().toStdString());

    // Enable sandboxing on OS X and Linux (Desktop / Embedded) by default.
    bool disable_sandbox = qEnvironmentVariableIsSet(kDisableSandboxEnv);
    if (!disable_sandbox) {
#if defined(Q_OS_LINUX)
        parsedCommandLine->AppendSwitch(sandbox::policy::switches::kDisableSetuidSandbox);
#endif
    } else {
        parsedCommandLine->AppendSwitch(sandbox::policy::switches::kNoSandbox);
        qInfo() << "Sandboxing disabled by user.";
    }

    parsedCommandLine->AppendSwitch(switches::kEnableThreadedCompositing);

    // Do not advertise a feature we have removed at compile time
    parsedCommandLine->AppendSwitch(switches::kDisableSpeechAPI);

    std::vector<std::string> disableFeatures;
    std::vector<std::string> enableFeatures;

    enableFeatures.push_back(features::kNetworkServiceInProcess.name);
    enableFeatures.push_back(features::kTracingServiceInProcess.name);

    // When enabled, event.movement is calculated in blink instead of in browser.
    disableFeatures.push_back(features::kConsolidatedMovementXY.name);

    // Avoid crashing when websites tries using this feature (since 83)
    disableFeatures.push_back(features::kInstalledApp.name);

    // Not implemented but it overrides the devtools eyedropper
    // Should be sync with kEyeDropper base::Feature
    parsedCommandLine->AppendSwitchASCII(switches::kDisableBlinkFeatures, "EyeDropperAPI");
    disableFeatures.push_back(features::kEyeDropper.name);

    // Explicitly tell Chromium about default-on features we do not support
    disableFeatures.push_back(features::kBackgroundFetch.name);
    disableFeatures.push_back(features::kWebOTP.name);
    disableFeatures.push_back(features::kWebPayments.name);
    disableFeatures.push_back(features::kWebUsb.name);

    if (useEmbeddedSwitches) {
        // embedded switches are based on the switches for Android, see content/browser/android/content_startup_flags.cc
        enableFeatures.push_back(features::kOverlayScrollbar.name);
        parsedCommandLine->AppendSwitch(switches::kEnableViewport);
        parsedCommandLine->AppendSwitch(cc::switches::kDisableCompositedAntialiasing);
    }

#if QT_CONFIG(webengine_vulkan)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Vulkan) {
        enableFeatures.push_back(features::kVulkan.name);
        parsedCommandLine->AppendSwitchASCII(switches::kUseVulkan,
                                             switches::kVulkanImplementationNameNative);
    }
#endif

#if defined(Q_OS_WIN)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11) {
        const QString luid = getAdapterLuid();
        if (!luid.isEmpty())
            parsedCommandLine->AppendSwitchASCII(switches::kUseAdapterLuid, luid.toStdString());
    }
#endif

    initializeFeatureList(parsedCommandLine, enableFeatures, disableFeatures);

    GLContextHelper::initialize();

    // If user requested GL support instead of using Skia rendering to
    // bitmaps, use software rendering via software OpenGL. This might be less
    // performant, but at least provides WebGL support.
    // TODO(miklocek), check if this still works with latest chromium
    const bool disableGpu = parsedCommandLine->HasSwitch(switches::kDisableGpu);
    const char *glType = getGLType(enableGLSoftwareRendering, disableGpu);

    parsedCommandLine->AppendSwitchASCII(switches::kUseGL, glType);
    parsedCommandLine->AppendSwitch(switches::kInProcessGPU);

    if (glType != gl::kGLImplementationDisabledName) {
        if (enableGLSoftwareRendering) {
            parsedCommandLine->AppendSwitch(switches::kDisableGpuRasterization);
            parsedCommandLine->AppendSwitch(switches::kIgnoreGpuBlocklist);
        }
#if QT_CONFIG(opengl)
        if (glType != gl::kGLImplementationANGLEName) {
            const QSurfaceFormat sharedFormat = QOpenGLContext::globalShareContext()->format();
            if (sharedFormat.profile() == QSurfaceFormat::CompatibilityProfile)
                parsedCommandLine->AppendSwitch(switches::kCreateDefaultGLContext);
#if defined(Q_OS_WIN)
            // This switch is used in Chromium's gl_context_wgl.cc file to determine whether to create
            // an OpenGL Core Profile context. If the switch is not set, it would always try to create a
            // Core Profile context, even if Qt uses a legacy profile, which causes
            // "Could not share GL contexts" warnings, because it's not possible to share between Core and
            // legacy profiles. See GLContextWGL::Initialize().
            if (sharedFormat.renderableType() == QSurfaceFormat::OpenGL
                && sharedFormat.profile() != QSurfaceFormat::CoreProfile)
                parsedCommandLine->AppendSwitch(switches::kDisableES3GLContext);
#endif
        }
#endif //QT_CONFIG(opengl)
    } else if (!disableGpu) {
        parsedCommandLine->AppendSwitch(switches::kDisableGpu);
    }

    logContext(glType, parsedCommandLine);

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
    m_contentRunner->Initialize(std::move(contentMainParams));

    mojo::core::Configuration mojoConfiguration;
    mojoConfiguration.is_broker_process = true;
    mojo::core::Init(mojoConfiguration);

    // This block mirrors ContentMainRunnerImpl::RunBrowser():
    m_mainDelegate->PreBrowserMain();
    base::MessagePump::OverrideMessagePumpForUIFactory(messagePumpFactory);
    content::BrowserTaskExecutor::Create();
    m_mainDelegate->PostEarlyInitialization({});
    content::StartBrowserThreadPool();
    content::BrowserTaskExecutor::PostFeatureListSetup();
    tracing::InitTracingPostThreadPoolStartAndFeatureList(false);
    base::PowerMonitor::Initialize(std::make_unique<base::PowerMonitorDeviceSource>());
    content::ProcessVisibilityTracker::GetInstance();
    m_discardableSharedMemoryManager = std::make_unique<discardable_memory::DiscardableSharedMemoryManager>();
    power_scheduler::PowerModeArbiter::GetInstance()->OnThreadPoolAvailable();

    m_mojoIpcSupport = std::make_unique<content::MojoIpcSupport>(content::BrowserTaskExecutor::CreateIOThread());
    download::SetIOTaskRunner(m_mojoIpcSupport->io_thread()->task_runner());
    std::unique_ptr<content::StartupData> startupData = m_mojoIpcSupport->CreateBrowserStartupData();

    // Once the MessageLoop has been created, attach a top-level RunLoop.
    m_runLoop.reset(new base::RunLoop);
    m_runLoop->BeforeRun();

    content::MainFunctionParams mainParams(base::CommandLine::ForCurrentProcess());
    mainParams.startup_data = std::move(startupData);
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

#if QT_CONFIG(webengine_pepper_plugins)
    // Creating pepper plugins from the page (which calls PluginService::GetPluginInfoArray)
    // might fail unless the page queried the list of available plugins at least once
    // (which ends up calling PluginService::GetPlugins). Since the plugins list can only
    // be created from the FILE thread, and that GetPluginInfoArray is synchronous, it
    // can't loads plugins synchronously from the IO thread to serve the render process' request
    // and we need to make sure that it happened beforehand.
    content::PluginService::GetInstance()->GetPlugins(base::BindOnce(&dummyGetPluginCallback));
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

#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
WebRtcLogUploader *WebEngineContext::webRtcLogUploader()
{
    if (!m_webrtcLogUploader)
        m_webrtcLogUploader = std::make_unique<WebRtcLogUploader>();
    return m_webrtcLogUploader.get();
}
#endif


base::CommandLine *WebEngineContext::initCommandLine(bool &useEmbeddedSwitches,
                                                     bool &enableGLSoftwareRendering)
{
    if (!base::CommandLine::CreateEmpty())
        qFatal("base::CommandLine has been initialized unexpectedly.");

    QStringList appArgs = QCoreApplication::arguments();
    if (appArgs.empty()) {
        qFatal("Argument list is empty, the program name is not passed to QCoreApplication. "
               "base::CommandLine cannot be properly initialized.");
    }

    base::CommandLine *parsedCommandLine = base::CommandLine::ForCurrentProcess();
    if (qEnvironmentVariableIsSet(kChromiumFlagsEnv)) {
        appArgs = appArgs.mid(0, 1); // Take application name and drop the rest
        appArgs.append(parseEnvCommandLine(qEnvironmentVariable(kChromiumFlagsEnv)));
    } else {
        int index = appArgs.indexOf(QLatin1String("--webEngineArgs"));
        if (index > -1) {
            appArgs.erase(appArgs.begin() + 1, appArgs.begin() + index + 1);
        } else {
            appArgs = appArgs.mid(0, 1);
        }
    }
#if defined(QTWEBENGINE_EMBEDDED_SWITCHES)
    useEmbeddedSwitches = !appArgs.contains(QStringLiteral("--disable-embedded-switches"));
#else
    useEmbeddedSwitches = appArgs.contains(QStringLiteral("--enable-embedded-switches"));
#endif
    enableGLSoftwareRendering =
            appArgs.removeAll(QStringLiteral("--enable-webgl-software-rendering"));
    appArgs.removeAll(QStringLiteral("--disable-embedded-switches"));
    appArgs.removeAll(QStringLiteral("--enable-embedded-switches"));

    base::CommandLine::StringVector argv;
    argv.resize(appArgs.size());
#if defined(Q_OS_WIN)
    for (int i = 0; i < appArgs.size(); ++i)
        argv[i] = appArgs[i].toStdWString();
#else
    for (int i = 0; i < appArgs.size(); ++i)
        argv[i] = appArgs[i].toStdString();
#endif
    parsedCommandLine->InitFromArgv(argv);

    return parsedCommandLine;
}

bool WebEngineContext::closingDown()
{
    return m_closingDown;
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
    return "119.0.6045.123"; // FIXME: Remember to update
}

QT_END_NAMESPACE
