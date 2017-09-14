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

#include "user_resource_controller.h"

#include "base/memory/weak_ptr.h"
#include "base/pending_task.h"
#include "base/strings/pattern.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_view_observer.h"
#include "extensions/common/url_pattern.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

#include "common/qt_messages.h"
#include "common/user_script_data.h"
#include "type_conversion.h"
#include "user_script.h"

Q_GLOBAL_STATIC(UserResourceController, qt_webengine_userResourceController)

static content::RenderView * const globalScriptsIndex = 0;

// Scripts meant to run after the load event will be run 500ms after DOMContentLoaded if the load event doesn't come within that delay.
static const int afterLoadTimeout = 500;

static bool scriptMatchesURL(const UserScriptData &scriptData, const GURL &url) {
    // Logic taken from Chromium (extensions/common/user_script.cc)
    bool matchFound;
    if (!scriptData.urlPatterns.empty()) {
        matchFound = false;
        for (auto it = scriptData.urlPatterns.begin(), end = scriptData.urlPatterns.end(); it != end; ++it) {
            URLPattern urlPattern(QtWebEngineCore::UserScript::validUserScriptSchemes(), *it);
            if (urlPattern.MatchesURL(url))
                matchFound = true;
        }
        if (!matchFound)
            return false;
    }

    if (!scriptData.globs.empty()) {
        matchFound = false;
        for (auto it = scriptData.globs.begin(), end = scriptData.globs.end(); it != end; ++it) {
            if (base::MatchPattern(url.spec(), *it))
                matchFound = true;
        }
        if (!matchFound)
            return false;
    }

    if (!scriptData.excludeGlobs.empty()) {
        for (auto it = scriptData.excludeGlobs.begin(), end = scriptData.excludeGlobs.end(); it != end; ++it) {
            if (base::MatchPattern(url.spec(), *it))
                return false;
        }
    }

    return true;
}

class UserResourceController::RenderFrameObserverHelper : public content::RenderFrameObserver
{
public:
    RenderFrameObserverHelper(content::RenderFrame* render_frame);

private:
    ~RenderFrameObserverHelper() override;

    // RenderFrameObserver implementation.
    void DidFinishDocumentLoad() override;
    void DidFinishLoad() override;
    void DidStartProvisionalLoad(blink::WebDataSource* data_source) override;
    void FrameDetached() override;
    void OnDestruct() override;
    bool OnMessageReceived(const IPC::Message& message) override;

    void onUserScriptAdded(const UserScriptData &);
    void onUserScriptRemoved(const UserScriptData &);
    void onScriptsCleared();

    void runScripts(UserScriptData::InjectionPoint, blink::WebLocalFrame *);

    // Set of frames which are pending to get an AfterLoad invocation of runScripts, if they
    // haven't gotten it already.
    QSet<blink::WebLocalFrame *> m_pendingFrames;
    base::WeakPtrFactory<RenderFrameObserverHelper> m_weakPtrFactory;
};

// Used only for script cleanup on RenderView destruction.
class UserResourceController::RenderViewObserverHelper : public content::RenderViewObserver
{
public:
    RenderViewObserverHelper(content::RenderView* render_view);
private:
    // RenderViewObserver implementation.
    void OnDestruct() override;
};

void UserResourceController::RenderFrameObserverHelper::runScripts(UserScriptData::InjectionPoint p, blink::WebLocalFrame *frame)
{
    if (p == UserScriptData::AfterLoad && !m_pendingFrames.remove(frame))
        return;

    UserResourceController::instance()->runScripts(p, frame);
}

void UserResourceController::runScripts(UserScriptData::InjectionPoint p, blink::WebLocalFrame *frame)
{
    content::RenderFrame *renderFrame = content::RenderFrame::FromWebFrame(frame);
    content::RenderView *renderView = renderFrame->GetRenderView();
    const bool isMainFrame = renderFrame->IsMainFrame();

    QList<uint64_t> scriptsToRun = m_viewUserScriptMap.value(0).toList();
    scriptsToRun.append(m_viewUserScriptMap.value(renderView).toList());

    Q_FOREACH (uint64_t id, scriptsToRun) {
        const UserScriptData &script = m_scripts.value(id);
        if (script.injectionPoint != p
                || (!script.injectForSubframes && !isMainFrame))
            continue;
        if (!scriptMatchesURL(script, frame->GetDocument().Url()))
            continue;
        blink::WebScriptSource source(blink::WebString::FromUTF8(script.source), script.url);
        if (script.worldId)
            frame->ExecuteScriptInIsolatedWorld(script.worldId, &source, /*numSources = */1, /*contentScriptExtentsionGroup = */ 0);
        else
            frame->ExecuteScript(source);
    }
}

void UserResourceController::RunScriptsAtDocumentStart(content::RenderFrame *render_frame)
{
    runScripts(UserScriptData::DocumentElementCreation, render_frame->GetWebFrame());
}

void UserResourceController::RunScriptsAtDocumentEnd(content::RenderFrame *render_frame)
{
    runScripts(UserScriptData::DocumentLoadFinished, render_frame->GetWebFrame());
}

UserResourceController::RenderFrameObserverHelper::RenderFrameObserverHelper(content::RenderFrame *render_frame)
    : content::RenderFrameObserver(render_frame), m_weakPtrFactory(this)
{
}

UserResourceController::RenderFrameObserverHelper::~RenderFrameObserverHelper()
{
    m_weakPtrFactory.InvalidateWeakPtrs();
}

