// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Based on components/favicon/content/content_favicon_driver.cc:
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "favicon_driver_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_engine_settings.h"

#include "components/favicon/content/favicon_url_util.h"
#include "components/favicon/core/favicon_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/page.h"
#include "content/public/browser/render_frame_host.h"
#include "third_party/blink/public/mojom/manifest/manifest.mojom.h"

namespace QtWebEngineCore {

namespace {

int activeHandlersCount(QWebEngineSettings *settings)
{
    bool touchIconsEnabled = settings->testAttribute(QWebEngineSettings::TouchIconsEnabled);
    return touchIconsEnabled ? 2 : 1;
}

} // namespace

FaviconStatusQt::FaviconStatusQt()
    : FaviconStatus(), source(favicon::FaviconDriverObserver::NON_TOUCH_16_DIP)
{
}

// static
void FaviconDriverQt::CreateForWebContents(content::WebContents *webContents,
                                           favicon::CoreFaviconService *faviconService,
                                           WebContentsAdapterClient *viewClient)
{
    if (FromWebContents(webContents))
        return;

    webContents->SetUserData(
            UserDataKey(),
            base::WrapUnique(new FaviconDriverQt(webContents, faviconService, viewClient)));
}

FaviconDriverQt::FaviconDriverQt(content::WebContents *webContents,
                                 favicon::CoreFaviconService *faviconService,
                                 WebContentsAdapterClient *viewClient)
    : content::WebContentsObserver(webContents)
    , content::WebContentsUserData<FaviconDriverQt>(*webContents)
    , m_faviconService(faviconService)
    , m_viewClient(viewClient)
{
    if (!m_faviconService)
        return;

    m_handlers.push_back(std::make_unique<favicon::FaviconHandler>(
            m_faviconService, this, favicon::FaviconDriverObserver::NON_TOUCH_16_DIP));
    m_handlers.push_back(std::make_unique<favicon::FaviconHandler>(
            m_faviconService, this, favicon::FaviconDriverObserver::NON_TOUCH_LARGEST));
    m_handlers.push_back(std::make_unique<favicon::FaviconHandler>(
            m_faviconService, this, favicon::FaviconDriverObserver::TOUCH_LARGEST));
}

void FaviconDriverQt::FetchFavicon(const GURL &page_url, bool is_same_document)
{
    QWebEngineSettings *settings = m_viewClient->webEngineSettings();
    bool iconsEnabled = settings->testAttribute(QWebEngineSettings::AutoLoadIconsForPage);
    bool touchIconsEnabled = settings->testAttribute(QWebEngineSettings::TouchIconsEnabled);

    if (!iconsEnabled)
        return;

    for (const std::unique_ptr<favicon::FaviconHandler> &handler : m_handlers) {
        switch (handler->Type()) {
        case favicon::FaviconDriverObserver::NON_TOUCH_16_DIP:
            if (touchIconsEnabled)
                continue;
            break;
        case favicon::FaviconDriverObserver::NON_TOUCH_LARGEST:
        case favicon::FaviconDriverObserver::TOUCH_LARGEST:
            if (!touchIconsEnabled)
                continue;
            break;
        }

        handler->FetchFavicon(page_url, is_same_document);
    }
}

gfx::Image FaviconDriverQt::GetFavicon() const
{
    // Like GetTitle(), we also want to use the favicon for the last committed
    // entry rather than a pending navigation entry.
    content::NavigationController &controller = web_contents()->GetController();

    content::NavigationEntry *entry = controller.GetLastCommittedEntry();
    if (entry)
        return entry->GetFavicon().image;
    return gfx::Image();
}

bool FaviconDriverQt::FaviconIsValid() const
{
    content::NavigationController &controller = web_contents()->GetController();

    content::NavigationEntry *entry = controller.GetLastCommittedEntry();
    if (entry)
        return entry->GetFavicon().valid;

    return false;
}

GURL FaviconDriverQt::GetActiveURL()
{
    content::NavigationEntry *entry = web_contents()->GetController().GetLastCommittedEntry();
    return entry ? entry->GetURL() : GURL();
}

GURL FaviconDriverQt::GetManifestURL(content::RenderFrameHost *rfh)
{
    DocumentManifestData *document_data = DocumentManifestData::GetOrCreateForCurrentDocument(rfh);
    return document_data->has_manifest_url ? rfh->GetPage().GetManifestUrl().value_or(GURL())
                                           : GURL();
}

GURL FaviconDriverQt::GetFaviconURL() const
{
    content::NavigationController &controller = web_contents()->GetController();

    content::NavigationEntry *entry = controller.GetLastCommittedEntry();
    if (entry)
        return entry->GetFavicon().url;
    return GURL();
}

FaviconDriverQt::DocumentManifestData::DocumentManifestData(content::RenderFrameHost *rfh)
    : content::DocumentUserData<DocumentManifestData>(rfh)
{
}

FaviconDriverQt::DocumentManifestData::~DocumentManifestData() = default;

FaviconDriverQt::NavigationManifestData::NavigationManifestData(
        content::NavigationHandle &navigation_handle)
{
}

FaviconDriverQt::NavigationManifestData::~NavigationManifestData() = default;

void FaviconDriverQt::OnDidDownloadManifest(ManifestDownloadCallback callback,
                                            const GURL &manifest_url,
                                            blink::mojom::ManifestPtr manifest)
{
    Q_UNUSED(manifest_url);

    // ~WebContentsImpl triggers running any pending callbacks for manifests.
    // As we're about to be destroyed ignore the request. To do otherwise may
    // result in calling back to this and attempting to use the WebContents, which
    // will crash.
    if (!web_contents())
        return;

    std::vector<favicon::FaviconURL> candidates;
    if (manifest) {
        for (const auto &icon : manifest->icons) {
            candidates.emplace_back(icon.src, favicon_base::IconType::kWebManifestIcon, icon.sizes);
        }
    }
    std::move(callback).Run(candidates);
}

int FaviconDriverQt::DownloadImage(const GURL &url, int max_image_size,
                                   ImageDownloadCallback callback)
{
    bool bypass_cache = (m_bypassCachePageURL == GetActiveURL());
    m_bypassCachePageURL = GURL();

    const gfx::Size preferred_size(max_image_size, max_image_size);
    return web_contents()->DownloadImage(url, true, preferred_size,
                                         /*max_bitmap_size=*/max_image_size,
                                         bypass_cache, std::move(callback));
}

void FaviconDriverQt::DownloadManifest(const GURL &url, ManifestDownloadCallback callback)
{
    // TODO(crbug.com/1201237): This appears to be reachable from pages other
    // than the primary page. This code should likely be refactored so that either
    // this is unreachable from other pages, or the correct page is plumbed in
    // here.
    web_contents()->GetPrimaryPage().GetManifest(base::BindOnce(
            &FaviconDriverQt::OnDidDownloadManifest, base::Unretained(this), std::move(callback)));
}

bool FaviconDriverQt::IsOffTheRecord()
{
    DCHECK(web_contents());
    return web_contents()->GetBrowserContext()->IsOffTheRecord();
}

void FaviconDriverQt::OnFaviconUpdated(
        const GURL &page_url,
        favicon::FaviconDriverObserver::NotificationIconType notification_icon_type,
        const GURL &icon_url, bool icon_url_changed, const gfx::Image &image)
{
    Q_UNUSED(page_url);

    QWebEngineSettings *settings = m_viewClient->webEngineSettings();
    bool touchIconsEnabled = settings->testAttribute(QWebEngineSettings::TouchIconsEnabled);

    // Prefer touch icons over favicons if touch icons are enabled.
    if (!touchIconsEnabled
        || m_latestFavicon.source != favicon::FaviconDriverObserver::TOUCH_LARGEST
        || notification_icon_type == favicon::FaviconDriverObserver::TOUCH_LARGEST) {
        m_latestFavicon.valid = true;
        m_latestFavicon.url = icon_url;
        m_latestFavicon.image = image;
        m_latestFavicon.source = notification_icon_type;
    }

    NotifyFaviconUpdatedObservers(notification_icon_type, icon_url, icon_url_changed, image);
    emitIconChangedIfNeeded();
}

void FaviconDriverQt::OnFaviconDeleted(
        const GURL &page_url,
        favicon::FaviconDriverObserver::NotificationIconType notification_icon_type)
{
    content::NavigationEntry *entry = web_contents()->GetController().GetLastCommittedEntry();
    DCHECK(entry && entry->GetURL() == page_url);

    entry->GetFavicon() = FaviconStatusQt();
    web_contents()->NotifyNavigationStateChanged(content::INVALIDATE_TYPE_TAB);

    NotifyFaviconUpdatedObservers(notification_icon_type, /*icon_url=*/GURL(),
                                  /*icon_url_changed=*/true, FaviconStatusQt().image);
}

void FaviconDriverQt::OnHandlerCompleted(favicon::FaviconHandler *handler)
{
    Q_UNUSED(handler);

    if (activeHandlersCount(m_viewClient->webEngineSettings()) > m_completedHandlersCount)
        ++m_completedHandlersCount;
    emitIconChangedIfNeeded();
}

void FaviconDriverQt::DidUpdateFaviconURL(
        content::RenderFrameHost *rfh, const std::vector<blink::mojom::FaviconURLPtr> &candidates)
{
    // Ignore the update if there is no last committed navigation entry. This can
    // occur when loading an initially blank page.
    content::NavigationEntry *entry = web_contents()->GetController().GetLastCommittedEntry();

    if (!entry)
        return;

    // We update |m_faviconUrls| even if the list is believed to be partial
    // (checked below), because callers of our getter favicon_urls() expect so.
    std::vector<blink::mojom::FaviconURLPtr> faviconUrls;
    for (const auto &candidate : candidates)
        faviconUrls.push_back(candidate.Clone());
    m_faviconUrls = std::move(faviconUrls);

    if (!rfh->IsDocumentOnLoadCompletedInMainFrame())
        return;

    OnUpdateCandidates(rfh->GetLastCommittedURL(),
                       favicon::FaviconURLsFromContentFaviconURLs(candidates), GetManifestURL(rfh));
}

void FaviconDriverQt::DidUpdateWebManifestURL(content::RenderFrameHost *rfh,
                                              const GURL &manifest_url)
{
    Q_UNUSED(manifest_url);

    // Ignore the update if there is no last committed navigation entry. This can
    // occur when loading an initially blank page.
    content::NavigationEntry *entry = web_contents()->GetController().GetLastCommittedEntry();
    if (!entry || !rfh->IsDocumentOnLoadCompletedInMainFrame())
        return;

    DocumentManifestData *document_data = DocumentManifestData::GetOrCreateForCurrentDocument(rfh);
    document_data->has_manifest_url = true;

    // On regular page loads, DidUpdateManifestURL() is guaranteed to be called
    // before DidUpdateFaviconURL(). However, a page can update the favicons via
    // javascript.
    if (!rfh->FaviconURLs().empty()) {
        OnUpdateCandidates(rfh->GetLastCommittedURL(),
                           favicon::FaviconURLsFromContentFaviconURLs(rfh->FaviconURLs()),
                           GetManifestURL(rfh));
    }
}

void FaviconDriverQt::DidStartNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInPrimaryMainFrame())
        return;

    if (!navigation_handle->IsSameDocument()) {
        m_completedHandlersCount = 0;
        m_latestFavicon = FaviconStatusQt();
        m_viewClient->iconChanged(QUrl());
    }

    content::ReloadType reload_type = navigation_handle->GetReloadType();
    if (reload_type == content::ReloadType::NONE || IsOffTheRecord())
        return;

    if (!navigation_handle->IsSameDocument()) {
        NavigationManifestData *navigation_data =
                NavigationManifestData::GetOrCreateForNavigationHandle(*navigation_handle);
        navigation_data->has_manifest_url = false;
    }

    if (reload_type == content::ReloadType::BYPASSING_CACHE)
        m_bypassCachePageURL = navigation_handle->GetURL();

    SetFaviconOutOfDateForPage(navigation_handle->GetURL(),
                               reload_type == content::ReloadType::BYPASSING_CACHE);
}

