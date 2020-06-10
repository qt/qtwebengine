/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_H
#define RENDER_WIDGET_HOST_VIEW_QT_H

#include "compositor/display_frame_sink.h"
#include "delegated_frame_host_client_qt.h"
#include "render_widget_host_view_qt_delegate.h"

#include "base/memory/weak_ptr.h"
#include "components/viz/common/resources/transferable_resource.h"
#include "components/viz/common/surfaces/parent_local_surface_id_allocator.h"
#include "components/viz/host/host_frame_sink_client.h"
#include "content/browser/accessibility/browser_accessibility_manager.h"
#include "content/browser/renderer_host/input/mouse_wheel_phase_handler.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/browser/renderer_host/text_input_manager.h"
#include "content/public/browser/render_process_host_observer.h"
#include "gpu/ipc/common/gpu_messages.h"
#include "ui/events/gesture_detection/filtered_gesture_provider.h"

#include <QMap>
#include <QPoint>
#include <QtGlobal>
#include <QtGui/QTouchEvent>

QT_BEGIN_NAMESPACE
class QAccessibleInterface;
QT_END_NAMESPACE

namespace content {
class RenderFrameHost;
class RenderWidgetHostImpl;
namespace mojom {
class FrameInputHandler;
}
}

namespace ui {
class TouchSelectionController;
}

namespace QtWebEngineCore {

class TouchHandleDrawableClient;
class TouchSelectionControllerClientQt;
class TouchSelectionMenuController;

struct MultipleMouseClickHelper
{
    QPoint lastPressPosition;
    Qt::MouseButton lastPressButton;
    int clickCounter;
    ulong lastPressTimestamp;

    MultipleMouseClickHelper()
        : lastPressPosition(QPoint())
        , lastPressButton(Qt::NoButton)
        , clickCounter(0)
        , lastPressTimestamp(0)
    {
    }
};

class RenderWidgetHostViewQt
    : public content::RenderWidgetHostViewBase
    , public ui::GestureProviderClient
    , public RenderWidgetHostViewQtDelegateClient
    , public base::SupportsWeakPtr<RenderWidgetHostViewQt>
    , public content::TextInputManager::Observer
    , public DisplayConsumer
{
public:
    enum LoadVisuallyCommittedState {
        NotCommitted,
        DidFirstVisuallyNonEmptyPaint,
        DidFirstCompositorFrameSwap
    };

    RenderWidgetHostViewQt(content::RenderWidgetHost* widget);
    ~RenderWidgetHostViewQt();

    RenderWidgetHostViewQtDelegate *delegate() { return m_delegate.get(); }
    void setDelegate(RenderWidgetHostViewQtDelegate *delegate);
    WebContentsAdapterClient *adapterClient() { return m_adapterClient; }
    void setAdapterClient(WebContentsAdapterClient *adapterClient);

    void InitAsChild(gfx::NativeView) override;
    void InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&) override;
    void InitAsFullscreen(content::RenderWidgetHostView*) override;
    void SetSize(const gfx::Size& size) override;
    void SetBounds(const gfx::Rect&) override;
    gfx::NativeView GetNativeView() override;
    gfx::NativeViewAccessible GetNativeViewAccessible() override;
    void Focus() override;
    bool HasFocus() override;
    bool IsMouseLocked() override;
    bool IsSurfaceAvailableForCopy() override;
    void CopyFromSurface(const gfx::Rect &src_rect,
                         const gfx::Size &output_size,
                         base::OnceCallback<void(const SkBitmap &)> callback) override;
    void Show() override;
    void Hide() override;
    bool IsShowing() override;
    gfx::Rect GetViewBounds() override;
    void UpdateBackgroundColor() override;
    blink::mojom::PointerLockResult LockMouse(bool) override;
    blink::mojom::PointerLockResult ChangeMouseLock(bool) override;
    void UnlockMouse() override;
    void UpdateCursor(const content::WebCursor&) override;
    void DisplayCursor(const content::WebCursor&) override;
    void SetIsLoading(bool) override;
    void ImeCancelComposition() override;
    void ImeCompositionRangeChanged(const gfx::Range&, const std::vector<gfx::Rect>&) override;
    void RenderProcessGone() override;
    void Destroy() override;
    void SetTooltipText(const base::string16 &tooltip_text) override;
    void DisplayTooltipText(const base::string16& tooltip_text) override;
    void WheelEventAck(const blink::WebMouseWheelEvent &event, content::InputEventAckState ack_result) override;
    void GestureEventAck(const blink::WebGestureEvent &event, content::InputEventAckState ack_result) override;
    content::MouseWheelPhaseHandler *GetMouseWheelPhaseHandler() override;
    viz::ScopedSurfaceIdAllocator DidUpdateVisualProperties(const cc::RenderFrameMetadata &metadata) override;
    void OnDidUpdateVisualPropertiesComplete(const cc::RenderFrameMetadata &metadata);

