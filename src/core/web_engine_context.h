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

#ifndef WEB_ENGINE_CONTEXT_H
#define WEB_ENGINE_CONTEXT_H

#include "qtwebenginecoreglobal_p.h"

#include "build_config_qt.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"

#include <QtGui/qtgui-config.h>
#include <QVector>

namespace base {
class RunLoop;
class CommandLine;
}

namespace content {
class BrowserMainRunner;
class ContentMainRunner;
class GpuProcess;
class GpuThreadController;
class InProcessChildThreadParams;
class ServiceManagerEnvironment;
struct StartupData;
}

namespace discardable_memory {
class DiscardableSharedMemoryManager;
}

namespace gpu {
struct GpuPreferences;
class SyncPointManager;
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

namespace QtWebEngineCore {

class AccessibilityActivationObserver;
class ContentMainDelegateQt;
class DevToolsServerQt;
class ProfileAdapter;

bool usingSoftwareDynamicGL();

#ifdef Q_OS_WIN
Q_WEBENGINECORE_PRIVATE_EXPORT sandbox::SandboxInterfaceInfo *staticSandboxInterfaceInfo(sandbox::SandboxInterfaceInfo *info = nullptr);
#endif

typedef std::tuple<bool, QString, QString> ProxyAuthentication;

class WebEngineContext : public base::RefCounted<WebEngineContext> {
public:
    static WebEngineContext *current();
    static void destroyContextPostRoutine();
    static ProxyAuthentication qProxyNetworkAuthentication(QString host, int port);

    ProfileAdapter *createDefaultProfileAdapter();
    ProfileAdapter *defaultProfileAdapter();

    QObject *globalQObject();
#if QT_CONFIG(webengine_printing_and_pdf)
    printing::PrintJobManager* getPrintJobManager();
#endif
    void destroyProfileAdapter();
    void addProfileAdapter(ProfileAdapter *profileAdapter);
    void removeProfileAdapter(ProfileAdapter *profileAdapter);
    void destroy();
    static base::CommandLine* commandLine();

    static gpu::SyncPointManager *syncPointManager();

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
    std::unique_ptr<content::StartupData> m_startupData;
    std::unique_ptr<content::ServiceManagerEnvironment> m_serviceManagerEnvironment;
    std::unique_ptr<QObject> m_globalQObject;
    std::unique_ptr<ProfileAdapter> m_defaultProfileAdapter;
    std::unique_ptr<DevToolsServerQt> m_devtoolsServer;
    QVector<ProfileAdapter*> m_profileAdapters;
#if QT_CONFIG(accessibility)
    std::unique_ptr<AccessibilityActivationObserver> m_accessibilityActivationObserver;
#endif

#if QT_CONFIG(webengine_printing_and_pdf)
    std::unique_ptr<printing::PrintJobManager> m_printJobManager;
#endif
    static scoped_refptr<QtWebEngineCore::WebEngineContext> m_handle;
    static bool m_destroyed;
    static QAtomicPointer<gpu::SyncPointManager> s_syncPointManager;
};

} // namespace

#endif // WEB_ENGINE_CONTEXT_H
