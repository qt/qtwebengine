// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RENDER_WIDGET_HOST_VIEW_QT_H
#define RENDER_WIDGET_HOST_VIEW_QT_H

#include "compositor/compositor.h"
#include "delegated_frame_host_client_qt.h"
#include "render_widget_host_view_qt_delegate.h"

#include "base/memory/weak_ptr.h"
#include "components/viz/common/resources/transferable_resource.h"
#include "components/viz/common/surfaces/parent_local_surface_id_allocator.h"
#include "components/viz/host/host_frame_sink_client.h"
#include "content/browser/accessibility/web_contents_accessibility.h"
#include "content/browser/renderer_host/input/mouse_wheel_phase_handler.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/browser/renderer_host/text_input_manager.h"
#include "ui/events/gesture_detection/filtered_gesture_provider.h"

namespace content {
class RenderFrameHost;
class RenderWidgetHostImpl;
class WebContents;
}

namespace ui {
class TouchSelectionController;
}

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegateClient;
class InputEventObserverQt;
class TouchSelectionControllerClientQt;
class WebContentsAccessibilityQt;
class WebContentsAdapterClient;

class RenderWidgetHostViewQt
    : public content::RenderWidgetHostViewBase
    , public ui::GestureProviderClient
    , public base::SupportsWeakPtr<RenderWidgetHostViewQt>
    , public content::TextInputManager::Observer
    , public content::RenderFrameMetadataProvider::Observer
    , public content::RenderWidgetHost::InputEventObserver
{
public:
    RenderWidgetHostViewQt(content::RenderWidgetHost* widget);
    ~RenderWidgetHostViewQt();

    RenderWidgetHostViewQtDelegate *delegate() { return m_delegate.get(); }
    void setDelegate(RenderWidgetHostViewQtDelegate *delegate);
    WebContentsAdapterClient *adapterClient() { return m_adapterClient; }
    void setAdapterClient(WebContentsAdapterClient *adapterClient);
    RenderWidgetHostViewQtDelegateClient *delegateClient() const { return m_delegateClient.get(); }

    // Overridden from RenderWidgetHostView:
    void InitAsChild(gfx::NativeView) override;
    void InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&, const gfx::Rect&) override;
    void SetSize(const gfx::Size& size) override;
    void SetBounds(const gfx::Rect&) override;
    gfx::NativeView GetNativeView() override;
    gfx::NativeViewAccessible GetNativeViewAccessible() override { return nullptr; }
    void Focus() override;
    bool HasFocus() override;
    bool IsMouseLocked() override;
    viz::FrameSinkId GetRootFrameSinkId() override;
    bool IsSurfaceAvailableForCopy() override;
    void CopyFromSurface(const gfx::Rect &src_rect,
                         const gfx::Size &output_size,
                         base::OnceCallback<void(const SkBitmap &)> callback) override;
    void ShowWithVisibility(content::PageVisibilityState page_visibility) override;
    void Hide() override;
    bool IsShowing() override;
    gfx::Rect GetViewBounds() override;
    void UpdateBackgroundColor() override;
    blink::mojom::PointerLockResult LockMouse(bool) override;
    blink::mojom::PointerLockResult ChangeMouseLock(bool) override;
    void UnlockMouse() override;
    void UpdateCursor(const ui::Cursor&) override;
    void DisplayCursor(const ui::Cursor&) override;
    content::CursorManager *GetCursorManager() override;
    void SetIsLoading(bool) override;
    void ImeCancelComposition() override;
    void ImeCompositionRangeChanged(const gfx::Range&, const std::vector<gfx::Rect>&) override;
    void RenderProcessGone() override;
    bool TransformPointToCoordSpaceForView(const gfx::PointF &point,
                                           content::RenderWidgetHostViewBase *target_view,
                                           gfx::PointF *transformed_point) override;
    void Destroy() override;
    void UpdateTooltipUnderCursor(const std::u16string &tooltip_text) override;
    void UpdateTooltip(const std::u16string& tooltip_text) override;
    void WheelEventAck(const blink::WebMouseWheelEvent &event,
                       blink::mojom::InputEventResultState ack_result) override;
    void GestureEventAck(const blink::WebGestureEvent &event,
                         blink::mojom::InputEventResultState ack_result,
                         blink::mojom::ScrollResultDataPtr scroll_result_data) override;
    content::MouseWheelPhaseHandler *GetMouseWheelPhaseHandler() override;
    viz::ScopedSurfaceIdAllocator DidUpdateVisualProperties(const cc::RenderFrameMetadata &metadata) override;
    void OnDidUpdateVisualPropertiesComplete(const cc::RenderFrameMetadata &metadata);

    // Overridden from RenderWidgetHostViewBase:
    gfx::Rect GetBoundsInRootWindow() override;
    void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo &touch,
                                blink::mojom::InputEventResultState ack_result) override;
    viz::SurfaceId GetCurrentSurfaceId() const override;
    const viz::FrameSinkId &GetFrameSinkId() const override;
    const viz::LocalSurfaceId &GetLocalSurfaceId() const override;
    void FocusedNodeChanged(bool is_editable_node, const gfx::Rect& node_bounds_in_screen) override;
    base::flat_map<std::string, std::string> GetKeyboardLayoutMap() override;

    void TakeFallbackContentFrom(content::RenderWidgetHostView *view) override;
    void EnsureSurfaceSynchronizedForWebTest() override;
    uint32_t GetCaptureSequenceNumber() const override;
    void ResetFallbackToFirstNavigationSurface() override;
    void DidStopFlinging() override;
    std::unique_ptr<content::SyntheticGestureTarget> CreateSyntheticGestureTarget() override;
    ui::Compositor *GetCompositor() override;
    absl::optional<content::DisplayFeature> GetDisplayFeature() override;
    void SetDisplayFeatureForTesting(const content::DisplayFeature*) override;
    content::WebContentsAccessibility *GetWebContentsAccessibility() override;
