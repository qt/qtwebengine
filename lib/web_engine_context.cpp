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

#include "shared/shared_globals.h"
#include <math.h>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/app/content_main_delegate.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "webkit/common/user_agent/user_agent_util.h"

#include "content_browser_client_qt.h"
#include "type_conversion.h"
#include <QCoreApplication>
#include <QStringList>

namespace {

scoped_refptr<WebEngineContext> sContext;

static QByteArray subProcessPath() {
    static bool initialized = false;
#ifdef QTWEBENGINEPROCESS_PATH
    static QByteArray processPath(QTWEBENGINEPROCESS_PATH);
#else
    static QByteArray processPath;
#endif
    if (initialized)
        return processPath;
    // Allow overriding at runtime for the time being.
    const QByteArray fromEnv = qgetenv("QTWEBENGINEPROCESS_PATH");
    if (!fromEnv.isEmpty())
        processPath = fromEnv;
    if (processPath.isEmpty())
        qFatal("QTWEBENGINEPROCESS_PATH environment variable not set or empty.");
    initialized = true;
    return processPath;
}

} // namespace

class ContentMainDelegateQt : public content::ContentMainDelegate
{
public:

    // This is where the embedder puts all of its startup code that needs to run
    // before the sandbox is engaged.
    void PreSandboxStartup() Q_DECL_OVERRIDE
    {
        PathService::Override(base::FILE_EXE, base::FilePath(toFilePathString(subProcessPath())));
    }

    content::ContentBrowserClient* CreateContentBrowserClient() Q_DECL_OVERRIDE
    {
        m_browserClient.reset(new ContentBrowserClientQt);
        return m_browserClient.get();
    }

private:
    scoped_ptr<ContentBrowserClientQt> m_browserClient;
};

WebEngineContext::WebEngineContext()
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
    parsedCommandLine->AppendSwitchASCII(switches::kBrowserSubprocessPath, subProcessPath().constData());
    parsedCommandLine->AppendSwitch(switches::kNoSandbox);
    parsedCommandLine->AppendSwitch(switches::kDisablePlugins);

    m_contentRunner->Initialize(0, 0, m_mainDelegate.get());
    m_browserRunner->Initialize(content::MainFunctionParams(*CommandLine::ForCurrentProcess()));

    // Once the MessageLoop has been created, attach a top-level RunLoop.
    m_runLoop.reset(new base::RunLoop);
    m_runLoop->BeforeRun();
}

WebEngineContext::~WebEngineContext()
{
    m_runLoop->AfterRun();
}

scoped_refptr<WebEngineContext> WebEngineContext::current()
{
    if (!sContext)
        sContext = new WebEngineContext;
    return sContext;
}
