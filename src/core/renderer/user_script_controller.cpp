/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "user_script_controller.h"

#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_observer.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

#include "common/qt_messages.h"
#include "common/user_script_data.h"

Q_GLOBAL_STATIC(UserScriptController, qt_webengine_userScriptController)

static content::RenderView * const globalScriptsIndex = 0;

// Scripts meant to run after the load event will be run 500ms after DOMContentLoaded if the load event doesn't come within that delay.
static const int afterLoadTimeout = 500;

class UserScriptController::RenderViewObserverHelper : public content::RenderViewObserver
{
public:
    RenderViewObserverHelper(content::RenderView *);
private:
    // RenderViewObserver implementation.
    virtual void DidCreateDocumentElement(blink::WebLocalFrame* frame) Q_DECL_OVERRIDE;
    virtual void DidFinishDocumentLoad(blink::WebLocalFrame* frame) Q_DECL_OVERRIDE;
    virtual void DidFinishLoad(blink::WebLocalFrame* frame) Q_DECL_OVERRIDE;
    virtual void DidStartProvisionalLoad(blink::WebLocalFrame* frame) Q_DECL_OVERRIDE;
    virtual void FrameDetached(blink::WebFrame* frame) Q_DECL_OVERRIDE;
    virtual void OnDestruct() Q_DECL_OVERRIDE;
    virtual bool OnMessageReceived(const IPC::Message& message) Q_DECL_OVERRIDE;

    void onUserScriptAdded(const UserScriptData &);
    void onUserScriptRemoved(const UserScriptData &);
    void onScriptsCleared();

    void runScripts(UserScriptData::InjectionPoint, blink::WebLocalFrame *);
    QSet<blink::WebLocalFrame *> m_pendingFrames;
};

void UserScriptController::RenderViewObserverHelper::runScripts(UserScriptData::InjectionPoint p, blink::WebLocalFrame *frame)
{
    if (p == UserScriptData::AfterLoad && !m_pendingFrames.remove(frame))
        return;
    content::RenderView *renderView = content::RenderView::FromWebView(frame->view());
    const bool isMainFrame = (frame == renderView->GetWebView()->mainFrame());

    QList<uint64> scriptsToRun = UserScriptController::instance()->m_viewUserScriptMap.value(globalScriptsIndex).toList();
    scriptsToRun.append(UserScriptController::instance()->m_viewUserScriptMap.value(renderView).toList());

    Q_FOREACH (uint64 id, scriptsToRun) {
        const UserScriptData &script = UserScriptController::instance()->m_scripts.value(id);
        if (script.injectionPoint != p
                || (!script.injectForSubframes && !isMainFrame))
            continue;
        blink::WebScriptSource source(blink::WebString::fromUTF8(script.source), script.url);
        if (script.worldId)
            frame->executeScriptInIsolatedWorld(script.worldId, &source, /*numSources = */1, /*contentScriptExtentsionGroup = */ 1);
        else
            frame->executeScript(source);
    }
}


UserScriptController::RenderViewObserverHelper::RenderViewObserverHelper(content::RenderView *renderView)
    : content::RenderViewObserver(renderView)
{
}

void UserScriptController::RenderViewObserverHelper::DidCreateDocumentElement(blink::WebLocalFrame *frame)
{
    runScripts(UserScriptData::DocumentElementCreation, frame);
}

void UserScriptController::RenderViewObserverHelper::DidFinishDocumentLoad(blink::WebLocalFrame *frame)
{
    runScripts(UserScriptData::DocumentLoadFinished, frame);
    m_pendingFrames.insert(frame);
    base::MessageLoop::current()->PostDelayedTask(FROM_HERE, base::Bind(&UserScriptController::RenderViewObserverHelper::runScripts,
                                                                        base::Unretained(this), UserScriptData::AfterLoad, frame),
                                                  base::TimeDelta::FromMilliseconds(afterLoadTimeout));
}

