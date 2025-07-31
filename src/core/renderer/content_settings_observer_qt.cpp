// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

// Based on chrome/renderer/content_settings_observer.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content_settings_observer_qt.h"

#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/platform/browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "url/origin.h"

namespace {

bool IsUniqueFrame(blink::WebFrame *frame)
{
    return frame->GetSecurityOrigin().IsOpaque() ||
           frame->Top()->GetSecurityOrigin().IsOpaque();
}

} // namespace

namespace QtWebEngineCore {

using ContentSettingsManager = content_settings::mojom::ContentSettingsManager;
ContentSettingsManager::StorageType ConvertToMojoStorageType(
        ContentSettingsObserverQt::StorageType storage_type)
{
    switch (storage_type) {
    case ContentSettingsObserverQt::StorageType::kDatabase:
        return ContentSettingsManager::StorageType::DATABASE;
    case ContentSettingsObserverQt::StorageType::kIndexedDB:
        return ContentSettingsManager::StorageType::INDEXED_DB;
    case ContentSettingsObserverQt::StorageType::kCacheStorage:
        return ContentSettingsManager::StorageType::CACHE;
    case ContentSettingsObserverQt::StorageType::kWebLocks:
        return ContentSettingsManager::StorageType::WEB_LOCKS;
    case ContentSettingsObserverQt::StorageType::kFileSystem:
        return ContentSettingsManager::StorageType::FILE_SYSTEM;
    case ContentSettingsObserverQt::StorageType::kLocalStorage:
        return ContentSettingsManager::StorageType::LOCAL_STORAGE;
    case ContentSettingsObserverQt::StorageType::kSessionStorage:
        return ContentSettingsManager::StorageType::SESSION_STORAGE;
    }
}

ContentSettingsObserverQt::ContentSettingsObserverQt(content::RenderFrame *render_frame)
    : content::RenderFrameObserver(render_frame)
    , content::RenderFrameObserverTracker<ContentSettingsObserverQt>(render_frame)
    , m_currentRequestId(0)
{
    ClearBlockedContentSettings();
    render_frame->GetWebFrame()->SetContentSettingsClient(this);
}

ContentSettingsObserverQt::~ContentSettingsObserverQt() {}

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
    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    if (IsUniqueFrame(frame)) {
        std::move(callback).Run(false);
        return;
    }

    base::OnceCallback<void(bool)> allowStorageCallBack =
            base::BindOnce([](base::OnceCallback<void(bool)> original_cb,
                              bool result) { std::move(original_cb).Run(result); },
                           std::move(callback));

    GetContentSettingsManager()->AllowStorageAccess(
            frame->GetLocalFrameToken(), ConvertToMojoStorageType(storage_type),
            frame->GetSecurityOrigin(), frame->GetDocument().SiteForCookies(),
            frame->GetDocument().TopFrameOrigin(), std::move(allowStorageCallBack));
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
    GetContentSettingsManager()->AllowStorageAccess(
            frame->GetLocalFrameToken(), ConvertToMojoStorageType(storage_type),
            frame->GetSecurityOrigin(), frame->GetDocument().SiteForCookies(),
            frame->GetDocument().TopFrameOrigin(), &result);
    if (sameOrigin)
        m_cachedStoragePermissions[key] = result;
    return result;
}

mojo::Remote<ContentSettingsManager> &ContentSettingsObserverQt::GetContentSettingsManager()
{
    if (!m_contentSettingsManager) {
        render_frame()->GetBrowserInterfaceBroker().GetInterface(
                m_contentSettingsManager.BindNewPipeAndPassReceiver());
    }
    return m_contentSettingsManager;
}

void ContentSettingsObserverQt::ClearBlockedContentSettings()
{
    m_cachedStoragePermissions.clear();
}

} // namespace QtWebEngineCore
