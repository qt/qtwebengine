// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "web_engine_context.h"

#include <map>
#include <math.h>
#include <QtGui/private/qrhi_p.h>

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
#include "components/viz/common/features.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/app/mojo_ipc_support.h"
#include "content/browser/devtools/devtools_http_handler.h"
#include "content/browser/gpu/gpu_main_thread_factory.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/scheduler/browser_task_executor.h"
#include "content/browser/startup_data_impl.h"
#include "content/browser/startup_helper.h"
#include "content/browser/utility_process_host.h"
#include "content/gpu/in_process_gpu_thread.h"
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
#include "services/tracing/public/cpp/trace_startup.h"
#include "services/tracing/public/cpp/tracing_features.h"
#include "third_party/blink/public/common/features.h"
#include "ui/base/ui_base_features.h"
#include "ui/events/event_switches.h"
#include "ui/native_theme/native_theme_features.h"
#include "ui/gl/gl_utils.h"
#include "ui/gl/gl_switches.h"
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
#include "type_conversion.h"
#include "web_engine_library_info.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QMutex>
#include <QOffscreenSurface>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QStringList>
#include <QSurfaceFormat>
#include <QNetworkProxy>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtQuick/private/qsgrhisupport_p.h>
#include <QLoggingCategory>

#if QT_CONFIG(opengl) && (defined(USE_OZONE) || defined(Q_OS_WIN))
#include "ozone/gl_context_qt.h"

#include <QOpenGLContext>
#include <qopenglcontext_platform.h>

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE
#endif

#define STRINGIFY_LITERAL(x) #x
#define STRINGIFY_EXPANDED(x) STRINGIFY_LITERAL(x)

namespace QtWebEngineCore {

Q_LOGGING_CATEGORY(webEngineContextLog, "qt.webenginecontext")

class GPUInfo
{
public:
    enum Vendor {
        Unknown = -1,

        // PCI-SIG-registered vendors
        AMD,
        Apple,
        ARM,
        Google,
        ImgTec,
        Intel,
        Microsoft,
        Nvidia,
        Qualcomm,
        Samsung,
        Broadcom,
        VMWare,
        VirtIO,

        // Khronos-registered vendors
        Vivante,
        VeriSilicon,
        Kazan,
        CodePlay,
        Mesa,
        PoCL,
    };

    static GPUInfo *instance()
    {
        static GPUInfo instance;
        return &instance;
    }

    static Vendor vendorIdToVendor(quint64 vendorId)
    {
        // clang-format off
        // Based on //third_party/angle/src/gpu_info_util/SystemInfo.h
        static const std::map<quint64, Vendor> vendorIdMap = {
            {0x0, Unknown},
            {0x1002, AMD},
            {0x106B, Apple},
            {0x13B5, ARM},
            {0x1AE0, Google},
            {0x1010, ImgTec},
            {0x8086, Intel},
            {0x1414, Microsoft},
            {0x10DE, Nvidia},
            {0x5143, Qualcomm},
            {0x144D, Samsung},
            {0x14E4, Broadcom},
            {0x15AD, VMWare},
            {0x1AF4, VirtIO},
            {0x10001, Vivante},
            {0x10002, VeriSilicon},
            {0x10003, Kazan},
            {0x10004, CodePlay},
            {0x10005, Mesa},
            {0x10006, PoCL},
        };
        // clang-format on

        auto it = vendorIdMap.find(vendorId);
        if (it != vendorIdMap.end())
            return it->second;

        qWarning() << "Unknown Vendor ID:" << QStringLiteral("0x%1").arg(vendorId, 0, 16);
        return Unknown;
    }

    static Vendor deviceNameToVendor(const QString &deviceName)
    {
        // TODO: Test and add more vendors to the list.
        if (deviceName.contains(QLatin1StringView("AMD"), Qt::CaseInsensitive))
            return AMD;
        if (deviceName.contains(QLatin1StringView("Intel"), Qt::CaseInsensitive))
            return Intel;
        if (deviceName.contains(QLatin1StringView("Nvidia"), Qt::CaseInsensitive))
            return Nvidia;

#if defined(USE_OZONE)
        if (deviceName.contains(QLatin1StringView("Mesa llvmpipe")))
            return Mesa;
#endif

#if defined(Q_OS_MACOS)
        if (deviceName.contains(QLatin1StringView("Apple")))
            return Apple;
#endif

        return Unknown;
    }

