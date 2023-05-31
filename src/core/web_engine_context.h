// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_ENGINE_CONTEXT_H
#define WEB_ENGINE_CONTEXT_H

#include "qtwebenginecoreglobal_p.h"

#include "build_config_qt.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"

#include <QtGui/qtgui-config.h>
#include <QList>

namespace base {
class RunLoop;
class CommandLine;
}

namespace content {
class BrowserMainRunner;
class ContentMainRunner;
class GpuThreadController;
class InProcessChildThreadParams;
class MojoIpcSupport;
}

namespace discardable_memory {
class DiscardableSharedMemoryManager;
}

namespace gpu {
struct GpuPreferences;
}

#if QT_CONFIG(webengine_printing_and_pdf)
namespace printing {
class PrintJobManager;
}
#endif

#ifdef Q_OS_WIN
namespace sandbox {
struct SandboxInterfaceInfo;
}
#endif

QT_FORWARD_DECLARE_CLASS(QObject)
class WebRtcLogUploader;

namespace QtWebEngineCore {

class AccessibilityActivationObserver;
class ContentMainDelegateQt;
class DevToolsServerQt;
class ProfileAdapter;

bool usingSoftwareDynamicGL();

typedef std::tuple<bool, QString, QString> ProxyAuthentication;

class WebEngineContext : public base::RefCounted<WebEngineContext> {
public:
    static WebEngineContext *current();
    static void destroyContextPostRoutine();
    static ProxyAuthentication qProxyNetworkAuthentication(QString host, int port);
    static void flushMessages();
    static bool closingDown();
    ProfileAdapter *createDefaultProfileAdapter();
    ProfileAdapter *defaultProfileAdapter();

    QObject *globalQObject();
#if QT_CONFIG(webengine_printing_and_pdf)
    printing::PrintJobManager* getPrintJobManager();
#endif
#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
    WebRtcLogUploader *webRtcLogUploader();
#endif
    void destroyProfileAdapter();
    void addProfileAdapter(ProfileAdapter *profileAdapter);
    void removeProfileAdapter(ProfileAdapter *profileAdapter);
    void destroy();
    static base::CommandLine *initCommandLine(bool &useEmbeddedSwitches,
                                              bool &enableGLSoftwareRendering);

    static bool isGpuServiceOnUIThread();

private:
    friend class base::RefCounted<WebEngineContext>;
    friend class ProfileAdapter;
    WebEngineContext();
    ~WebEngineContext();

    static void registerMainThreadFactories();
    static void destroyGpuProcess();

    std::unique_ptr<base::RunLoop> m_runLoop;
    std::unique_ptr<ContentMainDelegateQt> m_mainDelegate;
    std::unique_ptr<content::ContentMainRunner> m_contentRunner;
    std::unique_ptr<content::BrowserMainRunner> m_browserRunner;
    std::unique_ptr<discardable_memory::DiscardableSharedMemoryManager> m_discardableSharedMemoryManager;
    std::unique_ptr<content::MojoIpcSupport> m_mojoIpcSupport;
    std::unique_ptr<QObject> m_globalQObject;
    std::unique_ptr<ProfileAdapter> m_defaultProfileAdapter;
    std::unique_ptr<DevToolsServerQt> m_devtoolsServer;
    QList<ProfileAdapter*> m_profileAdapters;
#if QT_CONFIG(accessibility)
    std::unique_ptr<AccessibilityActivationObserver> m_accessibilityActivationObserver;
#endif

#if QT_CONFIG(webengine_printing_and_pdf)
    std::unique_ptr<printing::PrintJobManager> m_printJobManager;
#endif
#if QT_CONFIG(webengine_webrtc) && QT_CONFIG(webengine_extensions)
    std::unique_ptr<WebRtcLogUploader> m_webrtcLogUploader;
#endif
    static scoped_refptr<QtWebEngineCore::WebEngineContext> m_handle;
    static bool m_destroyed;
    static bool m_closingDown;
};

} // namespace

#endif // WEB_ENGINE_CONTEXT_H