UserResourceController::RenderViewObserverHelper::RenderViewObserverHelper(content::RenderView *render_view)
    : content::RenderViewObserver(render_view)
{
}

void UserResourceController::RenderFrameObserverHelper::DidFinishDocumentLoad()
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    m_pendingFrames.insert(frame);
    base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(FROM_HERE, base::Bind(&UserResourceController::RenderFrameObserverHelper::runScripts,
                                                                               m_weakPtrFactory.GetWeakPtr(), UserScriptData::AfterLoad, frame),
                                                         base::TimeDelta::FromMilliseconds(afterLoadTimeout));
}

void UserResourceController::RenderFrameObserverHelper::DidFinishLoad()
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();

    // DidFinishDocumentLoad always comes before this, so frame has already been marked as pending.
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, base::Bind(&UserResourceController::RenderFrameObserverHelper::runScripts,
                                                                        m_weakPtrFactory.GetWeakPtr(), UserScriptData::AfterLoad, frame));
}

void UserResourceController::RenderFrameObserverHelper::DidStartProvisionalLoad(blink::WebDataSource *data_source)
{
    Q_UNUSED(data_source);
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    m_pendingFrames.remove(frame);
}

void UserResourceController::RenderFrameObserverHelper::FrameDetached()
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    m_pendingFrames.remove(frame);
}

void UserResourceController::RenderFrameObserverHelper::OnDestruct()
{
    delete this;
}

void UserResourceController::RenderViewObserverHelper::OnDestruct()
{
    // Remove all scripts associated with the render view.
    UserResourceController::instance()->renderViewDestroyed(render_view());
    delete this;
}

bool UserResourceController::RenderFrameObserverHelper::OnMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(UserResourceController::RenderFrameObserverHelper, message)
        IPC_MESSAGE_HANDLER(RenderFrameObserverHelper_AddScript, onUserScriptAdded)
        IPC_MESSAGE_HANDLER(RenderFrameObserverHelper_RemoveScript, onUserScriptRemoved)
        IPC_MESSAGE_HANDLER(RenderFrameObserverHelper_ClearScripts, onScriptsCleared)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
            return handled;
}

void UserResourceController::RenderFrameObserverHelper::onUserScriptAdded(const UserScriptData &script)
{
    content::RenderView *view = render_frame()->GetRenderView();
    UserResourceController::instance()->addScriptForView(script, view);
}

void UserResourceController::RenderFrameObserverHelper::onUserScriptRemoved(const UserScriptData &script)
{
    content::RenderView *view = render_frame()->GetRenderView();
    UserResourceController::instance()->removeScriptForView(script, view);
}

void UserResourceController::RenderFrameObserverHelper::onScriptsCleared()
{
    content::RenderView *view = render_frame()->GetRenderView();
    UserResourceController::instance()->clearScriptsForView(view);
}

UserResourceController *UserResourceController::instance()
{
    return qt_webengine_userResourceController();
}

bool UserResourceController::OnControlMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(UserResourceController, message)
        IPC_MESSAGE_HANDLER(UserResourceController_AddScript, onAddScript)
        IPC_MESSAGE_HANDLER(UserResourceController_RemoveScript, onRemoveScript)
        IPC_MESSAGE_HANDLER(UserResourceController_ClearScripts, onClearScripts)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
}

UserResourceController::UserResourceController()
{
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
    static bool onlyCalledOnce = true;
    Q_ASSERT(onlyCalledOnce);
    onlyCalledOnce = false;
#endif // !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
}

void UserResourceController::renderFrameCreated(content::RenderFrame *renderFrame)
{
    // Will destroy itself when the RenderFrame is destroyed.
    new RenderFrameObserverHelper(renderFrame);
}

void UserResourceController::renderViewCreated(content::RenderView *renderView)
{
    // Will destroy itself when the RenderView is destroyed.
    new RenderViewObserverHelper(renderView);
}

void UserResourceController::renderViewDestroyed(content::RenderView *renderView)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(renderView);
    if (it == m_viewUserScriptMap.end()) // ASSERT maybe?
        return;
    Q_FOREACH (uint64_t id, it.value()) {
        m_scripts.remove(id);
    }
    m_viewUserScriptMap.remove(renderView);
}

void UserResourceController::addScriptForView(const UserScriptData &script, content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        it = m_viewUserScriptMap.insert(view, UserScriptSet());

    (*it).insert(script.scriptId);
    m_scripts.insert(script.scriptId, script);
}

void UserResourceController::removeScriptForView(const UserScriptData &script, content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        return;

    (*it).remove(script.scriptId);
    m_scripts.remove(script.scriptId);
}

void UserResourceController::clearScriptsForView(content::RenderView *view)
{
    ViewUserScriptMap::iterator it = m_viewUserScriptMap.find(view);
    if (it == m_viewUserScriptMap.end())
        return;
    Q_FOREACH (uint64_t id, it.value())
        m_scripts.remove(id);

    m_viewUserScriptMap.remove(view);
}

void UserResourceController::onAddScript(const UserScriptData &script)
{
    addScriptForView(script, globalScriptsIndex);
}

void UserResourceController::onRemoveScript(const UserScriptData &script)
{
    removeScriptForView(script, globalScriptsIndex);
}

void UserResourceController::onClearScripts()
{
    clearScriptsForView(globalScriptsIndex);
}