    static std::string vendorToString(Vendor vendor)
    {
        // clang-format off
        static const std::map<Vendor, std::string> vendorNameMap = {
            {Unknown, "Unknown"},
            {AMD, "AMD"},
            {Apple, "Apple"},
            {ARM, "ARM"},
            {Google, "Google"},
            {ImgTec, "Img Tec"},
            {Intel, "Intel"},
            {Microsoft, "Microsoft"},
            {Nvidia, "Nvidia"},
            {Qualcomm, "Qualcomm"},
            {Samsung, "Samsung"},
            {Broadcom, "Broadcom"},
            {VMWare, "VMWare"},
            {VirtIO, "VirtIO"},
            {Vivante, "Vivante"},
            {VeriSilicon, "VeriSilicon"},
            {Kazan, "Kazan"},
            {CodePlay, "CodePlay"},
            {Mesa, "Mesa"},
            {PoCL, "PoCL"},
        };
        // clang-format on

        auto it = vendorNameMap.find(vendor);
        if (it != vendorNameMap.end())
            return it->second;

        Q_UNREACHABLE();
        return "Unknown";
    }

    Vendor vendor() const { return m_vendor; }
    QString getAdapterLuid() const { return m_adapterLuid; }

private:
    GPUInfo()
    {
#if defined(Q_OS_WIN)
        {
            static const bool preferSoftwareDevice =
                    qEnvironmentVariableIntValue("QSG_RHI_PREFER_SOFTWARE_RENDERER");
            QRhiD3D11InitParams params;
            QRhi::Flags flags;
            if (preferSoftwareDevice) {
                flags |= QRhi::PreferSoftwareRenderer;
            }
            QScopedPointer<QRhi> d3d11Rhi(QRhi::create(QRhi::D3D11, &params, flags, nullptr));
            // mimic what QSGRhiSupport and QBackingStoreRhi does
            if (!d3d11Rhi && !preferSoftwareDevice) {
                flags |= QRhi::PreferSoftwareRenderer;
                d3d11Rhi.reset(QRhi::create(QRhi::D3D11, &params, flags, nullptr));
            }
            if (d3d11Rhi) {
                m_vendor = vendorIdToVendor(d3d11Rhi->driverInfo().vendorId);

                const QRhiD3D11NativeHandles *handles =
                        static_cast<const QRhiD3D11NativeHandles *>(d3d11Rhi->nativeHandles());
                Q_ASSERT(handles);
                m_adapterLuid = QString::number(handles->adapterLuidHigh) % QLatin1Char(',')
                        % QString::number(handles->adapterLuidLow);
            }
        }
#elif defined(Q_OS_MACOS)
        {
            QRhiMetalInitParams params;
            QScopedPointer<QRhi> metalRhi(
                    QRhi::create(QRhi::Metal, &params, QRhi::Flags(), nullptr));
            if (metalRhi)
                m_vendor = deviceNameToVendor(QLatin1StringView(metalRhi->driverInfo().deviceName));
        }
#endif

#if QT_CONFIG(opengl)
        if (m_vendor == Unknown) {
            QRhiGles2InitParams params;
            params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
            QScopedPointer<QRhi> glRhi(
                    QRhi::create(QRhi::OpenGLES2, &params, QRhi::Flags(), nullptr));
            if (glRhi)
                m_vendor = deviceNameToVendor(QLatin1StringView(glRhi->driverInfo().deviceName));
        }
#endif

#if QT_CONFIG(webengine_vulkan)
        if (m_vendor == Unknown) {
            QVulkanInstance vulkanInstance;
            vulkanInstance.setApiVersion(QVersionNumber(1, 1));
            if (vulkanInstance.create()) {
                QRhiVulkanInitParams params;
                params.inst = &vulkanInstance;
                QScopedPointer<QRhi> vulkanRhi(
                        QRhi::create(QRhi::Vulkan, &params, QRhi::Flags(), nullptr));
                if (vulkanRhi) {
                    // TODO: The primary GPU is not necessarily the one which is connected to the
                    // display in case of a Multi-GPU setup on Linux. This can be workarounded by
                    // installing the Mesa's Device Selection Layer,
                    // see https://www.phoronix.com/news/Mesa-20.1-Vulkan-Dev-Selection
                    // Try to detect this case and at least warn about it.
                    m_vendor = vendorIdToVendor(vulkanRhi->driverInfo().vendorId);
                }
            }
        }
#endif

        if (m_vendor == Unknown)
            qWarning("Unable to detect GPU vendor.");
    }

