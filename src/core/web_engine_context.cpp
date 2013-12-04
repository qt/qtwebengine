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

#include "web_engine_context.h"

#include <math.h>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/run_loop.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "ui/gl/gl_switches.h"
#include "webkit/common/user_agent/user_agent_util.h"

#include "content_browser_client_qt.h"
#include "content_client_qt.h"
#include "content_main_delegate_qt.h"
#include "type_conversion.h"
#include <QGuiApplication>
#include <QStringList>
#include <qpa/qplatformnativeinterface.h>

namespace {

scoped_refptr<WebEngineContext> sContext;

} // namespace

WebEngineContext::~WebEngineContext()
{
    m_runLoop->AfterRun();
}

scoped_refptr<WebEngineContext> WebEngineContext::currentOrCreate(WebContentsAdapterClient::RenderingMode renderingMode)
{
    if (!sContext)
        sContext = new WebEngineContext(renderingMode);
    else if (renderingMode != sContext->renderingMode())
        qFatal("Switching the QtWebEngine rendering mode once initialized in an application is not supported."
            " If you're using both a QQuickWebView and a QtQuick WebEngineView, make sure that the"
            " later is configured to use software rendering by setting:"
            "\nqApp->setProperty(\"QQuickWebEngineView_DisableHardwareAcceleration\", QVariant(true));");
    return sContext;
}

scoped_refptr<WebEngineContext> WebEngineContext::current()
{
    return sContext;
}

WebContentsAdapterClient::RenderingMode WebEngineContext::renderingMode()
{
    return CommandLine::ForCurrentProcess()->HasSwitch(switches::kEnableDelegatedRenderer)
        ? WebContentsAdapterClient::HardwareAccelerationMode
        : WebContentsAdapterClient::SoftwareRenderingMode;
}

WebEngineContext::WebEngineContext(WebContentsAdapterClient::RenderingMode renderingMode)
    : m_mainDelegate(new ContentMainDelegateQt)
    , m_contentRunner(content::ContentMainRunner::Create())
    , m_browserRunner(content::BrowserMainRunner::Create())
{
    QList<QByteArray> args;
    Q_FOREACH (const QString& arg, QCoreApplication::arguments())
        args << arg.toUtf8();
    const char* argv[args.size()];
    for (int i = 0; i < args.size(); ++i)
        argv[i] = args[i].constData();
    CommandLine::Init(args.size(), argv);

    CommandLine* parsedCommandLine = CommandLine::ForCurrentProcess();
    parsedCommandLine->AppendSwitchASCII(switches::kUserAgent, webkit_glue::BuildUserAgentFromProduct("QtWebEngine/0.1"));
    parsedCommandLine->AppendSwitch(switches::kNoSandbox);
    parsedCommandLine->AppendSwitch(switches::kDisablePlugins);

    if (renderingMode == WebContentsAdapterClient::HardwareAccelerationMode && !parsedCommandLine->HasSwitch(switches::kDisableDelegatedRenderer)) {
        parsedCommandLine->AppendSwitch(switches::kEnableDelegatedRenderer);
        parsedCommandLine->AppendSwitch(switches::kEnableThreadedCompositing);
        parsedCommandLine->AppendSwitch(switches::kInProcessGPU);
    }

#if defined(ANDROID)
    // Force single-process mode for now.
    parsedCommandLine->AppendSwitch(switches::kSingleProcess);
    // This is needed so that we do not assert in single process mode.
    parsedCommandLine->AppendSwitch(switches::kEnableThreadedCompositing);
#endif

    // Tell Chromium to use EGL instead of GLX if the Qt xcb plugin also does.
    if (qApp->platformName() == QStringLiteral("xcb") && qApp->platformNativeInterface()->nativeResourceForWindow(QByteArrayLiteral("egldisplay"), 0))
        parsedCommandLine->AppendSwitchASCII(switches::kUseGL, gfx::kGLImplementationEGLName);

    m_contentRunner->Initialize(0, 0, m_mainDelegate.get());
    m_browserRunner->Initialize(content::MainFunctionParams(*CommandLine::ForCurrentProcess()));

    // Once the MessageLoop has been created, attach a top-level RunLoop.
    m_runLoop.reset(new base::RunLoop);
    m_runLoop->BeforeRun();
}