    void GetScreenInfo(content::ScreenInfo *results) override;
    gfx::Rect GetBoundsInRootWindow() override;
    void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo &touch, content::InputEventAckState ack_result) override;
    viz::SurfaceId GetCurrentSurfaceId() const override;
    const viz::FrameSinkId &GetFrameSinkId() const override;
    const viz::LocalSurfaceIdAllocation &GetLocalSurfaceIdAllocation() const override;
    void TakeFallbackContentFrom(content::RenderWidgetHostView *view) override;
    void EnsureSurfaceSynchronizedForWebTest() override;
    uint32_t GetCaptureSequenceNumber() const override;
    void ResetFallbackToFirstNavigationSurface() override;
    void DidStopFlinging() override;
    std::unique_ptr<content::SyntheticGestureTarget> CreateSyntheticGestureTarget() override;
    ui::Compositor *GetCompositor() override;

    // Overridden from ui::GestureProviderClient.
    void OnGestureEvent(const ui::GestureEventData& gesture) override;

    // Overridden from RenderWidgetHostViewQtDelegateClient.
    QSGNode *updatePaintNode(QSGNode *) override;
    void notifyShown() override;
    void notifyHidden() override;
    void visualPropertiesChanged() override;
    bool forwardEvent(QEvent *) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) override;
    void closePopup() override;

    // Overridden from content::TextInputManager::Observer
    void OnUpdateTextInputStateCalled(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view, bool did_update_state) override;
    void OnSelectionBoundsChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view) override;
    void OnTextSelectionChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view) override;

    void handleMouseEvent(QMouseEvent*);
    void handleKeyEvent(QKeyEvent*);
    void handleWheelEvent(QWheelEvent*);
    void handleTouchEvent(QTouchEvent*);
#if QT_CONFIG(tabletevent)
    void handleTabletEvent(QTabletEvent *ev);
#endif
#ifndef QT_NO_GESTURES
    void handleGestureEvent(QNativeGestureEvent *);
#endif
    void handleHoverEvent(QHoverEvent*);
    void handleFocusEvent(QFocusEvent*);
    void handleInputMethodEvent(QInputMethodEvent*);
    void handleInputMethodQueryEvent(QInputMethodQueryEvent*);

    template<class T> void handlePointerEvent(T*);

#if defined(OS_MACOSX)
    void SetActive(bool active) override { QT_NOT_YET_IMPLEMENTED }
    void SpeakSelection() override { QT_NOT_YET_IMPLEMENTED }
    void ShowDefinitionForSelection() override { QT_NOT_YET_IMPLEMENTED }