    Vendor m_vendor = Unknown;
    QString m_adapterLuid;
};

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
        if (args.at(index).startsWith(QLatin1StringView("--device="))) {
            device = args.at(index).mid(9);
            break;
        }
    }

    if (device.isEmpty())
        device = qEnvironmentVariable("QT_QUICK_BACKEND");
    if (device.isEmpty())
        device = qEnvironmentVariable("QMLSCENE_DEVICE");

    return device.isEmpty() || device == QLatin1StringView("rhi");
}

#if QT_CONFIG(opengl)
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
    return openGLModule.fileName().contains(QLatin1StringView("opengl32sw"), Qt::CaseInsensitive);
#else
    return false;
#endif
}

static bool openGLPlatformSupport()
{
    return QGuiApplicationPrivate::platformIntegration()->hasCapability(
            QPlatformIntegration::OpenGL);
}

static std::string getGLType(bool enableGLSoftwareRendering, bool disableGpu)
{
    const bool tryGL =
            usingSupportedSGBackend() && !usingSoftwareDynamicGL() && openGLPlatformSupport();
    if (disableGpu || (!tryGL && !enableGLSoftwareRendering))
        return gl::kGLImplementationDisabledName;

#if defined(Q_OS_MACOS)
    return gl::kGLImplementationANGLEName;
#else
#if defined(Q_OS_WIN)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11
        || QQuickWindow::graphicsApi() == QSGRendererInterface::Vulkan) {
        return gl::kGLImplementationANGLEName;
    }
#endif

    if (!qt_gl_global_share_context() || !qt_gl_global_share_context()->isValid()) {
        qWarning("WebEngineContext is used before QtWebEngineQuick::initialize() or OpenGL context "
                 "creation failed.");
        return gl::kGLImplementationDisabledName;
    }

    const QSurfaceFormat sharedFormat = qt_gl_global_share_context()->format();

    switch (sharedFormat.renderableType()) {
    case QSurfaceFormat::OpenGL:
        if (sharedFormat.profile() == QSurfaceFormat::CoreProfile) {
            qWarning("An OpenGL Core Profile was requested, but it is not supported "
                     "on the current platform. Falling back to a non-Core profile. "
                     "Note that this might cause rendering issues.");
        }
        return gl::kGLImplementationDesktopName;
    case QSurfaceFormat::OpenGLES:
        return gl::kGLImplementationEGLName;
    case QSurfaceFormat::OpenVG:
    case QSurfaceFormat::DefaultRenderableType:
    default:
        // Shared contex created but no rederable type set.
        qWarning("Unsupported rendering surface format. Please open bug report at "
                 "https://bugreports.qt.io");
    }

    return gl::kGLImplementationDisabledName;
#endif // defined(Q_OS_MACOS)
}
#else
static std::string getGLType(bool /*enableGLSoftwareRendering*/, bool disableGpu)
{
    if (disableGpu)
        return gl::kGLImplementationDisabledName;
#if defined(Q_OS_MACOS)
    return gl::kGLImplementationANGLEName;
#elif defined(Q_OS_WIN)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11
        || QQuickWindow::graphicsApi() == QSGRendererInterface::Vulkan) {
        return gl::kGLImplementationANGLEName;
    }
#endif
    return gl::kGLImplementationDisabledName;
}
#endif // QT_CONFIG(opengl)

static std::string getVulkanType(base::CommandLine *cmd)
{
#if QT_CONFIG(webengine_vulkan)
    if (cmd->HasSwitch(switches::kUseVulkan))
        return cmd->GetSwitchValueASCII(switches::kUseVulkan);
#endif

    return "disabled";
}

static std::string getAngleType(const std::string &glType, base::CommandLine *cmd)
{
    if (glType == gl::kGLImplementationANGLEName) {
        if (cmd->HasSwitch(switches::kUseANGLE))
            return cmd->GetSwitchValueASCII(switches::kUseANGLE);

#if defined(Q_OS_WIN)
        return gl::kANGLEImplementationD3D11Name;
#elif defined(Q_OS_MACOS)
        return gl::kANGLEImplementationMetalName;
#else
        return gl::kANGLEImplementationDefaultName;
#endif
    }

    return "disabled";
}

#if QT_CONFIG(webengine_pepper_plugins)
void dummyGetPluginCallback(const std::vector<content::WebPluginInfo>&)
{
}
#endif

