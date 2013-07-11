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

#include "content_browser_client_qt.h"

#include "base/message_loop/message_loop.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/main_function_params.h"

#include "browser_context_qt.h"
#include "web_contents_view_qt.h"

#include <QCoreApplication>

namespace {

ContentBrowserClientQt* gBrowserClient = 0; // Owned by ContentMainDelegateQt.

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
        // our own event loop, attach it explicitly ourselves.
        : m_delegate(base::MessageLoopForUI::current())
    {
    }

    virtual void Run(Delegate *delegate)
    {
        // FIXME: This could be needed if we want to run Chromium tests.
        // We could run a QEventLoop here.
        Q_UNREACHABLE();
    }

    virtual void Quit()
    {
        Q_UNREACHABLE();
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

} // namespace

class BrowserMainPartsQt : public content::BrowserMainParts
{
public:
    BrowserMainPartsQt()
        : content::BrowserMainParts()
    { }

    void PreMainMessageLoopStart() Q_DECL_OVERRIDE
    {
        base::MessageLoop::InitMessagePumpForUIFactory(::messagePumpFactory);
    }

    void PreMainMessageLoopRun() Q_DECL_OVERRIDE
    {
        m_browserContext.reset(new BrowserContextQt());
    }

    void PostMainMessageLoopRun()
    {
        m_browserContext.reset();
    }

    int PreCreateThreads() Q_DECL_OVERRIDE
    {
        base::ThreadRestrictions::SetIOAllowed(true);
        return 0;
    }

    BrowserContextQt* browser_context() const {
        return m_browserContext.get();
    }

private:
    scoped_ptr<BrowserContextQt> m_browserContext;

    DISALLOW_COPY_AND_ASSIGN(BrowserMainPartsQt);
};

content::WebContentsViewPort* ContentBrowserClientQt::OverrideCreateWebContentsView(content::WebContents* web_contents,
                                                                                    content::RenderViewHostDelegateView** render_view_host_delegate_view)
{
    WebContentsViewQt* rv = new WebContentsViewQt(web_contents);
    *render_view_host_delegate_view = rv;
    return rv;
}

ContentBrowserClientQt::ContentBrowserClientQt()
    : m_browserMainParts(0)
{
    Q_ASSERT(!gBrowserClient);
    gBrowserClient = this;
}

ContentBrowserClientQt::~ContentBrowserClientQt()
{
    gBrowserClient = 0;
}

ContentBrowserClientQt *ContentBrowserClientQt::Get()
{
    return gBrowserClient;
}

content::BrowserMainParts *ContentBrowserClientQt::CreateBrowserMainParts(const content::MainFunctionParams&)
{
    m_browserMainParts = new BrowserMainPartsQt;
    return m_browserMainParts;
}


BrowserContextQt* ContentBrowserClientQt::browser_context() {
    Q_ASSERT(m_browserMainParts);
    return static_cast<BrowserMainPartsQt*>(m_browserMainParts)->browser_context();
}

net::URLRequestContextGetter* ContentBrowserClientQt::CreateRequestContext(content::BrowserContext* content_browser_context, content::ProtocolHandlerMap* protocol_handlers)
{
    if (content_browser_context != browser_context())
        fprintf(stderr, "Warning: off the record browser context not implemented !\n");
    return static_cast<BrowserContextQt*>(browser_context())->CreateRequestContext(protocol_handlers);
}
