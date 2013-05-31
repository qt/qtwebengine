#include "blinqapplication.h"

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
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_content_browser_client.h"
#include "webkit/common/user_agent/user_agent_util.h"

#include "web_contents_view_qt.h"

namespace {

static inline base::FilePath::StringType qStringToStringType(const QString &str)
{
#if defined(OS_POSIX)
    return str.toStdString();
#elif defined(OS_WIN)
    return str.toStdWString();
#endif
}

static QByteArray blinqProcessPath() {
    static bool initialized = false;
#ifdef BLINQ_PROCESS_PATH
    static QByteArray processPath(BLINQ_PROCESS_PATH);
#else
    static QByteArray processPath;
#endif
    if (initialized)
        return processPath;
    // Allow overriding at runtime for the time being.
    const QByteArray fromEnv = qgetenv("BLINQ_PROCESS_PATH");
    if (!fromEnv.isEmpty())
        processPath = fromEnv;
    if (processPath.isEmpty())
        qFatal("BLINQ_PROCESS_PATH environment variable not set or empty.");
    initialized = true;
    return processPath;
}

static void initializeBlinkPaths()
{
    static bool initialized = false;
    if (initialized)
        return;

    PathService::Override(content::CHILD_PROCESS_EXE, base::FilePath(qStringToStringType(QString(blinqProcessPath()))));
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

class MessagePump : public QObject,
                    public base::MessagePump
{
public:
    MessagePump()
        : m_delegate(0)
    {
    }

    virtual void Run(Delegate *delegate)
    {
        // It would be possible to do like the Android message loop and use
        // Start(Delegate*) instead of Run to avoid blocking, but we still
        // need to grab the command line arguments, so keep it simple for now
        // by forcing the use of BlinqApplication.
        m_delegate = delegate;
        QApplication::exec();
        m_delegate = 0;
    }

    virtual void Quit()
    {
        QCoreApplication::instance()->quit();
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
    return new MessagePump;
}

class ContentBrowserClientQt : public content::ShellContentBrowserClient
{
public:
    virtual content::WebContentsViewPort* OverrideCreateWebContentsView(content::WebContents* web_contents, content::RenderViewHostDelegateView** render_view_host_delegate_view)
    {
      fprintf(stderr, "OverrideCreateWebContentsView\n");
        WebContentsViewQt* rv = new WebContentsViewQt(web_contents);
        *render_view_host_delegate_view = rv;
        return rv;
    }
};

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

BlinqApplication::BlinqApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    {
        int myArgc = argc + 3;
        const char **myArgv = new const char *[myArgc];

        for (int i = 0; i < argc; ++i)
            myArgv[i] = argv[i];
        QByteArray subProcessPathOption("--browser-subprocess-path=");
        subProcessPathOption.append(blinqProcessPath());
        myArgv[argc] = subProcessPathOption.constData();
        myArgv[argc + 1] = "--no-sandbox";

        std::string ua = webkit_glue::BuildUserAgentFromProduct("Qrome/0.1");

        QByteArray userAgentParameter("--user-agent=");
        userAgentParameter.append(QString::fromStdString(ua).toUtf8());
        myArgv[argc + 2] = userAgentParameter.constData();

        CommandLine::Init(myArgc, myArgv);

        delete [] myArgv;
    }

    base::MessageLoop::InitMessagePumpForUIFactory(::messagePumpFactory);

    static content::ContentMainRunner *runner = 0;
    if (!runner) {
        runner = content::ContentMainRunner::Create();
        runner->Initialize(0, 0, new ContentMainDelegateQt);
    }

    initializeBlinkPaths();

    static content::BrowserMainRunner *browserRunner = 0;
    if (!browserRunner) {
        //CommandLine::Init(0, 0);

        browserRunner = content::BrowserMainRunner::Create();

        browserRunner->Initialize(content::MainFunctionParams(*CommandLine::ForCurrentProcess()));
    }

    base::ThreadRestrictions::SetIOAllowed(true);
}

int BlinqApplication::exec()
{
    base::RunLoop runLoop;
    runLoop.Run();
}
