/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
#include "third_party/blink/public/common/manifest/manifest.h"

namespace QtWebEngineCore {

namespace {

void ExtractManifestIcons(FaviconDriverQt::ManifestDownloadCallback callback,
                          const GURL &manifest_url, const blink::Manifest &manifest)
{
    std::vector<favicon::FaviconURL> candidates;
    for (const auto &icon : manifest.icons) {
        candidates.emplace_back(icon.src, favicon_base::IconType::kWebManifestIcon, icon.sizes);
    }
    std::move(callback).Run(candidates);
}

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
                                           favicon::FaviconService *faviconService,
                                           WebContentsAdapterClient *viewClient)
{
    if (FromWebContents(webContents))
        return;

    webContents->SetUserData(
            UserDataKey(),
            base::WrapUnique(new FaviconDriverQt(webContents, faviconService, viewClient)));
}

FaviconDriverQt::FaviconDriverQt(content::WebContents *webContents,
                                 favicon::FaviconService *faviconService,
                                 WebContentsAdapterClient *viewClient)
    : content::WebContentsObserver(webContents)
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

GURL FaviconDriverQt::GetFaviconURL() const
{
    content::NavigationController &controller = web_contents()->GetController();

    content::NavigationEntry *entry = controller.GetLastCommittedEntry();
    if (entry)
        return entry->GetFavicon().url;
    return GURL();
}

int FaviconDriverQt::DownloadImage(const GURL &url, int max_image_size,
                                   ImageDownloadCallback callback)
{
    bool bypass_cache = (m_bypassCachePageURL == GetActiveURL());
    m_bypassCachePageURL = GURL();

    return web_contents()->DownloadImage(url, true, /*preferred_size=*/max_image_size,
                                         /*max_bitmap_size=*/max_image_size, bypass_cache,
                                         std::move(callback));
}

void FaviconDriverQt::DownloadManifest(const GURL &url, ManifestDownloadCallback callback)
{
    web_contents()->GetManifest(base::BindOnce(&ExtractManifestIcons, std::move(callback)));
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
        content::RenderFrameHost *render_frame_host,
        const std::vector<blink::mojom::FaviconURLPtr> &candidates)
{
    Q_UNUSED(render_frame_host);

    // Ignore the update if there is no last committed navigation entry. This can
    // occur when loading an initially blank page.
    content::NavigationEntry *entry = web_contents()->GetController().GetLastCommittedEntry();
    if (!entry)
        return;

    // We update |m_faviconUrls| even if the list is believed to be partial
    // (checked below), because callers of our getter favicon_urls() expect so.
    std::vector<blink::mojom::FaviconURL> faviconUrls;
    for (const auto &candidate : candidates)
        faviconUrls.push_back(*candidate);
    m_faviconUrls = faviconUrls;

    if (!m_documentOnLoadCompleted)
        return;

    OnUpdateCandidates(entry->GetURL(),
                       favicon::FaviconURLsFromContentFaviconURLs(
                               m_faviconUrls.value_or(std::vector<blink::mojom::FaviconURL>())),
                       m_manifestUrl);
}

void FaviconDriverQt::DidUpdateWebManifestURL(content::RenderFrameHost *target_frame,
                                              const base::Optional<GURL> &manifest_url)
{
    Q_UNUSED(target_frame);

    // Ignore the update if there is no last committed navigation entry. This can
    // occur when loading an initially blank page.
    content::NavigationEntry *entry = web_contents()->GetController().GetLastCommittedEntry();
    if (!entry || !m_documentOnLoadCompleted)
        return;

    m_manifestUrl = manifest_url.value_or(GURL());

    // On regular page loads, DidUpdateManifestURL() is guaranteed to be called
    // before DidUpdateFaviconURL(). However, a page can update the favicons via
    // javascript.
    if (m_faviconUrls.has_value()) {
        OnUpdateCandidates(entry->GetURL(),
                           favicon::FaviconURLsFromContentFaviconURLs(*m_faviconUrls),
                           m_manifestUrl);
    }
}

void FaviconDriverQt::DidStartNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInMainFrame())
        return;

    m_faviconUrls.reset();
    m_completedHandlersCount = 0;
    m_latestFavicon = FaviconStatusQt();

    if (!navigation_handle->IsSameDocument()) {
        m_documentOnLoadCompleted = false;
        m_manifestUrl = GURL();
    }

    m_viewClient->iconChanged(QUrl());

    content::ReloadType reload_type = navigation_handle->GetReloadType();
    if (reload_type == content::ReloadType::NONE || IsOffTheRecord())
        return;

    m_bypassCachePageURL = navigation_handle->GetURL();
    SetFaviconOutOfDateForPage(navigation_handle->GetURL(),
                               reload_type == content::ReloadType::BYPASSING_CACHE);
}

void FaviconDriverQt::DidFinishNavigation(content::NavigationHandle *navigation_handle)
{
    if (!navigation_handle->IsInMainFrame() || !navigation_handle->HasCommitted()
        || navigation_handle->IsErrorPage()) {
        return;
    }

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

void FaviconDriverQt::DocumentOnLoadCompletedInMainFrame()
{
    m_documentOnLoadCompleted = true;
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

WEB_CONTENTS_USER_DATA_KEY_IMPL(FaviconDriverQt)

} // namespace QtWebEngineCore