#if BUILDFLAG(IS_MAC)
    void ShowSharePicker(
        const std::string &title,
        const std::string &text,
        const std::string &url,
        const std::vector<std::string> &file_paths,
            blink::mojom::ShareService::ShareCallback callback) override { QT_NOT_YET_IMPLEMENTED }
    void SetActive(bool active) override { QT_NOT_YET_IMPLEMENTED }
    void SpeakSelection() override { QT_NOT_YET_IMPLEMENTED }
    void ShowDefinitionForSelection() override { QT_NOT_YET_IMPLEMENTED }
    void SetWindowFrameInScreen(const gfx::Rect&) override { QT_NOT_YET_IMPLEMENTED }
#endif // BUILDFLAG(IS_MAC)
    void NotifyHostAndDelegateOnWasShown(blink::mojom::RecordContentToVisibleTimeRequestPtr) override { QT_NOT_YET_IMPLEMENTED }
    void RequestSuccessfulPresentationTimeFromHostOrDelegate(blink::mojom::RecordContentToVisibleTimeRequestPtr) override {}
    void CancelSuccessfulPresentationTimeRequestForHostAndDelegate() override {}

    // Overridden from ui::GestureProviderClient.
    void OnGestureEvent(const ui::GestureEventData& gesture) override;

    // Overridden from content::TextInputManager::Observer
    void OnUpdateTextInputStateCalled(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view, bool did_update_state) override;
    void OnSelectionBoundsChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view) override;
    void OnTextSelectionChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view) override;

    // Overridden from content::RenderFrameMetadataProvider::Observer
    void OnRenderFrameMetadataChangedAfterActivation(base::TimeTicks activation_time) override;
    void OnRenderFrameMetadataChangedBeforeActivation(const cc::RenderFrameMetadata &) override {}
    void OnRenderFrameSubmission() override {}
    void OnLocalSurfaceIdChanged(const cc::RenderFrameMetadata &) override {}

    // Overridden from content::RenderWidgetHost::InputEventObserver
    void OnInputEvent(const blink::WebInputEvent &) override { }
    void OnInputEventAck(blink::mojom::InputEventResultSource,
                         blink::mojom::InputEventResultState state,
                         const blink::WebInputEvent &event) override;

    static void registerInputEventObserver(content::WebContents *, content::RenderFrameHost *);

    // Called from RenderWidgetHostViewQtDelegateClient.
    Compositor::Id compositorId();
    void notifyShown();
    void notifyHidden();
    bool updateScreenInfo();
    void handleWheelEvent(QWheelEvent *);
    void processMotionEvent(const ui::MotionEvent &motionEvent);
    void resetInputManagerState() { m_imState = 0; }

    // Called from WebContentsAdapter.
    gfx::SizeF lastContentsSize() const { return m_lastContentsSize; }
    gfx::PointF lastScrollOffset() const { return m_lastScrollOffset; }

    ui::TouchSelectionController *getTouchSelectionController() const { return m_touchSelectionController.get(); }
    TouchSelectionControllerClientQt *getTouchSelectionControllerClient() const { return m_touchSelectionControllerClient.get(); }
    blink::mojom::FrameWidgetInputHandler *getFrameWidgetInputHandler();
    ui::TextInputType getTextInputType() const;

    void synchronizeVisualProperties(
            const absl::optional<viz::LocalSurfaceId> &childSurfaceId);

    void resetTouchSelectionController();

