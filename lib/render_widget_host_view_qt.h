/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CONTENT_BROWSER_RENDERER_HOST_RENDER_WIDGET_HOST_VIEW_QT_H_
#define CONTENT_BROWSER_RENDERER_HOST_RENDER_WIDGET_HOST_VIEW_QT_H_

#include "shared/shared_globals.h"

#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include <qglobal.h>

class BackingStoreQt;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QTouchEvent;
class QWheelEvent;
class RenderWidgetHostViewQtDelegate;

class RenderWidgetHostViewQt
    : public content::RenderWidgetHostViewBase
{
public:
    RenderWidgetHostViewQt(content::RenderWidgetHost* widget);
    ~RenderWidgetHostViewQt();

    void SetDelegate(RenderWidgetHostViewQtDelegate* delegate) { m_delegate = delegate; }
    bool handleEvent(QEvent* event);
    BackingStoreQt* GetBackingStore();

    virtual content::BackingStore *AllocBackingStore(const gfx::Size &size);

    virtual void InitAsChild(gfx::NativeView parent_view);
    virtual void InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&);
    virtual void InitAsFullscreen(content::RenderWidgetHostView*);
    virtual content::RenderWidgetHost* GetRenderWidgetHost() const;
    virtual void SetSize(const gfx::Size& size);
    virtual void SetBounds(const gfx::Rect& rect);
    virtual gfx::Size GetPhysicalBackingSize() const;
    virtual gfx::NativeView GetNativeView() const;
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
#if defined(TOOLKIT_GTK)
    virtual GdkEventButton* GetLastMouseDown();
    virtual gfx::NativeView BuildInputMethodsGtkMenu();
#endif  // defined(TOOLKIT_GTK)
    virtual void WasShown();
    virtual void WasHidden();
    virtual void MovePluginWindows(const gfx::Vector2d&, const std::vector<content::WebPluginGeometry>&);
    virtual void Blur();
    virtual void UpdateCursor(const WebCursor&);
    virtual void SetIsLoading(bool);
    virtual void TextInputTypeChanged(ui::TextInputType, bool, ui::TextInputMode);
    virtual void ImeCancelComposition();
    virtual void ImeCompositionRangeChanged(const ui::Range&, const std::vector<gfx::Rect>&);
    virtual void DidUpdateBackingStore(const gfx::Rect& scroll_rect, const gfx::Vector2d& scroll_delta, const std::vector<gfx::Rect>& copy_rects, const ui::LatencyInfo&);
    virtual void RenderProcessGone(base::TerminationStatus, int);
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

    void handleMouseEvent(QMouseEvent*);
    void handleKeyEvent(QKeyEvent*);
    void handleWheelEvent(QWheelEvent*);
    void handleTouchEvent(QTouchEvent*);
    void handleFocusEvent(QFocusEvent*);

#if defined(OS_MACOSX)
    virtual void SetTakesFocusOnlyOnMouseDown(bool flag) { QT_NOT_YET_IMPLEMENTED }
    virtual void SetActive(bool active) { QT_NOT_YET_IMPLEMENTED }
    virtual bool IsSpeaking() const { QT_NOT_YET_IMPLEMENTED; return false; }
    virtual void SpeakSelection() { QT_NOT_YET_IMPLEMENTED }
    virtual bool PostProcessEventForPluginIme(const content::NativeWebKeyboardEvent& event) { QT_NOT_YET_IMPLEMENTED; return false; }
    virtual void AboutToWaitForBackingStoreMsg() { QT_NOT_YET_IMPLEMENTED }
    virtual void StopSpeaking() { QT_NOT_YET_IMPLEMENTED }
    virtual void SetWindowVisibility(bool visible) { QT_NOT_YET_IMPLEMENTED }
    virtual bool SupportsSpeech() const { QT_NOT_YET_IMPLEMENTED; return false; }
    virtual void ShowDefinitionForSelection() { QT_NOT_YET_IMPLEMENTED }
    virtual void WindowFrameChanged() { QT_NOT_YET_IMPLEMENTED }
#endif // defined(OS_MACOSX)

private:
    void Paint(const gfx::Rect& damage_rect);

    bool IsPopup() const;

    content::RenderWidgetHostImpl *m_host;
    RenderWidgetHostViewQtDelegate *m_delegate;
    gfx::Size m_requestedSize;
};

#endif