static void logContext(const std::string &glType, base::CommandLine *cmd)
{
    if (Q_UNLIKELY(webEngineContextLog().isDebugEnabled())) {
        QStringList log;
        log << QLatin1StringView("\n");

        log << QLatin1StringView("Chromium GL Backend: " + glType) << QLatin1StringView("\n");
        log << QLatin1StringView("Chromium ANGLE Backend: " + getAngleType(glType, cmd))
            << QLatin1StringView("\n");
        log << QLatin1StringView("Chromium Vulkan Backend: " + getVulkanType(cmd))
            << QLatin1StringView("\n");
        log << QLatin1StringView("\n");

        log << QLatin1StringView("QSG RHI Backend:") << QSGRhiSupport::instance()->rhiBackendName()
            << QLatin1StringView("\n");
        log << QLatin1StringView("QSG RHI Backend Supported:")
            << QLatin1StringView(usingSupportedSGBackend() ? "yes" : "no")
            << QLatin1StringView("\n");
        log << QLatin1StringView("GPU Vendor: "
                                 + GPUInfo::vendorToString(GPUInfo::instance()->vendor()))
            << QLatin1StringView("\n");
        log << QLatin1StringView("\n");

#if QT_CONFIG(opengl)
#if defined(USE_OZONE)
        log << QLatin1StringView("Using GLX:")
            << QLatin1StringView(GLContextHelper::getGlxPlatformInterface() ? "yes" : "no")
            << QLatin1StringView("\n");
        log << QLatin1StringView("Using EGL:")
            << QLatin1StringView(GLContextHelper::getEglPlatformInterface() ? "yes" : "no")
            << QLatin1StringView("\n");
#endif
#if defined(USE_OZONE) || defined(Q_OS_WIN)
        log << QLatin1StringView("Using Shared GL:")
            << QLatin1StringView(qt_gl_global_share_context() ? "yes" : "no")
            << QLatin1StringView("\n");
        if (qt_gl_global_share_context()) {
            log << QLatin1StringView("Using Software Dynamic GL:")
                << QLatin1StringView(usingSoftwareDynamicGL() ? "yes" : "no")
                << QLatin1StringView("\n");

            const QSurfaceFormat sharedFormat = qt_gl_global_share_context()
                    ? qt_gl_global_share_context()->format()
                    : QSurfaceFormat::defaultFormat();
            const auto profile =
                    QMetaEnum::fromType<QSurfaceFormat::OpenGLContextProfile>().valueToKey(
                            sharedFormat.profile());
            const auto type = QMetaEnum::fromType<QSurfaceFormat::RenderableType>().valueToKey(
                    sharedFormat.renderableType());
            log << QLatin1StringView("Surface Type:") << QLatin1StringView(type)
                << QLatin1StringView("\n");
            log << QLatin1StringView("Surface Profile:") << QLatin1StringView(profile)
                << QLatin1StringView("\n");
            log << QStringLiteral("Surface Version: %1.%2\n")
                            .arg(sharedFormat.majorVersion())
                            .arg(sharedFormat.minorVersion());
        }
        log << QLatin1StringView("\n");
#endif // defined(USE_OZONE) || defined(Q_OS_WIN)
#endif // QT_CONFIG(opengl)

        log << QLatin1StringView("Init Parameters:\n");
        const base::CommandLine::SwitchMap switchMap = cmd->GetSwitches();
        for (const auto &pair : switchMap)
            log << QStringLiteral(" *  %1 %2\n").arg(toQt(pair.first)).arg(toQt(pair.second));

        qCDebug(webEngineContextLog) << qPrintable(log.join(QLatin1Char(' ')));
    }
}

extern std::unique_ptr<base::MessagePump> messagePumpFactory();