void FaviconDriverQt::DidFinishNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInPrimaryMainFrame() || !navigation_handle->HasCommitted()
        || navigation_handle->IsErrorPage()) {
        return;
    }

    // Transfer in-flight navigation data to the document user data.
    NavigationManifestData *navigation_data =
            NavigationManifestData::GetOrCreateForNavigationHandle(*navigation_handle);
    DocumentManifestData *document_data = DocumentManifestData::GetOrCreateForCurrentDocument(
            navigation_handle->GetRenderFrameHost());
    document_data->has_manifest_url = navigation_data->has_manifest_url;

    // Wait till the user navigates to a new URL to start checking the cache
    // again. The cache may be ignored for non-reload navigations (e.g.
    // history.replace() in-page navigation). This is allowed to increase the
    // likelihood that "reloading a page ignoring the cache" redownloads the
    // favicon. In particular, a page may do an in-page navigation before
    // FaviconHandler has the time to determine that the favicon needs to be
    // redownloaded.
    GURL url = navigation_handle->GetURL();
    if (url != m_bypassCachePageURL)
        m_bypassCachePageURL = GURL();

    // Get the favicon, either from history or request it from the net.
    FetchFavicon(url, navigation_handle->IsSameDocument());
}

void FaviconDriverQt::SetFaviconOutOfDateForPage(const GURL &url, bool force_reload)
{
    if (m_faviconService) {
        m_faviconService->SetFaviconOutOfDateForPage(url);
        if (force_reload)
            m_faviconService->ClearUnableToDownloadFavicons();
    }
}

