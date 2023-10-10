// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef FAVICON_DRIVER_QT_H
#define FAVICON_DRIVER_QT_H

#include "qtwebenginecoreglobal_p.h"

#include "components/favicon/core/favicon_driver.h"
#include "components/favicon/core/favicon_handler.h"
#include "content/public/browser/document_user_data.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_handle_user_data.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "third_party/blink/public/mojom/favicon/favicon_url.mojom.h"

namespace content {
class WebContents;
}

namespace favicon {
class CoreFaviconService;
}

namespace QtWebEngineCore {

class WebContentsAdapterClient;

struct FaviconStatusQt : public content::FaviconStatus
{
    FaviconStatusQt();

    // Type of the favicon::FaviconHandler that provided this icon.
    favicon::FaviconDriverObserver::NotificationIconType source;
};

class FaviconDriverQt : public favicon::FaviconDriver,
                        public favicon::FaviconHandler::Delegate,
                        public content::WebContentsObserver,
                        public content::WebContentsUserData<FaviconDriverQt>
{
public:
    static void CreateForWebContents(content::WebContents *webContents,
                                     favicon::CoreFaviconService *faviconService,
                                     WebContentsAdapterClient *viewClient);

    // FaviconDriver implementation.
    void FetchFavicon(const GURL &page_url, bool is_same_document) override;
    gfx::Image GetFavicon() const override;
    bool FaviconIsValid() const override;
    GURL GetActiveURL() override;

    GURL GetManifestURL(content::RenderFrameHost *rfh);
    GURL GetFaviconURL() const;

protected:
    FaviconDriverQt(content::WebContents *webContent, favicon::CoreFaviconService *faviconService,
                    WebContentsAdapterClient *viewClient);

private:
    friend class content::WebContentsUserData<FaviconDriverQt>;

    // TODO(crbug.com/1205018): these two classes are current used to ensure that
    // we disregard manifest URL updates that arrive prior to onload firing.
    struct DocumentManifestData : public content::DocumentUserData<DocumentManifestData>
    {
        explicit DocumentManifestData(content::RenderFrameHost *rfh);
        ~DocumentManifestData() override;
        DOCUMENT_USER_DATA_KEY_DECL();
        bool has_manifest_url = false;
    };

    struct NavigationManifestData : public content::NavigationHandleUserData<NavigationManifestData>
    {
        explicit NavigationManifestData(content::NavigationHandle &navigation_handle);
        ~NavigationManifestData() override;
        NAVIGATION_HANDLE_USER_DATA_KEY_DECL();
        bool has_manifest_url = false;
    };

    // Callback when a manifest is downloaded.
    void OnDidDownloadManifest(ManifestDownloadCallback callback, const GURL &manifest_url,
                               blink::mojom::ManifestPtr manifest);

    // FaviconHandler::Delegate implementation.
    int DownloadImage(const GURL &url, int max_image_size, ImageDownloadCallback callback) override;
    void DownloadManifest(const GURL &url, ManifestDownloadCallback callback) override;
    bool IsOffTheRecord() override;
    void OnFaviconUpdated(const GURL &page_url,
                          favicon::FaviconDriverObserver::NotificationIconType icon_type,
                          const GURL &icon_url, bool icon_url_changed,
                          const gfx::Image &image) override;
    void OnFaviconDeleted(
            const GURL &page_url,
            favicon::FaviconDriverObserver::NotificationIconType notification_icon_type) override;
    void OnHandlerCompleted(favicon::FaviconHandler *handler) override;

    // content::WebContentsObserver implementation.
    void DidUpdateFaviconURL(content::RenderFrameHost *rfh,
                             const std::vector<blink::mojom::FaviconURLPtr> &candidates) override;
    void DidUpdateWebManifestURL(content::RenderFrameHost *rfh, const GURL &manifest_url) override;
    void DidStartNavigation(content::NavigationHandle *navigation_handle) override;
    void DidFinishNavigation(content::NavigationHandle *navigation_handle) override;

    // Informs CoreFaviconService that the favicon for |url| is out of date. If
    // |force_reload| is true, then discard information about favicon download
    // failures.
    void SetFaviconOutOfDateForPage(const GURL &url, bool force_reload);

    // Broadcasts new favicon URL candidates to FaviconHandlers.
    void OnUpdateCandidates(const GURL &page_url,
                            const std::vector<favicon::FaviconURL> &candidates,
                            const GURL &manifest_url);

    void emitIconChangedIfNeeded();

    // KeyedService used by FaviconDriverImpl. It may be null during testing,
    // but if it is defined, it must outlive the FaviconDriverImpl.
    raw_ptr<favicon::CoreFaviconService> m_faviconService;

    WebContentsAdapterClient *m_viewClient;

    // FaviconHandlers are used to download the different kind of favicons.
    std::vector<std::unique_ptr<favicon::FaviconHandler>> m_handlers;

    GURL m_bypassCachePageURL;

    // nullopt until the actual list is reported via DidUpdateFaviconURL().
    absl::optional<std::vector<blink::mojom::FaviconURLPtr>> m_faviconUrls;

    int m_completedHandlersCount = 0;
    FaviconStatusQt m_latestFavicon;

    WEB_CONTENTS_USER_DATA_KEY_DECL();
};

} // namespace QtWebEngineCore

#endif // FAVICON_DRIVER_QT_H
