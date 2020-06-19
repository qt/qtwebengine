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
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "third_party/blink/public/mojom/favicon/favicon_url.mojom.h"

namespace content {
class WebContents;
}

namespace favicon {
class FaviconService;
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
                                     favicon::FaviconService *faviconService,
                                     WebContentsAdapterClient *viewClient);

    // FaviconDriver implementation.
    void FetchFavicon(const GURL &page_url, bool is_same_document) override;
    gfx::Image GetFavicon() const override;
    bool FaviconIsValid() const override;
    GURL GetActiveURL() override;

    GURL GetFaviconURL() const;

protected:
    FaviconDriverQt(content::WebContents *webContent, favicon::FaviconService *faviconService,
                    WebContentsAdapterClient *viewClient);

private:
    friend class content::WebContentsUserData<FaviconDriverQt>;

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
    void DidUpdateFaviconURL(content::RenderFrameHost *render_frame_host,
                             const std::vector<blink::mojom::FaviconURLPtr> &candidates) override;
    void DidUpdateWebManifestURL(content::RenderFrameHost *target_frame,
                                 const base::Optional<GURL> &manifest_url) override;
    void DidStartNavigation(content::NavigationHandle *navigation_handle) override;
    void DidFinishNavigation(content::NavigationHandle *navigation_handle) override;
    void DocumentOnLoadCompletedInMainFrame() override;

    // Informs FaviconService that the favicon for |url| is out of date. If
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
    favicon::FaviconService *m_faviconService;

    WebContentsAdapterClient *m_viewClient;

    // FaviconHandlers used to download the different kind of favicons.
    std::vector<std::unique_ptr<favicon::FaviconHandler>> m_handlers;

    GURL m_bypassCachePageURL;
    bool m_documentOnLoadCompleted = false;

    // nullopt until the actual list is reported via DidUpdateFaviconURL().
    base::Optional<std::vector<blink::mojom::FaviconURL>> m_faviconUrls;
    // Web Manifest URL or empty URL if none.
    GURL m_manifestUrl;

    int m_completedHandlersCount = 0;
    FaviconStatusQt m_latestFavicon;

    WEB_CONTENTS_USER_DATA_KEY_DECL();
    DISALLOW_COPY_AND_ASSIGN(FaviconDriverQt);
};

} // namespace QtWebEngineCore

#endif // FAVICON_DRIVER_QT_H