void FaviconDriverQt::OnUpdateCandidates(const GURL &page_url,
                                         const std::vector<favicon::FaviconURL> &candidates,
                                         const GURL &manifest_url)
{
    QWebEngineSettings *settings = m_viewClient->webEngineSettings();
    bool touchIconsEnabled = settings->testAttribute(QWebEngineSettings::TouchIconsEnabled);
    for (const std::unique_ptr<favicon::FaviconHandler> &handler : m_handlers) {
        switch (handler->Type()) {
        case favicon::FaviconDriverObserver::NON_TOUCH_16_DIP:
            if (touchIconsEnabled)
                continue;
            break;
        case favicon::FaviconDriverObserver::NON_TOUCH_LARGEST:
        case favicon::FaviconDriverObserver::TOUCH_LARGEST:
            if (!touchIconsEnabled)
                continue;
            break;
        }

        // We feed in the Web Manifest URL (if any) to the instance handling type
        // kWebManifestIcon, because those compete which each other (i.e. manifest
        // icons override inline touch icons).
        handler->OnUpdateCandidates(
                page_url, candidates,
                (handler->icon_types().count(favicon_base::IconType::kWebManifestIcon) != 0)
                        ? manifest_url
                        : GURL::EmptyGURL());
    }
}

void FaviconDriverQt::emitIconChangedIfNeeded()
{
    if (activeHandlersCount(m_viewClient->webEngineSettings()) != m_completedHandlersCount)
        return;

    content::NavigationEntry *entry = web_contents()->GetController().GetLastCommittedEntry();
    DCHECK(entry);

    if (entry->GetFavicon().url != m_latestFavicon.url) {
        entry->GetFavicon().valid = m_latestFavicon.valid;
        entry->GetFavicon().url = m_latestFavicon.url;
        entry->GetFavicon().image = m_latestFavicon.image;
        web_contents()->NotifyNavigationStateChanged(content::INVALIDATE_TYPE_TAB);
    }

    m_viewClient->iconChanged(toQt(m_latestFavicon.url));
}

NAVIGATION_HANDLE_USER_DATA_KEY_IMPL(FaviconDriverQt::NavigationManifestData);
DOCUMENT_USER_DATA_KEY_IMPL(FaviconDriverQt::DocumentManifestData);
WEB_CONTENTS_USER_DATA_KEY_IMPL(FaviconDriverQt);

} // namespace QtWebEngineCore
