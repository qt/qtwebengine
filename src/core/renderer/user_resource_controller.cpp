// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "user_resource_controller.h"

#include "base/memory/weak_ptr.h"
#include "base/strings/pattern.h"
#include "base/task/single_thread_task_runner.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "extensions/common/url_pattern.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_script_source.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

#include "qtwebengine/userscript/user_script_data.h"
#include "type_conversion.h"
#include "user_script.h"

#include <QRegularExpression>

#include <bitset>

namespace QtWebEngineCore {

static content::RenderFrame *const globalScriptsIndex = nullptr;

// Scripts meant to run after the load event will be run 500ms after DOMContentLoaded if the load event doesn't come within that delay.
static const int afterLoadTimeout = 500;

static int validUserScriptSchemes()
{
    return URLPattern::SCHEME_HTTP | URLPattern::SCHEME_HTTPS | URLPattern::SCHEME_FILE | URLPattern::SCHEME_QRC;
}

static bool regexMatchesURL(const std::string &pat, const GURL &url)
{
    QRegularExpression qre(QtWebEngineCore::toQt(pat));
    qre.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    if (!qre.isValid())
        return false;
    return qre.match(QtWebEngineCore::toQt(url.spec())).hasMatch();
}

static bool includeRuleMatchesURL(const std::string &pat, const GURL &url)
{
    // Match patterns for greasemonkey's @include and @exclude rules which can
    // be either strings with wildcards or regular expressions.
    if (pat.front() == '/' && pat.back() == '/') {
        std::string re(++pat.cbegin(), --pat.cend());
        if (regexMatchesURL(re, url))
            return true;
    } else if (base::MatchPattern(url.spec(), pat)) {
        return true;
    }
    return false;
}

static bool scriptMatchesURL(const QtWebEngineCore::UserScriptData &scriptData, const GURL &url)
{
    // Logic taken from Chromium (extensions/common/user_script.cc)
    bool matchFound;
    if (!scriptData.urlPatterns.empty()) {
        matchFound = false;
        for (auto it = scriptData.urlPatterns.begin(), end = scriptData.urlPatterns.end(); it != end; ++it) {
            URLPattern urlPattern(validUserScriptSchemes());
            if (urlPattern.Parse(*it) == URLPattern::ParseResult::kSuccess && urlPattern.MatchesURL(url))
                matchFound = true;
        }
        if (!matchFound)
            return false;
    }

    if (!scriptData.globs.empty()) {
        matchFound = false;
        for (auto it = scriptData.globs.begin(), end = scriptData.globs.end(); it != end; ++it) {
            if (includeRuleMatchesURL(*it, url))
                matchFound = true;
        }
        if (!matchFound)
            return false;
    }

    if (!scriptData.excludeGlobs.empty()) {
        for (auto it = scriptData.excludeGlobs.begin(), end = scriptData.excludeGlobs.end(); it != end; ++it) {
            if (includeRuleMatchesURL(*it, url))
                return false;
        }
    }

    return true;
}

// using UserScriptDataPtr = mojo::StructPtr<qtwebengine::mojom::UserScriptData>;

class UserResourceController::RenderFrameObserverHelper
    : public content::RenderFrameObserver,
      public qtwebengine::mojom::UserResourceControllerRenderFrame
{
public:
    RenderFrameObserverHelper(content::RenderFrame *render_frame,
                              UserResourceController *controller);
    void BindReceiver(
            mojo::PendingAssociatedReceiver<qtwebengine::mojom::UserResourceControllerRenderFrame>
                    receiver);

private:
    // RenderFrameObserver implementation.
    void DidCommitProvisionalLoad(ui::PageTransition transition) override;
    void DidDispatchDOMContentLoadedEvent() override;
    void DidFinishLoad() override;
    void WillDetach() override;
    void OnDestruct() override;
    void AddScript(const QtWebEngineCore::UserScriptData &data) override;
    void RemoveScript(const QtWebEngineCore::UserScriptData &data) override;
    void ClearScripts() override;

    class Runner;
    QScopedPointer<Runner> m_runner;
    mojo::AssociatedReceiver<qtwebengine::mojom::UserResourceControllerRenderFrame> m_binding;
    UserResourceController *m_userResourceController;
};

// Helper class to create WeakPtrs so the AfterLoad tasks can be canceled and to
// avoid running scripts more than once per injection point.
class UserResourceController::RenderFrameObserverHelper::Runner : public base::SupportsWeakPtr<Runner>
{
public:
    explicit Runner(blink::WebLocalFrame *frame, UserResourceController *controller)
        : m_frame(frame), m_userResourceController(controller)
    {
    }