static void setupProxyPac(base::CommandLine *commandLine)
{
    if (commandLine->HasSwitch(switches::kProxyPacUrl)) {
        QUrl pac_url(toQt(commandLine->GetSwitchValueASCII(switches::kProxyPacUrl)));
        if (pac_url.isValid()
            && (pac_url.isLocalFile()
                || !pac_url.scheme().compare(QLatin1StringView("qrc"), Qt::CaseInsensitive))) {
            QFile file;
            if (pac_url.isLocalFile())
                file.setFileName(pac_url.toLocalFile());
            else
                file.setFileName(pac_url.path().prepend(QLatin1Char(':')));
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

    // Normally the GPU thread is shut down when the GpuProcessHost is destroyed
    // on IO thread (triggered by ~BrowserMainRunner). But by that time the UI
    // task runner is not working anymore so we need to do this earlier.
    cleanupVizProcess();
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

#if QT_CONFIG(opengl) && (defined(USE_OZONE) || defined(Q_OS_WIN))
    // gpu thread is no longer around, so no more context is used, remove the helper
    GLContextHelper::destroy();
#endif

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

    // Do not advertise a feature we have removed at compile time
    parsedCommandLine->AppendSwitch(switches::kDisableSpeechAPI);

    std::vector<std::string> disableFeatures;
    std::vector<std::string> enableFeatures;

    enableFeatures.push_back(features::kNetworkServiceInProcess.name);
    enableFeatures.push_back(features::kTracingServiceInProcess.name);
#if defined(Q_OS_MACOS) && BUILDFLAG(USE_SCK)
    // The feature name should match the definition of kScreenCaptureKitMacScreen.
    enableFeatures.push_back("ScreenCaptureKitMacScreen");
#endif // defined(Q_OS_MACOS)

    // Avoid crashing when websites tries using this feature (since 83)
    disableFeatures.push_back(features::kInstalledApp.name);

    // Not implemented but it overrides the devtools eyedropper
    // Should be sync with kEyeDropper base::Feature
    parsedCommandLine->AppendSwitchASCII(switches::kDisableBlinkFeatures, "EyeDropperAPI");
    disableFeatures.push_back(features::kEyeDropper.name);

    // Explicitly tell Chromium about default-on features we do not support
    disableFeatures.push_back(features::kBackgroundFetch.name);
    parsedCommandLine->AppendSwitchASCII(switches::kDisableBlinkFeatures, "WebOTP");
    disableFeatures.push_back(features::kWebOTP.name);
    disableFeatures.push_back(features::kWebPayments.name);
    disableFeatures.push_back(features::kWebUsb.name);

    if (useEmbeddedSwitches) {
        // embedded switches are based on the switches for Android, see content/browser/android/content_startup_flags.cc
        enableFeatures.push_back(features::kOverlayScrollbar.name);
        parsedCommandLine->AppendSwitch(switches::kEnableViewport);
        parsedCommandLine->AppendSwitch(cc::switches::kDisableCompositedAntialiasing);
    }

#if defined(USE_OZONE)
    if (GPUInfo::instance()->vendor() == GPUInfo::Nvidia) {
        disableFeatures.push_back(media::kVaapiVideoDecodeLinux.name);
        parsedCommandLine->AppendSwitch(switches::kDisableGpuMemoryBufferVideoFrames);
    }

#if QT_CONFIG(webengine_vulkan)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::OpenGL) {
        // FIXME: We assume that ANGLE is explicitly enabled on Linux.
        //        Make sure to reimplement fallback if ANGLE becomes the default.
        bool usingANGLE = false;
        if (parsedCommandLine->HasSwitch(switches::kUseGL))
            usingANGLE = (parsedCommandLine->GetSwitchValueASCII(switches::kUseGL)
                          == gl::kGLImplementationANGLEName);
        if (usingANGLE && GPUInfo::instance()->vendor() == GPUInfo::Nvidia) {
            qWarning("Disable ANGLE because GBM is not supported with the current configuration. "
                     "Fallback to Vulkan rendering in Chromium.");
            parsedCommandLine->RemoveSwitch(switches::kUseANGLE);
            parsedCommandLine->RemoveSwitch(switches::kUseGL);
            parsedCommandLine->AppendSwitchASCII(switches::kUseGL,
                                                 gl::kGLImplementationDesktopName);
            parsedCommandLine->AppendSwitchASCII(switches::kUseVulkan,
                                                 switches::kVulkanImplementationNameNative);
            enableFeatures.push_back(features::kVulkan.name);
        }
    }

    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Vulkan && usingSupportedSGBackend()) {
        enableFeatures.push_back(features::kVulkan.name);
        parsedCommandLine->AppendSwitchASCII(switches::kUseVulkan,
                                             switches::kVulkanImplementationNameNative);
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
                qWarning().nospace()
                        << "Vulkan rendering may fail because " << deviceExtensionsVar
                        << " environment variable is already set but it doesn't contain"
                        << " some of the required Vulkan device extensions:\n"
                        << qPrintable(requiredDeviceExtensions.join('\n'));
            }
        } else {
            qputenv(deviceExtensionsVar, requiredDeviceExtensions.join(';'));
        }
    }
#endif // QT_CONFIG(webengine_vulkan)
#endif // defined(USE_OZONE)