void UserScriptController::RenderViewObserverHelper::DidFinishLoad(blink::WebLocalFrame *frame)
{
    // DidFinishDocumentLoad always comes before this, so frame has already been marked as pending.
    base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&UserScriptController::RenderViewObserverHelper::runScripts,
                                                                 base::Unretained(this), UserScriptData::AfterLoad, frame));
}

void UserScriptController::RenderViewObserverHelper::DidStartProvisionalLoad(blink::WebLocalFrame *frame)
{
    m_pendingFrames.remove(frame);
}

void UserScriptController::RenderViewObserverHelper::FrameDetached(blink::WebFrame *frame)
{
    if (frame->isWebLocalFrame())
        m_pendingFrames.remove(frame->toWebLocalFrame());
}

void UserScriptController::RenderViewObserverHelper::OnDestruct()
{
    UserScriptController::instance()->renderViewDestroyed(render_view());
}

bool UserScriptController::RenderViewObserverHelper::OnMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(UserScriptController::RenderViewObserverHelper, message)
        IPC_MESSAGE_HANDLER(RenderViewObserverHelper_AddScript, onUserScriptAdded)
        IPC_MESSAGE_HANDLER(RenderViewObserverHelper_RemoveScript, onUserScriptRemoved)
        IPC_MESSAGE_HANDLER(RenderViewObserverHelper_ClearScripts, onScriptsCleared)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
            return handled;
}

void UserScriptController::RenderViewObserverHelper::onUserScriptAdded(const UserScriptData &script)
{
    UserScriptController::instance()->addScriptForView(script, render_view());
}

void UserScriptController::RenderViewObserverHelper::onUserScriptRemoved(const UserScriptData &script)
{
    UserScriptController::instance()->removeScriptForView(script, render_view());
}

void UserScriptController::RenderViewObserverHelper::onScriptsCleared()
{
    UserScriptController::instance()->clearScriptsForView(render_view());
}

UserScriptController *UserScriptController::instance()
{
    return qt_webengine_userScriptController();
}

bool UserScriptController::OnControlMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(UserScriptController, message)
        IPC_MESSAGE_HANDLER(UserScriptController_AddScript, onAddScript)
        IPC_MESSAGE_HANDLER(UserScriptController_RemoveScript, onRemoveScript)
        IPC_MESSAGE_HANDLER(UserScriptController_ClearScripts, onClearScripts)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
}

UserScriptController::UserScriptController()
{
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
    static bool onlyCalledOnce = true;
    Q_ASSERT(onlyCalledOnce);
    onlyCalledOnce = false;
#endif // !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
}

void UserScriptController::renderViewCreated(content::RenderView *renderView)
{
    // Will destroy itself with their RenderView.
    new RenderViewObserverHelper(renderView);
}

void UserScriptController::renderViewDestroyed(content::RenderView *renderView)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(renderView);
    if (it == m_viewUserScriptMap.end()) // ASSERT maybe?
        return;
    Q_FOREACH (uint64 id, it.value()) {
        m_scripts.remove(id);
    }
    m_viewUserScriptMap.remove(renderView);
}

void UserScriptController::addScriptForView(const UserScriptData &script, content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        it = m_viewUserScriptMap.insert(view, UserScriptSet());

    (*it).insert(script.scriptId);
    m_scripts.insert(script.scriptId, script);
}

void UserScriptController::removeScriptForView(const UserScriptData &script, content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        return;

    (*it).remove(script.scriptId);
    m_scripts.remove(script.scriptId);
}

void UserScriptController::clearScriptsForView(content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        return;
    Q_FOREACH (uint64 id, it.value())
        m_scripts.remove(id);

    m_viewUserScriptMap.remove(view);
}

void UserScriptController::onAddScript(const UserScriptData &script)
{
    addScriptForView(script, globalScriptsIndex);
}

void UserScriptController::onRemoveScript(const UserScriptData &script)
{
    removeScriptForView(script, globalScriptsIndex);
}

void UserScriptController::onClearScripts()
{
    clearScriptsForView(globalScriptsIndex);
}

