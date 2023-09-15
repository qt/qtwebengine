// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on content/shell/browser/shell_devtools_frontend.cc:
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "devtools_frontend_qt.h"

#include "profile_qt.h"
#include "profile_adapter.h"
#include "web_contents_adapter.h"
#include "web_contents_delegate_qt.h"

#include "chrome/browser/devtools/devtools_eye_dropper.h"
#include "chrome/browser/devtools/devtools_ui_bindings.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/site_instance.h"
#include "url/gurl.h"

using namespace QtWebEngineCore;

namespace {
static const char kScreencastEnabled[] = "screencastEnabled";

static std::string GetFrontendURL()
{
    return "devtools://devtools/bundled/inspector.html";
}
} // namespace

namespace QtWebEngineCore {

// static
DevToolsFrontendQt *DevToolsFrontendQt::Show(QSharedPointer<WebContentsAdapter> frontendAdapter,
                                             content::WebContents *inspectedContents)
{
    DCHECK(frontendAdapter);
    DCHECK(inspectedContents);

    if (!frontendAdapter->isInitialized()) {
        scoped_refptr<content::SiteInstance> site = content::SiteInstance::CreateForURL(
                frontendAdapter->profile(), GURL(GetFrontendURL()));
        frontendAdapter->initialize(site.get());
    }

    frontendAdapter->setInspector(true);

    content::WebContents *contents = frontendAdapter->webContents();
    if (contents == inspectedContents) {
        LOG(WARNING) << "You can not inspect yourself";
        return nullptr;
    }

    DevToolsFrontendQt *devtoolsFrontend =
            new DevToolsFrontendQt(frontendAdapter, inspectedContents);

    if (contents->GetURL() == GURL(GetFrontendURL())) {
        contents->GetController().LoadOriginalRequestURL();
    } else {
        content::NavigationController::LoadURLParams loadParams((GURL(GetFrontendURL())));
        loadParams.transition_type = ui::PageTransitionFromInt(ui::PAGE_TRANSITION_AUTO_TOPLEVEL
                                                               | ui::PAGE_TRANSITION_FROM_API);
        contents->GetController().LoadURLWithParams(loadParams);
    }

    return devtoolsFrontend;
}

DevToolsFrontendQt::DevToolsFrontendQt(QSharedPointer<WebContentsAdapter> webContentsAdapter,
                                       content::WebContents *inspectedContents)
    : content::WebContentsObserver(webContentsAdapter->webContents())
    , m_frontendAdapter(webContentsAdapter)
    , m_inspectedContents(inspectedContents)
    , m_bindings(new DevToolsUIBindings(webContentsAdapter->webContents()))
{
    // bindings take ownership over devtools
    m_bindings->SetDelegate(this);
    m_bindings->AttachTo(content::DevToolsAgentHost::GetOrCreateFor(m_inspectedContents));

    auto *prefService = m_bindings->profile()->GetPrefs();
    const auto &devtoolsPrefs = prefService->GetDict(prefs::kDevToolsPreferences);

    if (!devtoolsPrefs.Find(kScreencastEnabled)) {
        ScopedDictPrefUpdate update(prefService, prefs::kDevToolsPreferences);
        update->Set(kScreencastEnabled, "false");
    }
}

DevToolsFrontendQt::~DevToolsFrontendQt()
{
    if (QSharedPointer<WebContentsAdapter> p = m_frontendAdapter)
        p->setInspector(false);
}

void DevToolsFrontendQt::Activate()
{
    web_contents()->GetDelegate()->ActivateContents(web_contents());
}

void DevToolsFrontendQt::Focus()
{
    web_contents()->Focus();
}

void DevToolsFrontendQt::InspectElementAt(int x, int y)
{
    if (!m_inspectedContents)
        return;
    scoped_refptr<content::DevToolsAgentHost> agent(
            content::DevToolsAgentHost::GetOrCreateFor(m_inspectedContents));
    agent->InspectElement(m_inspectedContents->GetFocusedFrame(), x, y);
}

void DevToolsFrontendQt::Close()
{
    // Don't close the webContents, it might be reused, but pretend it was
    WebContentsDestroyed();
}

void DevToolsFrontendQt::DisconnectFromTarget()
{
    m_bindings->Detach();
}

WebContentsDelegateQt *DevToolsFrontendQt::frontendDelegate() const
{
    return static_cast<WebContentsDelegateQt *>(web_contents()->GetDelegate());
}

void DevToolsFrontendQt::ColorPickedInEyeDropper(int r, int g, int b, int a)
{
    base::Value::Dict color;
    color.Set("r", r);
    color.Set("g", g);
    color.Set("b", b);
    color.Set("a", a);
    m_bindings->CallClientMethod("DevToolsAPI", "eyeDropperPickedColor", base::Value(std::move(color)));
}

// content::WebContentsObserver implementation
void DevToolsFrontendQt::WebContentsDestroyed()
{
    WebContentsAdapter *inspectedAdapter =
            static_cast<WebContentsDelegateQt *>(m_inspectedContents->GetDelegate())
                    ->webContentsAdapter();
    if (inspectedAdapter)
        inspectedAdapter->devToolsFrontendDestroyed(this);

    delete m_bindings; // it will call ~DevToolsFrontendQt()
}

// DevToolsUIBindings::Delegate implementation
void DevToolsFrontendQt::ActivateWindow()
{
    web_contents()->Focus();
}

void DevToolsFrontendQt::OnLoadCompleted()
{
    m_bindings->CallClientMethod("DevToolsAPI", "setUseSoftMenu", base::Value(true));
}

void DevToolsFrontendQt::OpenInNewTab(const std::string &url)
{
    content::OpenURLParams params(GURL(url), content::Referrer(),
                                  WindowOpenDisposition::NEW_FOREGROUND_TAB,
                                  ui::PAGE_TRANSITION_LINK, false);

    m_inspectedContents->OpenURL(params);
}

void DevToolsFrontendQt::SetEyeDropperActive(bool active)
{
    if (!m_inspectedContents)
        return;
    if (active) {
        m_eyeDropper.reset(new DevToolsEyeDropper(
                m_inspectedContents,
                base::BindRepeating(&DevToolsFrontendQt::ColorPickedInEyeDropper,
                                    base::Unretained(this))));
    } else {
        m_eyeDropper.reset();
    }
}

// static
bool DevToolsFrontendQt::IsValidFrontendURL(const GURL &url)
{
    // NOTE: the inspector app does not change the frontend url.
    // If we bring back the devtools_app, the url must be sanitized
    // according to chrome/browser/devtools/devtools_ui_bindings.cc.
    return url.spec() == GetFrontendURL();
}

void DevToolsFrontendQt::InspectedContentsClosing()
{
    web_contents()->ClosePage();
}

std::string DevToolsFrontendQt::GetId(content::WebContents *inspectedContents)
{
    return content::DevToolsAgentHost::GetOrCreateFor(inspectedContents)->GetId();
}

void DevToolsFrontendQt::CloseWindow()
{
    web_contents()->Close();
}

} // namespace QtWebEngineCore