private:
    friend class DelegatedFrameHostClientQt;
    friend class WebContentsAccessibilityQt;

    bool isPopup() const;

    bool updateCursorFromResource(ui::mojom::CursorType type);

    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;

    std::unique_ptr<content::CursorManager> m_cursorManager;

    ui::FilteredGestureProvider m_gestureProvider;

    viz::FrameSinkId m_frameSinkId;
    std::unique_ptr<RenderWidgetHostViewQtDelegateClient> m_delegateClient;
    std::unique_ptr<RenderWidgetHostViewQtDelegate> m_delegate;
    std::unique_ptr<WebContentsAccessibilityQt> m_webContentsAccessibility;
    QMetaObject::Connection m_adapterClientDestroyedConnection;
    WebContentsAdapterClient *m_adapterClient = nullptr;

    bool m_isMouseLocked = false;
    bool m_visible = false;
    bool m_deferredShow = false;
    gfx::PointF m_lastScrollOffset;
    gfx::SizeF m_lastContentsSize;
    DelegatedFrameHostClientQt m_delegatedFrameHostClient { this };

    // VIZ
    std::unique_ptr<content::DelegatedFrameHost> m_delegatedFrameHost;
    std::unique_ptr<ui::Layer> m_rootLayer;
    std::unique_ptr<ui::Compositor> m_uiCompositor;
    viz::ParentLocalSurfaceIdAllocator m_dfhLocalSurfaceIdAllocator;
    viz::ParentLocalSurfaceIdAllocator m_uiCompositorLocalSurfaceIdAllocator;

    // IME
    uint m_imState = 0;

    // Wheel
    bool m_wheelAckPending = false;
    QList<blink::WebMouseWheelEvent> m_pendingWheelEvents;
    content::MouseWheelPhaseHandler m_mouseWheelPhaseHandler { this };

    // TouchSelection
    std::unique_ptr<TouchSelectionControllerClientQt> m_touchSelectionControllerClient;
    std::unique_ptr<ui::TouchSelectionController> m_touchSelectionController;
    gfx::SelectionBound m_selectionStart;
    gfx::SelectionBound m_selectionEnd;

    base::WeakPtrFactory<RenderWidgetHostViewQt> m_weakPtrFactory { this };
};

class WebContentsAccessibilityQt : public content::WebContentsAccessibility
{
    RenderWidgetHostViewQt *m_rwhv;
public:
    WebContentsAccessibilityQt(RenderWidgetHostViewQt *rwhv) : m_rwhv(rwhv) {}
    QObject *accessibilityParentObject() const;
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_H