#endif // defined(OS_MACOSX)

    // Overridden from content::BrowserAccessibilityDelegate
    content::BrowserAccessibilityManager* CreateBrowserAccessibilityManager(content::BrowserAccessibilityDelegate* delegate, bool for_root_frame) override;

    // Called from WebContentsDelegateQt
    void OnDidFirstVisuallyNonEmptyPaint();

    // Overridden from content::RenderFrameMetadataProvider::Observer
    void OnRenderFrameMetadataChangedAfterActivation() override;

    // Overridden from DisplayConsumer
    void scheduleUpdate() override;

    gfx::SizeF lastContentsSize() const { return m_lastContentsSize; }
    gfx::Vector2dF lastScrollOffset() const { return m_lastScrollOffset; }

    ui::TouchSelectionController *getTouchSelectionController() const { return m_touchSelectionController.get(); }
    TouchSelectionControllerClientQt *getTouchSelectionControllerClient() const { return m_touchSelectionControllerClient.get(); }
    content::mojom::FrameInputHandler *getFrameInputHandler();
    ui::TextInputType getTextInputType() const;

private:
    friend class DelegatedFrameHostClientQt;

    void processMotionEvent(const ui::MotionEvent &motionEvent);
    void clearPreviousTouchMotionState();
    QList<QTouchEvent::TouchPoint> mapTouchPointIds(const QList<QTouchEvent::TouchPoint> &inputPoints);

    bool IsPopup() const;

    void selectionChanged();
    content::RenderFrameHost *getFocusedFrameHost();

    void synchronizeVisualProperties(const base::Optional<viz::LocalSurfaceIdAllocation> &childSurfaceId);

    void callUpdate();

    // Geometry of the view in screen DIPs.
    gfx::Rect m_viewRectInDips;
    // Geometry of the window, including frame, in screen DIPs.
    gfx::Rect m_windowRectInDips;
    content::ScreenInfo m_screenInfo;

    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;

    ui::FilteredGestureProvider m_gestureProvider;
    base::TimeDelta m_eventsToNowDelta;
    bool m_sendMotionActionDown;
    bool m_touchMotionStarted;
    QMap<int, int> m_touchIdMapping;
    QList<QTouchEvent::TouchPoint> m_previousTouchPoints;
    std::unique_ptr<RenderWidgetHostViewQtDelegate> m_delegate;

    bool m_visible;
    bool m_deferredShow = false;
    DelegatedFrameHostClientQt m_delegatedFrameHostClient{this};
    std::unique_ptr<content::DelegatedFrameHost> m_delegatedFrameHost;
    std::unique_ptr<ui::Layer> m_rootLayer;
    std::unique_ptr<ui::Compositor> m_uiCompositor;
    scoped_refptr<DisplayFrameSink> m_displayFrameSink;
    LoadVisuallyCommittedState m_loadVisuallyCommittedState;

    QMetaObject::Connection m_adapterClientDestroyedConnection;
    WebContentsAdapterClient *m_adapterClient;
    MultipleMouseClickHelper m_clickHelper;

    bool m_imeInProgress;
    bool m_receivedEmptyImeEvent;
    QPoint m_previousMousePosition;
    bool m_isMouseLocked;

    gfx::Vector2dF m_lastScrollOffset;
    gfx::SizeF m_lastContentsSize;
    viz::ParentLocalSurfaceIdAllocator m_dfhLocalSurfaceIdAllocator;
    viz::ParentLocalSurfaceIdAllocator m_uiCompositorLocalSurfaceIdAllocator;

    uint m_imState;
    int m_anchorPositionWithinSelection;
    int m_cursorPositionWithinSelection;
    uint m_cursorPosition;
    bool m_emptyPreviousSelection;
    QString m_surroundingText;

    bool m_imeHasHiddenTextCapability;

    bool m_wheelAckPending;
    QList<blink::WebMouseWheelEvent> m_pendingWheelEvents;
    content::MouseWheelPhaseHandler m_mouseWheelPhaseHandler;
    viz::FrameSinkId m_frameSinkId;

    std::string m_editCommand;

    std::unique_ptr<TouchSelectionControllerClientQt> m_touchSelectionControllerClient;
    std::unique_ptr<ui::TouchSelectionController> m_touchSelectionController;
    gfx::SelectionBound m_selectionStart;
    gfx::SelectionBound m_selectionEnd;

    base::WeakPtrFactory<RenderWidgetHostViewQt> m_weakPtrFactory{this};

    uint m_mouseButtonPressed = 0;
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_H
