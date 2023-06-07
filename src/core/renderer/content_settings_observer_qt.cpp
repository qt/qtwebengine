// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Based on chrome/renderer/content_settings_observer.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content_settings_observer_qt.h"

#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "url/origin.h"

#include "common/qt_messages.h"

namespace {

bool IsUniqueFrame(blink::WebFrame *frame)
{
    return frame->GetSecurityOrigin().IsOpaque() ||
           frame->Top()->GetSecurityOrigin().IsOpaque();
}

} // namespace

namespace QtWebEngineCore {

ContentSettingsObserverQt::ContentSettingsObserverQt(content::RenderFrame *render_frame)
    : content::RenderFrameObserver(render_frame)
    , content::RenderFrameObserverTracker<ContentSettingsObserverQt>(render_frame)
    , m_currentRequestId(0)
{
    ClearBlockedContentSettings();
    render_frame->GetWebFrame()->SetContentSettingsClient(this);
}

ContentSettingsObserverQt::~ContentSettingsObserverQt() {}

bool ContentSettingsObserverQt::OnMessageReceived(const IPC::Message &message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ContentSettingsObserverQt, message)
        IPC_MESSAGE_HANDLER(QtWebEngineMsg_RequestStorageAccessAsyncResponse, OnRequestStorageAccessAsyncResponse)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()

    return handled;
}

void ContentSettingsObserverQt::DidCommitProvisionalLoad(ui::PageTransition /*transition*/)
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    if (frame->Parent())
        return; // Not a top-level navigation.

    ClearBlockedContentSettings();

    GURL url = frame->GetDocument().Url();
    // If we start failing this DCHECK, please makes sure we don't regress
    // this bug: http://code.google.com/p/chromium/issues/detail?id=79304
    DCHECK(frame->GetDocument().GetSecurityOrigin().ToString() == "null" || !url.SchemeIs(url::kDataScheme));
}

void ContentSettingsObserverQt::OnDestruct()
{
    delete this;
}

void ContentSettingsObserverQt::AllowStorageAccess(StorageType storage_type,
                                                   base::OnceCallback<void(bool)> callback)
{
    blink::WebFrame *frame = render_frame()->GetWebFrame();
    if (IsUniqueFrame(frame)) {
        std::move(callback).Run(false);
        return;
    }

    ++m_currentRequestId;
    bool inserted = m_permissionRequests.insert(std::make_pair(m_currentRequestId, std::move(callback))).second;

    // Verify there are no duplicate insertions.
    DCHECK(inserted);

    Send(new QtWebEngineHostMsg_RequestStorageAccessAsync(routing_id(), m_currentRequestId,
                                                          url::Origin(frame->GetSecurityOrigin()).GetURL(),
                                                          url::Origin(frame->Top()->GetSecurityOrigin()).GetURL(),
                                                          int(storage_type)));
}

bool ContentSettingsObserverQt::AllowStorageAccessSync(StorageType storage_type)
{
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    if (IsUniqueFrame(frame))
        return false;

    bool sameOrigin = url::Origin(frame->Top()->GetSecurityOrigin()).IsSameOriginWith(url::Origin(frame->GetSecurityOrigin()));
    StoragePermissionsKey key(url::Origin(frame->GetSecurityOrigin()).GetURL(), int(storage_type));
    if (sameOrigin) {
        const auto permissions = m_cachedStoragePermissions.find(key);
        if (permissions != m_cachedStoragePermissions.end())
            return permissions->second;
    }

    bool result = false;
    Send(new QtWebEngineHostMsg_AllowStorageAccess(routing_id(), url::Origin(frame->GetSecurityOrigin()).GetURL(),
                                                   url::Origin(frame->Top()->GetSecurityOrigin()).GetURL(),
                                                   int(storage_type), &result));
    if (sameOrigin)
        m_cachedStoragePermissions[key] = result;
    return result;
}

void ContentSettingsObserverQt::OnRequestStorageAccessAsyncResponse(int request_id, bool allowed)
{
    auto it = m_permissionRequests.find(request_id);
    if (it == m_permissionRequests.end())
        return;

    base::OnceCallback<void(bool)> callback = std::move(it->second);
    m_permissionRequests.erase(it);

    std::move(callback).Run(allowed);
}

void ContentSettingsObserverQt::ClearBlockedContentSettings()
{
    m_cachedStoragePermissions.clear();
}

} // namespace QtWebEngineCore
