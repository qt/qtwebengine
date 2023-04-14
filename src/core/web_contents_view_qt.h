// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WEB_CONTENTS_VIEW_QT_H
#define WEB_CONTENTS_VIEW_QT_H

#include "content/browser/renderer_host/render_view_host_delegate_view.h"
#include "content/browser/web_contents/web_contents_view.h"

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

QT_FORWARD_DECLARE_CLASS(QWebEngineContextMenuRequest)

namespace extensions {
class MimeHandlerViewGuestDelegateQt;
}

namespace content {
class WebContents;
}

namespace QtWebEngineCore {
class WebContentsAdapterClient;

class WebContentsViewQt
    : public content::WebContentsView
    , public content::RenderViewHostDelegateView
{
public:
    static inline WebContentsViewQt *from(WebContentsView *view) { return static_cast<WebContentsViewQt*>(view); }

    WebContentsViewQt(content::WebContents* webContents);

    void setFactoryClient(WebContentsAdapterClient* client);
    void setClient(WebContentsAdapterClient* client);
    WebContentsAdapterClient *client() { return m_client; }

    // content::WebContentsView overrides:
    content::RenderWidgetHostViewBase *CreateViewForWidget(content::RenderWidgetHost *render_widget_host) override;

    void CreateView(gfx::NativeView context) override;

    content::RenderWidgetHostViewBase *CreateViewForChildWidget(content::RenderWidgetHost* render_widget_host) override;

    void SetPageTitle(const std::u16string& title) override { }

    void RenderViewReady() override { }

    void RenderViewHostChanged(content::RenderViewHost*, content::RenderViewHost*) override { }

    void SetOverscrollControllerEnabled(bool enabled) override { }

    gfx::NativeView GetNativeView() const override;

    gfx::NativeView GetContentNativeView() const override { return nullptr; }

    gfx::NativeWindow GetTopLevelNativeWindow() const override { return nullptr; }

    gfx::Rect GetContainerBounds() const override;

    void Focus() override;

    void SetInitialFocus() override;

    void StoreFocus() override { }

    void RestoreFocus() override { }

    content::DropData* GetDropData() const override { QT_NOT_YET_IMPLEMENTED return nullptr; }

    gfx::Rect GetViewBounds() const override { return gfx::Rect(); }

    void FocusThroughTabTraversal(bool reverse) override;
    void OnCapturerCountChanged() override { QT_NOT_YET_IMPLEMENTED }
    void FullscreenStateChanged(bool) override { }
    void UpdateWindowControlsOverlay(const gfx::Rect &) override { QT_NOT_YET_IMPLEMENTED }

#if BUILDFLAG(IS_MAC)
    bool CloseTabAfterEventTrackingIfNeeded() override { QT_NOT_YET_IMPLEMENTED return false; }
#endif

    // content::RenderViewHostDelegateView overrides:
    void StartDragging(const content::DropData& drop_data, blink::DragOperationsMask allowed_ops,
                       const gfx::ImageSkia& image, const gfx::Vector2d& image_offset,
                       const gfx::Rect& drag_obj_rect,
                       const blink::mojom::DragEventSourceInfo &event_info,
                       content::RenderWidgetHostImpl *source_rwh) override;

    void UpdateDragCursor(ui::mojom::DragOperation dragOperation) override;

    void ShowContextMenu(content::RenderFrameHost &, const content::ContextMenuParams &params) override;

    void GotFocus(content::RenderWidgetHostImpl *render_widget_host) override;
    void LostFocus(content::RenderWidgetHostImpl *render_widget_host) override;
    void TakeFocus(bool reverse) override;

private:
    static void update(QWebEngineContextMenuRequest *request,
                       const content::ContextMenuParams &params, bool spellcheckEnabled);

private:
    content::WebContents *m_webContents;
    WebContentsAdapterClient *m_client;
    WebContentsAdapterClient *m_factoryClient;
    std::unique_ptr<QWebEngineContextMenuRequest> m_contextMenuRequest;

    friend class extensions::MimeHandlerViewGuestDelegateQt;
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_VIEW_QT_H