    void run(QtWebEngineCore::UserScriptData::InjectionPoint p)
    {
        DCHECK_LT(p, m_ran.size());
        if (!m_ran[p]) {
            m_userResourceController->runScripts(p, m_frame);
            m_ran[p] = true;
        }
    }

private:
    blink::WebLocalFrame *m_frame;
    std::bitset<3> m_ran;
    UserResourceController *m_userResourceController;
};

void UserResourceController::runScripts(QtWebEngineCore::UserScriptData::InjectionPoint p,
                                        blink::WebLocalFrame *frame)
{
    content::RenderFrame *renderFrame = content::RenderFrame::FromWebFrame(frame);
    if (!renderFrame)
        return;
    const bool isMainFrame = renderFrame->IsMainFrame();

    QList<uint64_t> scriptsToRun = m_frameUserScriptMap.value(globalScriptsIndex);
    scriptsToRun.append(m_frameUserScriptMap.value(renderFrame));

    for (uint64_t id : std::as_const(scriptsToRun)) {
        const QtWebEngineCore::UserScriptData &script = m_scripts.value(id);
        if (script.injectionPoint != p || (!script.injectForSubframes && !isMainFrame))
            continue;
        if (!scriptMatchesURL(script, frame->GetDocument().Url()))
            continue;
        blink::WebScriptSource source(blink::WebString::FromUTF8(script.source), script.url);
        if (script.worldId)
            frame->ExecuteScriptInIsolatedWorld(script.worldId, source, blink::BackForwardCacheAware::kAllow); // FIXME, check
        else
            frame->ExecuteScript(source);
    }
}

void UserResourceController::RunScriptsAtDocumentEnd(content::RenderFrame *render_frame)
{
    runScripts(QtWebEngineCore::UserScriptData::DocumentLoadFinished, render_frame->GetWebFrame());
}

UserResourceController::RenderFrameObserverHelper::RenderFrameObserverHelper(
        content::RenderFrame *render_frame, UserResourceController *controller)
    : content::RenderFrameObserver(render_frame)
    , m_binding(this)
    , m_userResourceController(controller)
{
    render_frame->GetAssociatedInterfaceRegistry()->AddInterface<qtwebengine::mojom::UserResourceControllerRenderFrame>(
            base::BindRepeating(&UserResourceController::RenderFrameObserverHelper::BindReceiver,
                                base::Unretained(this)));
}


void UserResourceController::RenderFrameObserverHelper::BindReceiver(
        mojo::PendingAssociatedReceiver<qtwebengine::mojom::UserResourceControllerRenderFrame>
                receiver)
{
    m_binding.Bind(std::move(receiver));
}

void UserResourceController::RenderFrameObserverHelper::DidCommitProvisionalLoad(ui::PageTransition /*transition*/)
{
    // We are almost ready to run scripts. We still have to wait until the host
    // process has been notified of the DidCommitProvisionalLoad event to ensure
    // that the WebChannelTransportHost is ready to receive messages.

    m_runner.reset(new Runner(render_frame()->GetWebFrame(), m_userResourceController));

    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
            FROM_HERE,
            base::BindOnce(&Runner::run, m_runner->AsWeakPtr(),
                           QtWebEngineCore::UserScriptData::DocumentElementCreation));
}

void UserResourceController::RenderFrameObserverHelper::DidDispatchDOMContentLoadedEvent()
{
    // Don't run scripts if provisional load failed (DidFailProvisionalLoad
    // called instead of DidCommitProvisionalLoad).
    if (m_runner)
        base::SingleThreadTaskRunner::GetCurrentDefault()->PostDelayedTask(
                FROM_HERE,
                base::BindOnce(&Runner::run, m_runner->AsWeakPtr(),
                               QtWebEngineCore::UserScriptData::AfterLoad),
                base::Milliseconds(afterLoadTimeout));
}

void UserResourceController::RenderFrameObserverHelper::DidFinishLoad()
{
    if (m_runner)
        base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
                FROM_HERE,
                base::BindOnce(&Runner::run, m_runner->AsWeakPtr(),
                               QtWebEngineCore::UserScriptData::AfterLoad));
}

void UserResourceController::RenderFrameObserverHelper::WillDetach()
{
    m_runner.reset();
}

