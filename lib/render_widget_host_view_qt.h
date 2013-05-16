#ifndef CONTENT_BROWSER_RENDERER_HOST_RENDER_WIDGET_HOST_VIEW_QT_H_
#define CONTENT_BROWSER_RENDERER_HOST_RENDER_WIDGET_HOST_VIEW_QT_H_

#include "content/browser/renderer_host/render_widget_host_view_base.h"

class QEvent;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class RasterWindow;

namespace content {

class RenderWidgetHostViewQt
    : public content::RenderWidgetHostViewBase
{
public:
    RenderWidgetHostViewQt(content::RenderWidgetHost* widget);
    ~RenderWidgetHostViewQt();

    bool handleEvent(QEvent* event);

    virtual content::BackingStore *AllocBackingStore(const gfx::Size &size);
    static RenderWidgetHostView* CreateViewForWidget(content::RenderWidgetHost* widget);

    virtual void InitAsChild(gfx::NativeView parent_view);
    virtual void InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&);
    virtual void InitAsFullscreen(content::RenderWidgetHostView*);
    virtual content::RenderWidgetHost* GetRenderWidgetHost() const;
    virtual void SetSize(const gfx::Size& size);
    virtual void SetBounds(const gfx::Rect& rect);
    virtual gfx::NativeView GetNativeView() const;
    virtual QWindow* GetNativeViewQt() const OVERRIDE;
    virtual gfx::NativeViewId GetNativeViewId() const;
    virtual gfx::NativeViewAccessible GetNativeViewAccessible();
    virtual void Focus();
    virtual bool HasFocus() const;
    virtual bool IsSurfaceAvailableForCopy() const;
    virtual void Show();
    virtual void Hide();
    virtual bool IsShowing();
    virtual gfx::Rect GetViewBounds() const;
    virtual void SetBackground(const SkBitmap& background);
    virtual bool LockMouse();
    virtual void UnlockMouse();
    virtual bool IsMouseLocked();
#if defined(TOOLKIT_GTK)
    virtual GdkEventButton* GetLastMouseDown();
    virtual gfx::NativeView BuildInputMethodsGtkMenu();
#endif  // defined(TOOLKIT_GTK)
    virtual void WasShown();
    virtual void WasHidden();
    virtual void MovePluginWindows(const gfx::Vector2d&, const std::vector<webkit::npapi::WebPluginGeometry>&);
    virtual void Blur();
    virtual void UpdateCursor(const WebCursor&);
    virtual void SetIsLoading(bool);
    virtual void TextInputStateChanged(const ViewHostMsg_TextInputState_Params&);
    virtual void ImeCancelComposition();
    virtual void ImeCompositionRangeChanged(const ui::Range&, const std::vector<gfx::Rect>&);
    virtual void DidUpdateBackingStore(const gfx::Rect& scroll_rect, const gfx::Vector2d& scroll_delta, const std::vector<gfx::Rect>& copy_rects);
    virtual void RenderViewGone(base::TerminationStatus, int);
    virtual void Destroy();
    virtual void SetTooltipText(const string16&);
    virtual void SelectionBoundsChanged(const ViewHostMsg_SelectionBounds_Params&);
    virtual void ScrollOffsetChanged();
    virtual void CopyFromCompositingSurface(const gfx::Rect& src_subrect, const gfx::Size& /* dst_size */, const base::Callback<void(bool, const SkBitmap&)>& callback);
    virtual void CopyFromCompositingSurfaceToVideoFrame(const gfx::Rect& src_subrect, const scoped_refptr<media::VideoFrame>& target, const base::Callback<void(bool)>& callback);
    virtual bool CanCopyToVideoFrame() const;
    virtual void OnAcceleratedCompositingStateChange();
    virtual void AcceleratedSurfaceBuffersSwapped(const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params, int gpu_host_id);
    virtual void AcceleratedSurfacePostSubBuffer(const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params, int gpu_host_id);
    virtual void AcceleratedSurfaceSuspend();
    virtual void AcceleratedSurfaceRelease();
    virtual bool HasAcceleratedSurface(const gfx::Size&);
    virtual void GetScreenInfo(WebKit::WebScreenInfo* results);
    virtual gfx::Rect GetBoundsInRootWindow();
    virtual gfx::GLSurfaceHandle GetCompositingSurface();
    virtual void SetHasHorizontalScrollbar(bool);
    virtual void SetScrollOffsetPinning(bool, bool);
    virtual void OnAccessibilityNotifications(const std::vector<AccessibilityHostMsg_NotificationParams>&);

private:
    void Paint(const gfx::Rect& scroll_rect);

    bool IsPopup() const;
    void handleMouseEvent(QMouseEvent*);
    void handleKeyEvent(QKeyEvent*);
    void handleWheelEvent(QWheelEvent*);

    content::RenderWidgetHostImpl *m_host;
    RasterWindow *m_view;
    gfx::Size m_requestedSize;
};

}

#endif
