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
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/app/content_main_delegate.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/main_function_params.h"
#include "webkit/common/user_agent/user_agent_util.h"

#include "content_browser_client_qt.h"
#include <QCoreApplication>
#include <QStringList>

namespace {

static WebEngineContext* sContext = 0;

static inline base::FilePath::StringType qStringToStringType(const QString &str)
{
#if defined(OS_POSIX)
    return str.toStdString();
#elif defined(OS_WIN)
    return str.toStdWString();
#endif
}

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

static void initializeBlinkPaths()
{
    static bool initialized = false;
    if (initialized)
        return;

    PathService::Override(content::CHILD_PROCESS_EXE, base::FilePath(qStringToStringType(QString(subProcessPath()))));
}

// Return a timeout suitable for the glib loop, -1 to block forever,
// 0 to return right away, or a timeout in milliseconds from now.
int GetTimeIntervalMilliseconds(const base::TimeTicks& from) {
  if (from.is_null())
    return -1;

  // Be careful here.  TimeDelta has a precision of microseconds, but we want a
  // value in milliseconds.  If there are 5.5ms left, should the delay be 5 or
  // 6?  It should be 6 to avoid executing delayed work too early.
  int delay = static_cast<int>(
      ceil((from - base::TimeTicks::Now()).InMillisecondsF()));

  // If this value is negative, then we need to run delayed work soon.
  return delay < 0 ? 0 : delay;
}

class MessagePumpForUIQt : public QObject,
                           public base::MessagePump
{
public:
    MessagePumpForUIQt()
        // Usually this gets passed through Run, but since we have
        // our own event loop, attach it explicitely ourselves.
        : m_delegate(base::MessageLoopForUI::current())
    {
    }

    virtual void Run(Delegate *delegate)
    {
        // FIXME: This could be needed if we want to run Chromium tests.
        // We could run a QEventLoop here.
        Q_ASSERT(false);
    }

    virtual void Quit()
    {
        Q_ASSERT(false);
    }

    virtual void ScheduleWork()
    {
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

    virtual void ScheduleDelayedWork(const base::TimeTicks &delayed_work_time)
    {
        startTimer(GetTimeIntervalMilliseconds(delayed_work_time));
    }

protected:
    virtual void customEvent(QEvent *ev)
    {
        if (handleScheduledWork())
            QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }

    virtual void timerEvent(QTimerEvent *ev)
    {
        killTimer(ev->timerId());

        base::TimeTicks next_delayed_work_time;
        m_delegate->DoDelayedWork(&next_delayed_work_time);

        if (!next_delayed_work_time.is_null())
            startTimer(GetTimeIntervalMilliseconds(next_delayed_work_time));
    }

private:
    bool handleScheduledWork() {
        bool more_work_is_plausible = m_delegate->DoWork();

        base::TimeTicks delayed_work_time;
        more_work_is_plausible |= m_delegate->DoDelayedWork(&delayed_work_time);

        if (more_work_is_plausible)
            return true;

        more_work_is_plausible |= m_delegate->DoIdleWork();
        if (!more_work_is_plausible && !delayed_work_time.is_null())
            startTimer(GetTimeIntervalMilliseconds(delayed_work_time));

        return more_work_is_plausible;
    }

    Delegate *m_delegate;
};

base::MessagePump* messagePumpFactory()
{
    return new MessagePumpForUIQt;
}

class ContentMainDelegateQt : public content::ContentMainDelegate
{
public:
    content::ContentBrowserClient* CreateContentBrowserClient()
    {
        m_browserClient.reset(new ContentBrowserClientQt);
        return m_browserClient.get();
    }

private:
    scoped_ptr<ContentBrowserClientQt> m_browserClient;
};

}

WebEngineContext::WebEngineContext()
{
    Q_ASSERT(!sContext);
    sContext = this;

    {
        QByteArray subProcessPathOption("--browser-subprocess-path=");
        subProcessPathOption.append(subProcessPath());

        std::string ua = webkit_glue::BuildUserAgentFromProduct("QtWebEngine/0.1");

        QByteArray userAgentParameter("--user-agent=");
        userAgentParameter.append(QString::fromStdString(ua).toUtf8());

        const QStringList args = QCoreApplication::arguments();
        const int argc = args.size() + 3;
        const char* argv[argc];
        int i = 0;
        for(; i < args.size(); ++i)
            argv[i] = args.at(i).toLatin1().constData();
        argv[i++] = subProcessPathOption.constData();
        argv[i++] = "--no-sandbox";
        argv[i] = userAgentParameter.constData();

        CommandLine::Init(argc, argv);
    }

    // This needs to be set before the MessageLoop is created by BrowserMainRunner.
    base::MessageLoop::InitMessagePumpForUIFactory(::messagePumpFactory);

    static content::ContentMainRunner *runner = 0;
    if (!runner) {
        runner = content::ContentMainRunner::Create();
        runner->Initialize(0, 0, new ContentMainDelegateQt);
    }

    initializeBlinkPaths();

    static content::BrowserMainRunner *browserRunner = 0;
    if (!browserRunner) {
        browserRunner = content::BrowserMainRunner::Create();
        browserRunner->Initialize(content::MainFunctionParams(*CommandLine::ForCurrentProcess()));
    }

    base::ThreadRestrictions::SetIOAllowed(true);

    // Once the MessageLoop has been created, attach a top-level RunLoop.
    m_runLoop.reset(new base::RunLoop);
    m_runLoop->BeforeRun();
}

WebEngineContext::~WebEngineContext()
{
    m_runLoop->AfterRun();

    Q_ASSERT(sContext == this);
    sContext = 0;
}

scoped_refptr<WebEngineContext> WebEngineContext::current()
{
    scoped_refptr<WebEngineContext> current = sContext;
    if (!current)
        current = new WebEngineContext;
    Q_ASSERT(sContext == current);
    return current;
}
