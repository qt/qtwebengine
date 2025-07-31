// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SETTINGS_OBSERVER_QT_H
#define CONTENT_SETTINGS_OBSERVER_QT_H

#include "base/containers/flat_map.h"
#include "content/common/buildflags.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "third_party/blink/public/platform/web_content_settings_client.h"
#include "url/gurl.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "components/content_settings/common/content_settings_manager.mojom.h"

namespace QtWebEngineCore {

// Handles blocking content per content settings for each RenderFrame.
class ContentSettingsObserverQt
    : public content::RenderFrameObserver
    , public content::RenderFrameObserverTracker<ContentSettingsObserverQt>
    , public blink::WebContentSettingsClient
{
public:
    ContentSettingsObserverQt(content::RenderFrame *render_frame);
    ~ContentSettingsObserverQt() override;

    // blink::WebContentSettingsClient:
    void AllowStorageAccess(StorageType storage_type,
                            base::OnceCallback<void(bool)> callback) override;
    bool AllowStorageAccessSync(StorageType storage_type) override;

private:
    // RenderFrameObserver implementation:
    void DidCommitProvisionalLoad(ui::PageTransition transition) override;
    void OnDestruct() override;

    // Message handlers.
    void OnRequestStorageAccessAsyncResponse(int request_id, bool allowed);

    // Clears m_cachedStoragePermissions
    void ClearBlockedContentSettings();

    mojo::Remote<content_settings::mojom::ContentSettingsManager> &GetContentSettingsManager();
    mojo::Remote<content_settings::mojom::ContentSettingsManager> m_contentSettingsManager;

    // Caches the result of AllowStorage.
    using StoragePermissionsKey = std::pair<GURL, int>;
    base::flat_map<StoragePermissionsKey, bool> m_cachedStoragePermissions;

    int m_currentRequestId;
    base::flat_map<int, base::OnceCallback<void(bool)>> m_permissionRequests;
};

} // namespace QtWebEngineCore

#endif // CONTENT_SETTINGS_OBSERVER_QT_H