void UserResourceController::RenderFrameObserverHelper::OnDestruct()
{
    if (content::RenderFrame *frame = render_frame()) {
        m_userResourceController->renderFrameDestroyed(frame);
    }
    delete this;
}

void UserResourceController::RenderFrameObserverHelper::AddScript(
        const QtWebEngineCore::UserScriptData &script)
{
    if (content::RenderFrame *frame = render_frame())
        m_userResourceController->addScriptForFrame(script, frame);
}

void UserResourceController::RenderFrameObserverHelper::RemoveScript(
        const QtWebEngineCore::UserScriptData &script)
{
    if (content::RenderFrame *frame = render_frame())
        m_userResourceController->removeScriptForFrame(script, frame);
}

void UserResourceController::RenderFrameObserverHelper::ClearScripts()
{
    if (content::RenderFrame *frame = render_frame())
        m_userResourceController->clearScriptsForFrame(frame);
}

void UserResourceController::BindReceiver(
        mojo::PendingAssociatedReceiver<qtwebengine::mojom::UserResourceController> receiver)
{
    m_binding.Bind(std::move(receiver));
}

UserResourceController::UserResourceController() : m_binding(this)
{
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
    static bool onlyCalledOnce = true;
    DCHECK(onlyCalledOnce);
    onlyCalledOnce = false;
#endif // !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
}

void UserResourceController::renderFrameCreated(content::RenderFrame *renderFrame)
{
    // Will destroy itself when the RenderFrame is destroyed.
    new RenderFrameObserverHelper(renderFrame, this);
}

void UserResourceController::renderFrameDestroyed(content::RenderFrame *renderFrame)
{
    FrameUserScriptMap::iterator it = m_frameUserScriptMap.find(renderFrame);
    if (it == m_frameUserScriptMap.end()) // ASSERT maybe?
        return;
    if (renderFrame->IsMainFrame()) {
        for (uint64_t id : std::as_const(it.value()))
            m_scripts.remove(id);
    }
    m_frameUserScriptMap.erase(it);
}

void UserResourceController::addScriptForFrame(const QtWebEngineCore::UserScriptData &script,
                                               content::RenderFrame *frame)
{
    FrameUserScriptMap::iterator it = m_frameUserScriptMap.find(frame);
    if (it == m_frameUserScriptMap.end())
        it = m_frameUserScriptMap.insert(frame, UserScriptList());

    if (!(*it).contains(script.scriptId))
        (*it).append(script.scriptId);
    if (!frame || frame->IsMainFrame())
        m_scripts.insert(script.scriptId, script);
}

void UserResourceController::removeScriptForFrame(const QtWebEngineCore::UserScriptData &script,
                                                  content::RenderFrame *frame)
{
    FrameUserScriptMap::iterator it = m_frameUserScriptMap.find(frame);
    if (it == m_frameUserScriptMap.end())
        return;

    (*it).removeOne(script.scriptId);
    if (!frame || frame->IsMainFrame())
        m_scripts.remove(script.scriptId);
}

void UserResourceController::clearScriptsForFrame(content::RenderFrame *frame)
{
    FrameUserScriptMap::iterator it = m_frameUserScriptMap.find(frame);
    if (it == m_frameUserScriptMap.end())
        return;
    if (!frame || frame->IsMainFrame()) {
        for (uint64_t id : std::as_const(it.value()))
            m_scripts.remove(id);
    }

    m_frameUserScriptMap.remove(frame);
}

void UserResourceController::AddScript(const QtWebEngineCore::UserScriptData &script)
{
    addScriptForFrame(script, globalScriptsIndex);
}

void UserResourceController::RemoveScript(const QtWebEngineCore::UserScriptData &script)
{
    removeScriptForFrame(script, globalScriptsIndex);
}

void UserResourceController::ClearScripts()
{
    clearScriptsForFrame(globalScriptsIndex);
}

void UserResourceController::RegisterMojoInterfaces(
        blink::AssociatedInterfaceRegistry *associated_interfaces)
{
    associated_interfaces->AddInterface<qtwebengine::mojom::UserResourceController>(
            base::BindRepeating(&UserResourceController::BindReceiver, base::Unretained(this)));
}

void UserResourceController::UnregisterMojoInterfaces(
        blink::AssociatedInterfaceRegistry *associated_interfaces)
{
    associated_interfaces->RemoveInterface(qtwebengine::mojom::UserResourceController::Name_);
}

} // namespace