#if defined(Q_OS_WIN)
    if (QQuickWindow::graphicsApi() == QSGRendererInterface::Direct3D11
        || QQuickWindow::graphicsApi() == QSGRendererInterface::Vulkan) {
        const QString luid = GPUInfo::instance()->getAdapterLuid();
        if (!luid.isEmpty())
            parsedCommandLine->AppendSwitchASCII(switches::kUseAdapterLuid, luid.toStdString());
    }
#endif
    // We need the FieldTrialList to make sure Chromium features are provided to child processes
    if (!base::FieldTrialList::GetInstance()) {
        m_fieldTrialList.reset(new base::FieldTrialList());
    }

    initializeFeatureList(parsedCommandLine, enableFeatures, disableFeatures);

#if QT_CONFIG(opengl) && (defined(USE_OZONE) || defined(Q_OS_WIN))
    GLContextHelper::initialize();
#endif

    // If user requested GL support instead of using Skia rendering to
    // bitmaps, use software rendering via software OpenGL. This might be less
    // performant, but at least provides WebGL support.
    // TODO(miklocek), check if this still works with latest chromium
    const bool disableGpu = parsedCommandLine->HasSwitch(switches::kDisableGpu);
    std::string glType;
    if (parsedCommandLine->HasSwitch(switches::kUseGL))
        glType = parsedCommandLine->GetSwitchValueASCII(switches::kUseGL);
    else {
        glType = getGLType(enableGLSoftwareRendering, disableGpu);
        parsedCommandLine->AppendSwitchASCII(switches::kUseGL, glType);
    }

    parsedCommandLine->AppendSwitch(switches::kInProcessGPU);

    if (glType != gl::kGLImplementationDisabledName) {
        if (enableGLSoftwareRendering) {
            parsedCommandLine->AppendSwitch(switches::kDisableGpuRasterization);
            parsedCommandLine->AppendSwitch(switches::kIgnoreGpuBlocklist);
        }
#if QT_CONFIG(opengl)
        if (glType != gl::kGLImplementationANGLEName) {
            QOpenGLContext *shareContext = QOpenGLContext::globalShareContext();
            Q_ASSERT(shareContext);
            const QSurfaceFormat sharedFormat = shareContext->format();
            if (sharedFormat.profile() == QSurfaceFormat::CompatibilityProfile)
                parsedCommandLine->AppendSwitch(switches::kCreateDefaultGLContext);
#if defined(Q_OS_WIN)
            // This switch is used in Chromium's gl_context_wgl.cc file to determine whether to create
            // an OpenGL Core Profile context. If the switch is not set, it would always try to create a
            // Core Profile context, even if Qt uses a legacy profile, which causes
            // "Could not share GL contexts" warnings, because it's not possible to share between Core and
            // legacy profiles. See GLContextWGL::Initialize().
            if (sharedFormat.renderableType() == QSurfaceFormat::OpenGL
                && sharedFormat.profile() != QSurfaceFormat::CoreProfile) {
                gl::GlWorkarounds workarounds = gl::GetGlWorkarounds();
                workarounds.disable_es3gl_context = true;
                gl::SetGlWorkarounds(workarounds);
            }
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
    int index = appArgs.indexOf(QRegularExpression(QLatin1StringView("--webEngineArgs"),
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
    useEmbeddedSwitches = !appArgs.contains(QStringLiteral("--disable-embedded-switches"));
#else
    useEmbeddedSwitches = appArgs.contains(QStringLiteral("--enable-embedded-switches"));
#endif
    enableGLSoftwareRendering =
            appArgs.removeAll(QStringLiteral("--enable-webgl-software-rendering"));
    appArgs.removeAll(QStringLiteral("--disable-embedded-switches"));
    appArgs.removeAll(QStringLiteral("--enable-embedded-switches"));

    bool isRemoteDebugPort =
            (-1
             != appArgs.indexOf(QRegularExpression(QStringLiteral("--remote-debugging-port=.*"),
                                                   QRegularExpression::CaseInsensitiveOption)))
            || !qEnvironmentVariable("QTWEBENGINE_REMOTE_DEBUGGING").isEmpty();
    bool isRemoteAllowOrigins =
            (-1
             != appArgs.indexOf(QRegularExpression(QStringLiteral("--remote-allow-origins=.*"),
                                                   QRegularExpression::CaseInsensitiveOption)));

    if (isRemoteDebugPort && !isRemoteAllowOrigins) {
        appArgs.append(QStringLiteral("--remote-allow-origins=*"));
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
    parsedCommandLine->InitFromArgv(argv);

    return parsedCommandLine;
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
    return "129.0.6668.70"; // FIXME: Remember to update
}

QT_END_NAMESPACE
