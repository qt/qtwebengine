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

#include "render_widget_host_view_qt.h"

#include "browser_accessibility_manager_qt.h"
#include "browser_accessibility_qt.h"
#include "chromium_overrides.h"
#include "delegated_frame_node.h"
#include "render_widget_host_view_qt_delegate.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"
#include "web_event_factory.h"

#include "base/command_line.h"
#include "cc/output/compositor_frame_ack.h"
#include "content/browser/accessibility/browser_accessibility_state_impl.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/common/view_messages.h"
#include "content/public/common/content_switches.h"
#include "content/public/browser/browser_accessibility_state.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#include "ui/events/event.h"
#include "ui/gfx/size_conversions.h"
#include "webkit/common/cursors/webcursor.h"

#include <QEvent>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QTextFormat>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QStyleHints>
#include <QVariant>
#include <QWheelEvent>
#include <QWindow>
#include <QtGui/qaccessible.h>

static inline ui::EventType toUIEventType(Qt::TouchPointState state)
{
    switch (state) {
    case Qt::TouchPointPressed:
        return ui::ET_TOUCH_PRESSED;
    case Qt::TouchPointMoved:
        return ui::ET_TOUCH_MOVED;
    case Qt::TouchPointStationary:
        return ui::ET_TOUCH_STATIONARY;
    case Qt::TouchPointReleased:
        return ui::ET_TOUCH_RELEASED;
    default:
        Q_ASSERT(false);
        return ui::ET_UNKNOWN;
    }
}

static inline Qt::InputMethodHints toQtInputMethodHints(ui::TextInputType inputType)
{
    switch (inputType) {
    case ui::TEXT_INPUT_TYPE_TEXT:
        return Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_SEARCH:
        return Qt::ImhPreferLowercase | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_PASSWORD:
        return Qt::ImhSensitiveData | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_EMAIL:
        return Qt::ImhEmailCharactersOnly;
    case ui::TEXT_INPUT_TYPE_NUMBER:
        return Qt::ImhFormattedNumbersOnly;
    case ui::TEXT_INPUT_TYPE_TELEPHONE:
        return Qt::ImhDialableCharactersOnly;
    case ui::TEXT_INPUT_TYPE_URL:
        return Qt::ImhUrlCharactersOnly | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_DATE_TIME:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_LOCAL:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_FIELD:
        return Qt::ImhDate | Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_DATE:
    case ui::TEXT_INPUT_TYPE_MONTH:
    case ui::TEXT_INPUT_TYPE_WEEK:
        return Qt::ImhDate;
    case ui::TEXT_INPUT_TYPE_TIME:
        return Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_TEXT_AREA:
    case ui::TEXT_INPUT_TYPE_CONTENT_EDITABLE:
        return Qt::ImhMultiLine | Qt::ImhPreferLowercase;
    default:
        return Qt::ImhNone;
    }
}

static inline gfx::Point toGfxPoint(const QPoint& point)
{
    return gfx::Point(point.x(), point.y());
}

static void UpdateWebTouchEventAfterDispatch(blink::WebTouchEvent* event, blink::WebTouchPoint* point) {
    if (point->state != blink::WebTouchPoint::StateReleased &&
        point->state != blink::WebTouchPoint::StateCancelled)
        return;
    --event->touchesLength;
    for (unsigned i = point - event->touches; i < event->touchesLength; ++i) {
        event->touches[i] = event->touches[i + 1];
    }
}

static bool shouldSendPinchGesture()
{
    static bool pinchAllowed = CommandLine::ForCurrentProcess()->HasSwitch(switches::kEnablePinch);
    return pinchAllowed;
}

RenderWidgetHostViewQt::RenderWidgetHostViewQt(content::RenderWidgetHost* widget)
    : m_host(content::RenderWidgetHostImpl::From(widget))
    , m_gestureRecognizer(ui::GestureRecognizer::Create())
    , m_frameNodeData(new DelegatedFrameNodeData)
    , m_needsDelegatedFrameAck(false)
    , m_didFirstVisuallyNonEmptyLayout(false)
    , m_adapterClient(0)
    , m_anchorPositionWithinSelection(0)
    , m_cursorPositionWithinSelection(0)
    , m_initPending(false)
{
    m_host->SetView(this);
    m_gestureRecognizer->AddGestureEventHelper(this);
}

RenderWidgetHostViewQt::~RenderWidgetHostViewQt()
{
    m_gestureRecognizer->RemoveGestureEventHelper(this);
}

void RenderWidgetHostViewQt::setDelegate(RenderWidgetHostViewQtDelegate* delegate)
{
    m_delegate.reset(delegate);
}

void RenderWidgetHostViewQt::setAdapterClient(WebContentsAdapterClient *adapterClient)
{
    Q_ASSERT(!m_adapterClient);

    m_adapterClient = adapterClient;
    if (m_initPending)
        InitAsChild(0);
}

content::BackingStore *RenderWidgetHostViewQt::AllocBackingStore(const gfx::Size &size)
{
    Q_UNREACHABLE();
}

void RenderWidgetHostViewQt::InitAsChild(gfx::NativeView)
{
    if (!m_adapterClient) {
        m_initPending = true;
        return;
    }
    m_initPending = false;
    m_delegate->initAsChild(m_adapterClient);
}

void RenderWidgetHostViewQt::InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect& rect)
{
    QRect screenRect = toQt(rect);
    screenRect.moveTo(m_adapterClient->mapToGlobal(screenRect.topLeft()));
    m_delegate->initAsPopup(screenRect);
}

void RenderWidgetHostViewQt::InitAsFullscreen(content::RenderWidgetHostView*)
{
}

content::RenderWidgetHost* RenderWidgetHostViewQt::GetRenderWidgetHost() const
{
    return m_host;
}

void RenderWidgetHostViewQt::SetSize(const gfx::Size& size)
{
    int width = size.width();
    int height = size.height();

    m_delegate->resize(width,height);
}

void RenderWidgetHostViewQt::SetBounds(const gfx::Rect& rect)
{
    // This is called when webkit has sent us a Move message.
     if (IsPopup())
         m_delegate->move(m_adapterClient->mapToGlobal(toQt(rect.origin())));
    SetSize(rect.size());
}

gfx::Size RenderWidgetHostViewQt::GetPhysicalBackingSize() const
{
    if (!m_delegate || !m_delegate->window() || !m_delegate->window()->screen())
        return gfx::Size();

    const QScreen* screen = m_delegate->window()->screen();
    gfx::SizeF size = toGfx(m_delegate->screenRect().size());
    return gfx::ToCeiledSize(gfx::ScaleSize(size, screen->devicePixelRatio()));
}

gfx::NativeView RenderWidgetHostViewQt::GetNativeView() const
{
    // gfx::NativeView is a typedef to a platform specific view
    // pointer (HWND, NSView*, GtkWidget*) and other ports use
    // this function in the renderer_host layer when setting up
    // the view hierarchy and for generating snapshots in tests.
    // Since we manage the view hierarchy in Qt its value hasn't
    // been meaningful.
    return gfx::NativeView();
}

gfx::NativeViewId RenderWidgetHostViewQt::GetNativeViewId() const
{
    return 0;
}

gfx::NativeViewAccessible RenderWidgetHostViewQt::GetNativeViewAccessible()
{
    return 0;
}

void RenderWidgetHostViewQt::CreateBrowserAccessibilityManagerIfNeeded()
{
    if (GetBrowserAccessibilityManager())
        return;

    SetBrowserAccessibilityManager(new content::BrowserAccessibilityManagerQt(
        m_adapterClient->accessibilityParentObject(),
        content::BrowserAccessibilityManagerQt::GetEmptyDocument(),
        this));
}

// Set focus to the associated View component.
void RenderWidgetHostViewQt::Focus()
{
    m_host->SetInputMethodActive(true);
    if (!IsPopup())
        m_delegate->setKeyboardFocus();
    m_host->Focus();
}

bool RenderWidgetHostViewQt::HasFocus() const
{
    return m_delegate->hasKeyboardFocus();
}

bool RenderWidgetHostViewQt::IsSurfaceAvailableForCopy() const
{
    return true;
}

void RenderWidgetHostViewQt::Show()
{
    m_delegate->show();
}

void RenderWidgetHostViewQt::Hide()
{
    m_delegate->hide();
}

bool RenderWidgetHostViewQt::IsShowing()
{
    return m_delegate->isVisible();
}

// Retrieve the bounds of the View, in screen coordinates.
gfx::Rect RenderWidgetHostViewQt::GetViewBounds() const
{
    QRectF p = m_delegate->contentsRect();
    float s = dpiScale();
    gfx::Point p1(floor(p.x() / s), floor(p.y() / s));
    gfx::Point p2(ceil(p.right() /s), ceil(p.bottom() / s));
    return gfx::BoundingRect(p1, p2);
}

// Subclasses should override this method to do what is appropriate to set
// the custom background for their platform.
void RenderWidgetHostViewQt::SetBackground(const SkBitmap& background)
{
    RenderWidgetHostViewBase::SetBackground(background);
    // Send(new ViewMsg_SetBackground(m_host->GetRoutingID(), background));
}

// Return value indicates whether the mouse is locked successfully or not.
bool RenderWidgetHostViewQt::LockMouse()
{
    QT_NOT_USED
    return false;
}
void RenderWidgetHostViewQt::UnlockMouse()
{
    QT_NOT_USED
}

void RenderWidgetHostViewQt::WasShown()
{
    m_host->WasShown();
}

void RenderWidgetHostViewQt::WasHidden()
{
    m_host->WasHidden();
}

void RenderWidgetHostViewQt::MovePluginWindows(const gfx::Vector2d&, const std::vector<content::WebPluginGeometry>&)
{
    // QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::Blur()
{
    m_host->SetInputMethodActive(false);
    m_host->Blur();
}

void RenderWidgetHostViewQt::UpdateCursor(const WebCursor &webCursor)
{
    WebCursor::CursorInfo cursorInfo;
    webCursor.GetCursorInfo(&cursorInfo);
    Qt::CursorShape shape;
    switch (cursorInfo.type) {
    case blink::WebCursorInfo::TypePointer:
        shape = Qt::ArrowCursor;
        break;
    case blink::WebCursorInfo::TypeCross:
        shape = Qt::CrossCursor;
        break;
    case blink::WebCursorInfo::TypeHand:
        shape = Qt::PointingHandCursor;
        break;
    case blink::WebCursorInfo::TypeIBeam:
        shape = Qt::IBeamCursor;
        break;
    case blink::WebCursorInfo::TypeWait:
        shape = Qt::WaitCursor;
        break;
    case blink::WebCursorInfo::TypeHelp:
        shape = Qt::WhatsThisCursor;
        break;
    case blink::WebCursorInfo::TypeEastResize:
    case blink::WebCursorInfo::TypeWestResize:
    case blink::WebCursorInfo::TypeEastWestResize:
    case blink::WebCursorInfo::TypeEastPanning:
    case blink::WebCursorInfo::TypeWestPanning:
        shape = Qt::SizeHorCursor;
        break;
    case blink::WebCursorInfo::TypeNorthResize:
    case blink::WebCursorInfo::TypeSouthResize:
    case blink::WebCursorInfo::TypeNorthSouthResize:
    case blink::WebCursorInfo::TypeNorthPanning:
    case blink::WebCursorInfo::TypeSouthPanning:
        shape = Qt::SizeVerCursor;
        break;
    case blink::WebCursorInfo::TypeNorthEastResize:
    case blink::WebCursorInfo::TypeSouthWestResize:
    case blink::WebCursorInfo::TypeNorthEastSouthWestResize:
    case blink::WebCursorInfo::TypeNorthEastPanning:
    case blink::WebCursorInfo::TypeSouthWestPanning:
        shape = Qt::SizeBDiagCursor;
        break;
    case blink::WebCursorInfo::TypeNorthWestResize:
    case blink::WebCursorInfo::TypeSouthEastResize:
    case blink::WebCursorInfo::TypeNorthWestSouthEastResize:
    case blink::WebCursorInfo::TypeNorthWestPanning:
    case blink::WebCursorInfo::TypeSouthEastPanning:
        shape = Qt::SizeFDiagCursor;
        break;
    case blink::WebCursorInfo::TypeColumnResize:
        shape = Qt::SplitHCursor;
        break;
    case blink::WebCursorInfo::TypeRowResize:
        shape = Qt::SplitVCursor;
        break;
    case blink::WebCursorInfo::TypeMiddlePanning:
    case blink::WebCursorInfo::TypeMove:
        shape = Qt::SizeAllCursor;
        break;
    case blink::WebCursorInfo::TypeVerticalText:
    case blink::WebCursorInfo::TypeCell:
    case blink::WebCursorInfo::TypeContextMenu:
    case blink::WebCursorInfo::TypeAlias:
    case blink::WebCursorInfo::TypeProgress:
    case blink::WebCursorInfo::TypeCopy:
    case blink::WebCursorInfo::TypeZoomIn:
    case blink::WebCursorInfo::TypeZoomOut:
        // FIXME: Load from the resource bundle.
        shape = Qt::ArrowCursor;
        break;
    case blink::WebCursorInfo::TypeNoDrop:
    case blink::WebCursorInfo::TypeNotAllowed:
        shape = Qt::ForbiddenCursor;
        break;
    case blink::WebCursorInfo::TypeNone:
        shape = Qt::BlankCursor;
        break;
    case blink::WebCursorInfo::TypeGrab:
        shape = Qt::OpenHandCursor;
        break;
    case blink::WebCursorInfo::TypeGrabbing:
        shape = Qt::ClosedHandCursor;
        break;
    case blink::WebCursorInfo::TypeCustom:
        // FIXME: Extract from the CursorInfo.
        shape = Qt::ArrowCursor;
        break;
    default:
        Q_UNREACHABLE();
        shape = Qt::ArrowCursor;
    }
    m_delegate->updateCursor(QCursor(shape));
}

void RenderWidgetHostViewQt::SetIsLoading(bool)
{
    // We use WebContentsDelegateQt::LoadingStateChanged to notify about loading state.
}

void RenderWidgetHostViewQt::TextInputTypeChanged(ui::TextInputType type, ui::TextInputMode, bool)
{
    m_currentInputType = type;
    m_delegate->inputMethodStateChanged(static_cast<bool>(type));
}

void RenderWidgetHostViewQt::ImeCancelComposition()
{
    qApp->inputMethod()->reset();
}

void RenderWidgetHostViewQt::ImeCompositionRangeChanged(const gfx::Range&, const std::vector<gfx::Rect>&)
{
    // FIXME: not implemented?
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::DidUpdateBackingStore(const gfx::Rect& scroll_rect, const gfx::Vector2d& scroll_delta, const std::vector<gfx::Rect>& copy_rects, const ui::LatencyInfo& /* latency_info */)
{
    if (!m_delegate->isVisible())
        return;

    Paint(scroll_rect);

    for (size_t i = 0; i < copy_rects.size(); ++i) {
        gfx::Rect rect = gfx::SubtractRects(copy_rects[i], scroll_rect);
        if (rect.IsEmpty())
            continue;
        Paint(rect);
    }
}

void RenderWidgetHostViewQt::RenderProcessGone(base::TerminationStatus, int)
{
    Destroy();
}

void RenderWidgetHostViewQt::Destroy()
{
    delete this;
}

void RenderWidgetHostViewQt::SetTooltipText(const base::string16 &tooltip_text)
{
    m_delegate->setTooltip(toQt(tooltip_text));
}

void RenderWidgetHostViewQt::SelectionBoundsChanged(const ViewHostMsg_SelectionBounds_Params &params)
{
    if (selection_range_.IsValid()) {
        if (params.is_anchor_first) {
            m_anchorPositionWithinSelection = selection_range_.GetMin() - selection_text_offset_;
            m_cursorPositionWithinSelection = selection_range_.GetMax() - selection_text_offset_;
        } else {
            m_anchorPositionWithinSelection = selection_range_.GetMax() - selection_text_offset_;
            m_cursorPositionWithinSelection = selection_range_.GetMin() - selection_text_offset_;
        }
    }

    gfx::Rect caretRect = gfx::UnionRects(params.anchor_rect, params.focus_rect);
    m_cursorRect = QRect(caretRect.x(), caretRect.y(), caretRect.width(), caretRect.height());
}

void RenderWidgetHostViewQt::ScrollOffsetChanged()
{
    // Not used.
}

void RenderWidgetHostViewQt::CopyFromCompositingSurface(const gfx::Rect& src_subrect, const gfx::Size& /* dst_size */, const base::Callback<void(bool, const SkBitmap&)>& callback)
{
    // Grab the snapshot from the renderer as that's the only reliable way to
    // readback from the GPU for this platform right now.
    // FIXME: is this true?
    GetRenderWidgetHost()->GetSnapshotFromRenderer(src_subrect, callback);
}

void RenderWidgetHostViewQt::CopyFromCompositingSurfaceToVideoFrame(const gfx::Rect& src_subrect, const scoped_refptr<media::VideoFrame>& target, const base::Callback<void(bool)>& callback)
{
    NOTIMPLEMENTED();
    callback.Run(false);
}

bool RenderWidgetHostViewQt::CanCopyToVideoFrame() const
{
    return false;
}

void RenderWidgetHostViewQt::OnAcceleratedCompositingStateChange()
{
    // bool activated = m_host->is_accelerated_compositing_active();
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::AcceleratedSurfaceInitialized(int host_id, int route_id)
{
}

void RenderWidgetHostViewQt::AcceleratedSurfaceBuffersSwapped(const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
    content::RenderWidgetHostImpl::CompositorFrameDrawn(params.latency_info);
}

void RenderWidgetHostViewQt::AcceleratedSurfacePostSubBuffer(const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
    content::RenderWidgetHostImpl::CompositorFrameDrawn(params.latency_info);
}

void RenderWidgetHostViewQt::AcceleratedSurfaceSuspend()
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::AcceleratedSurfaceRelease()
{
    QT_NOT_YET_IMPLEMENTED
}

bool RenderWidgetHostViewQt::HasAcceleratedSurface(const gfx::Size&)
{
    return false;
}

void RenderWidgetHostViewQt::OnSwapCompositorFrame(uint32 output_surface_id, scoped_ptr<cc::CompositorFrame> frame)
{
    Q_ASSERT(!m_needsDelegatedFrameAck);
    m_needsDelegatedFrameAck = true;
    m_pendingOutputSurfaceId = output_surface_id;
    Q_ASSERT(frame->delegated_frame_data);
    Q_ASSERT(!m_frameNodeData->frameData || m_frameNodeData->frameData->resource_list.empty());
    m_frameNodeData->frameData = frame->delegated_frame_data.Pass();
    m_frameNodeData->frameDevicePixelRatio = frame->metadata.device_scale_factor;

    // Support experimental.viewport.devicePixelRatio, see GetScreenInfo implementation below.
    float dpiScale = this->dpiScale();
    if (dpiScale != 0 && dpiScale != 1)
        m_frameNodeData->frameDevicePixelRatio /= dpiScale;

    m_delegate->update();

    if (m_didFirstVisuallyNonEmptyLayout) {
        m_adapterClient->loadVisuallyCommitted();
        m_didFirstVisuallyNonEmptyLayout = false;
    }
}

void RenderWidgetHostViewQt::GetScreenInfo(blink::WebScreenInfo* results)
{
    QWindow* window = m_delegate->window();
    if (!window)
        return;
    GetScreenInfoFromNativeWindow(window, results);

    // Support experimental.viewport.devicePixelRatio
    results->deviceScaleFactor *= dpiScale();
}

gfx::Rect RenderWidgetHostViewQt::GetBoundsInRootWindow()
{
    if (!m_delegate->window())
        return gfx::Rect();

    QRect r = m_delegate->window()->frameGeometry();
    return gfx::Rect(r.x(), r.y(), r.width(), r.height());
}

gfx::GLSurfaceHandle RenderWidgetHostViewQt::GetCompositingSurface()
{
    return gfx::GLSurfaceHandle(gfx::kNullPluginWindow, gfx::TEXTURE_TRANSPORT);
}

void RenderWidgetHostViewQt::SetHasHorizontalScrollbar(bool) { }

void RenderWidgetHostViewQt::SetScrollOffsetPinning(bool, bool) { }

void RenderWidgetHostViewQt::OnAccessibilityEvents(const std::vector<AccessibilityHostMsg_EventParams> &notifications)
{
    CreateBrowserAccessibilityManagerIfNeeded();
    GetBrowserAccessibilityManager()->OnAccessibilityEvents(notifications);
}

void RenderWidgetHostViewQt::SelectionChanged(const base::string16 &text, size_t offset, const gfx::Range &range)
{
    content::RenderWidgetHostViewBase::SelectionChanged(text, offset, range);
    m_adapterClient->selectionChanged();

#if defined(USE_X11) && !defined(OS_CHROMEOS)
    // Set the CLIPBOARD_TYPE_SELECTION to the ui::Clipboard.
    ui::ScopedClipboardWriter clipboard_writer(
                ui::Clipboard::GetForCurrentThread(),
                ui::CLIPBOARD_TYPE_SELECTION);
    clipboard_writer.WriteText(text);
#endif
}

bool RenderWidgetHostViewQt::CanDispatchToConsumer(ui::GestureConsumer *consumer)
{
    Q_ASSERT(static_cast<RenderWidgetHostViewQt*>(consumer) == this);
    return true;
}

void RenderWidgetHostViewQt::DispatchPostponedGestureEvent(ui::GestureEvent* event)
{
    ForwardGestureEventToRenderer(event);
}

void RenderWidgetHostViewQt::DispatchCancelTouchEvent(ui::TouchEvent *event)
{
    if (!m_host)
        return;

    blink::WebTouchEvent cancelEvent;
    cancelEvent.type = blink::WebInputEvent::TouchCancel;
    cancelEvent.timeStampSeconds = event->time_stamp().InSecondsF();
    m_host->ForwardTouchEventWithLatencyInfo(cancelEvent, *event->latency());
}

QSGNode *RenderWidgetHostViewQt::updatePaintNode(QSGNode *oldNode, QSGRenderContext *sgRenderContext)
{
    DelegatedFrameNode *frameNode = static_cast<DelegatedFrameNode *>(oldNode);
    if (!frameNode)
        frameNode = new DelegatedFrameNode(sgRenderContext);

    frameNode->commit(m_frameNodeData.data(), &m_resourcesToRelease);

    // This is possibly called from the Qt render thread, post the ack back to the UI
    // to tell the child compositors to release resources and trigger a new frame.
    if (m_needsDelegatedFrameAck) {
        m_needsDelegatedFrameAck = false;
        content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
            base::Bind(&RenderWidgetHostViewQt::sendDelegatedFrameAck, AsWeakPtr()));
    }

    return frameNode;
}

void RenderWidgetHostViewQt::notifyResize()
{
    GetRenderWidgetHost()->WasResized();
}

bool RenderWidgetHostViewQt::forwardEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        Focus(); // Fall through.
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        handleMouseEvent(static_cast<QMouseEvent*>(event));
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        handleKeyEvent(static_cast<QKeyEvent*>(event));
        break;
    case QEvent::Wheel:
        handleWheelEvent(static_cast<QWheelEvent*>(event));
        break;
    case QEvent::TouchBegin:
        Focus(); // Fall through.
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        handleTouchEvent(static_cast<QTouchEvent*>(event));
        break;
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
        handleHoverEvent(static_cast<QHoverEvent*>(event));
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        handleFocusEvent(static_cast<QFocusEvent*>(event));
        break;
    case QEvent::InputMethod:
        handleInputMethodEvent(static_cast<QInputMethodEvent*>(event));
        break;
    default:
        return false;
    }
    return true;
}

QVariant RenderWidgetHostViewQt::inputMethodQuery(Qt::InputMethodQuery query) const
{
    switch (query) {
    case Qt::ImEnabled:
        return QVariant(m_currentInputType != ui::TEXT_INPUT_TYPE_NONE);
    case Qt::ImCursorRectangle:
        return m_cursorRect;
    case Qt::ImFont:
        return QVariant();
    case Qt::ImCursorPosition:
        return static_cast<uint>(m_cursorPositionWithinSelection);
    case Qt::ImAnchorPosition:
        return static_cast<uint>(m_anchorPositionWithinSelection);
    case Qt::ImSurroundingText:
        return toQt(selection_text_);
    case Qt::ImCurrentSelection:
        return toQt(GetSelectedText());
    case Qt::ImMaximumTextLength:
        return QVariant(); // No limit.
    case Qt::ImHints:
        return int(toQtInputMethodHints(m_currentInputType));
    default:
        return QVariant();
    }
}

void RenderWidgetHostViewQt::windowChanged()
{
    if (m_delegate->window())
        m_host->NotifyScreenInfoChanged();
}

void RenderWidgetHostViewQt::ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo &touch, content::InputEventAckState ack_result) {
    ScopedVector<ui::TouchEvent> events;
    if (!content::MakeUITouchEventsFromWebTouchEvents(touch, &events, content::LOCAL_COORDINATES))
        return;

    ui::EventResult result = (ack_result == content::INPUT_EVENT_ACK_STATE_CONSUMED) ? ui::ER_HANDLED : ui::ER_UNHANDLED;
    for (ScopedVector<ui::TouchEvent>::iterator iter = events.begin(), end = events.end(); iter != end; ++iter)  {
        scoped_ptr<ui::GestureRecognizer::Gestures> gestures;
        gestures.reset(m_gestureRecognizer->ProcessTouchEventForGesture(*(*iter), result, this));
        ProcessGestures(gestures.get());
    }
}

void RenderWidgetHostViewQt::sendDelegatedFrameAck()
{
    cc::CompositorFrameAck ack;
    m_resourcesToRelease.swap(ack.resources);
    content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(
        m_host->GetRoutingID(), m_pendingOutputSurfaceId,
        m_host->GetProcess()->GetID(), ack);
}

void RenderWidgetHostViewQt::Paint(const gfx::Rect& damage_rect)
{
    Q_UNREACHABLE();
}

void RenderWidgetHostViewQt::ForwardGestureEventToRenderer(ui::GestureEvent* gesture)
{
    if ((gesture->type() == ui::ET_GESTURE_PINCH_BEGIN
       || gesture->type() == ui::ET_GESTURE_PINCH_UPDATE
       || gesture->type() == ui::ET_GESTURE_PINCH_END)
       && !shouldSendPinchGesture()
       ) {
        return;
    }

    blink::WebGestureEvent webGestureEvent = content::MakeWebGestureEventFromUIEvent(*gesture);

    if (webGestureEvent.type == blink::WebInputEvent::Undefined)
        return;

    if (webGestureEvent.type == blink::WebGestureEvent::GestureTapDown) {
        // Chromium does not stop a fling-scroll on tap-down.
        // So explicitly send an event to stop any in-progress flings.
        blink::WebGestureEvent flingCancel = webGestureEvent;
        flingCancel.type = blink::WebInputEvent::GestureFlingCancel;
        flingCancel.sourceDevice = blink::WebGestureEvent::Touchscreen;
        m_host->ForwardGestureEvent(flingCancel);
    }

    webGestureEvent.x = gesture->x();
    webGestureEvent.y = gesture->y();
    m_host->ForwardGestureEventWithLatencyInfo(webGestureEvent, *gesture->latency());
}

void RenderWidgetHostViewQt::ProcessGestures(ui::GestureRecognizer::Gestures *gestures)
{
    if (!gestures || gestures->empty())
        return;
    for (ui::GestureRecognizer::Gestures::iterator g_it = gestures->begin(); g_it != gestures->end(); ++g_it) {
        ForwardGestureEventToRenderer(*g_it);
    }
}

// Find (or create) a mapping to a 0-based ID.
int RenderWidgetHostViewQt::GetMappedTouch(int qtTouchId)
{
    QMap<int, int>::const_iterator it = m_touchIdMapping.find(qtTouchId);
    if (it != m_touchIdMapping.end())
        return it.value();
    int nextValue = 0;
    for (it = m_touchIdMapping.begin(); it != m_touchIdMapping.end(); ++it)
        nextValue = std::max(nextValue, it.value() + 1);
    m_touchIdMapping[qtTouchId] = nextValue;
    return nextValue;
}

void RenderWidgetHostViewQt::RemoveExpiredMappings(QTouchEvent *ev)
{
    QMap<int, int> newMap;
    for (QMap<int, int>::const_iterator it = m_touchIdMapping.begin(); it != m_touchIdMapping.end(); ++it) {
        Q_FOREACH (const QTouchEvent::TouchPoint& touchPoint, ev->touchPoints()) {
            if ((touchPoint.id() == it.key()) &&
                (touchPoint.state() != Qt::TouchPointReleased)) {
                newMap.insert(it.key(), it.value());
                break;
            }
        }
    }
    m_touchIdMapping.swap(newMap);
}

float RenderWidgetHostViewQt::dpiScale() const
{
    return m_adapterClient ? m_adapterClient->dpiScale() : 1.0;
}

bool RenderWidgetHostViewQt::IsPopup() const
{
    return popup_type_ != blink::WebPopupTypeNone;
}

void RenderWidgetHostViewQt::handleMouseEvent(QMouseEvent* event)
{
    blink::WebMouseEvent webEvent = WebEventFactory::toWebMouseEvent(event, dpiScale());
    if (event->type() == QMouseEvent::MouseButtonPress) {
        if (event->button() != m_clickHelper.lastPressButton
            || (event->timestamp() - m_clickHelper.lastPressTimestamp > static_cast<ulong>(qGuiApp->styleHints()->mouseDoubleClickInterval()))
            || (event->pos() - m_clickHelper.lastPressPosition).manhattanLength() > qGuiApp->styleHints()->startDragDistance())
            m_clickHelper.clickCounter = 0;

        m_clickHelper.lastPressTimestamp = event->timestamp();
        webEvent.clickCount = ++m_clickHelper.clickCounter;
        m_clickHelper.lastPressButton = event->button();
        m_clickHelper.lastPressPosition = QPointF(event->pos()).toPoint();
    }

    m_host->ForwardMouseEvent(webEvent);
}

void RenderWidgetHostViewQt::handleKeyEvent(QKeyEvent *ev)
{
    content::NativeWebKeyboardEvent webEvent = WebEventFactory::toWebKeyboardEvent(ev);
    m_host->ForwardKeyboardEvent(webEvent);
    if (webEvent.type == blink::WebInputEvent::RawKeyDown && !ev->text().isEmpty()) {
        webEvent.type = blink::WebInputEvent::Char;
        m_host->ForwardKeyboardEvent(webEvent);
    }
}

void RenderWidgetHostViewQt::handleInputMethodEvent(QInputMethodEvent *ev)
{
    if (!m_host)
        return;

    QString commitString = ev->commitString();
    QString preeditString = ev->preeditString();

    int replacementStart = ev->replacementStart();
    int replacementLength = ev->replacementLength();

    int cursorPositionInPreeditString = -1;
    gfx::Range selectionRange = gfx::Range::InvalidRange();

    const QList<QInputMethodEvent::Attribute> &attributes = ev->attributes();
    std::vector<blink::WebCompositionUnderline> underlines;

    Q_FOREACH (const QInputMethodEvent::Attribute &attribute, attributes) {
        switch (attribute.type) {
        case QInputMethodEvent::TextFormat: {
            if (preeditString.isEmpty())
                break;

            QTextCharFormat textCharFormat = attribute.value.value<QTextFormat>().toCharFormat();
            QColor qcolor = textCharFormat.underlineColor();
            blink::WebColor color = SkColorSetARGB(qcolor.alpha(), qcolor.red(), qcolor.green(), qcolor.blue());
            int start = qMin(attribute.start, (attribute.start + attribute.length));
            int end = qMax(attribute.start, (attribute.start + attribute.length));
            underlines.push_back(blink::WebCompositionUnderline(start, end, color, false));
            break;
        }
        case QInputMethodEvent::Cursor:
            if (attribute.length)
                cursorPositionInPreeditString = attribute.start;
            break;
        case QInputMethodEvent::Selection:
            selectionRange.set_start(qMin(attribute.start, (attribute.start + attribute.length)));
            selectionRange.set_end(qMax(attribute.start, (attribute.start + attribute.length)));
            break;
        default:
            break;
        }
    }

    if (preeditString.isEmpty()) {
        gfx::Range replacementRange = (replacementLength > 0) ? gfx::Range(replacementStart, replacementStart + replacementLength)
                                                              : gfx::Range::InvalidRange();
        m_host->ImeConfirmComposition(toString16(commitString), replacementRange, false);
    } else {
        if (!selectionRange.IsValid()) {
            // We did not receive a valid selection range, hence the range is going to mark the cursor position.
            int newCursorPosition = (cursorPositionInPreeditString < 0) ? preeditString.length() : cursorPositionInPreeditString;
            selectionRange.set_start(newCursorPosition);
            selectionRange.set_end(newCursorPosition);
        }
        m_host->ImeSetComposition(toString16(preeditString), underlines, selectionRange.start(), selectionRange.end());
    }
}

void RenderWidgetHostViewQt::SetAccessibilityFocus(int acc_obj_id)
{
    if (!m_host)
        return;
    m_host->AccessibilitySetFocus(acc_obj_id);
}

void RenderWidgetHostViewQt::AccessibilityDoDefaultAction(int acc_obj_id)
{
    if (!m_host)
      return;
    m_host->AccessibilityDoDefaultAction(acc_obj_id);
}

void RenderWidgetHostViewQt::AccessibilityScrollToMakeVisible(int acc_obj_id, gfx::Rect subfocus)
{
    if (!m_host)
        return;
    m_host->AccessibilityScrollToMakeVisible(acc_obj_id, subfocus);
}

void RenderWidgetHostViewQt::AccessibilityScrollToPoint(int acc_obj_id, gfx::Point point)
{
    if (!m_host)
      return;
    m_host->AccessibilityScrollToPoint(acc_obj_id, point);
}

void RenderWidgetHostViewQt::AccessibilitySetTextSelection(int acc_obj_id, int start_offset, int end_offset)
{
    if (!m_host)
        return;
    m_host->AccessibilitySetTextSelection(acc_obj_id, start_offset, end_offset);
}

gfx::Point RenderWidgetHostViewQt::GetLastTouchEventLocation() const
{
    QT_NOT_YET_IMPLEMENTED
    return gfx::Point();
}

void RenderWidgetHostViewQt::FatalAccessibilityTreeError()
{
    if (!m_host)
        return;
    m_host->FatalAccessibilityTreeError();
    SetBrowserAccessibilityManager(NULL);
}

void RenderWidgetHostViewQt::handleWheelEvent(QWheelEvent *ev)
{
    m_host->ForwardWheelEvent(WebEventFactory::toWebWheelEvent(ev, dpiScale()));
}

void RenderWidgetHostViewQt::handleTouchEvent(QTouchEvent *ev)
{
    // Chromium expects the touch event timestamps to be comparable to base::TimeTicks::Now().
    // Most importantly we also have to preserve the relative time distance between events.
    // Calculate a delta between event timestamps and Now() on the first received event, and
    // apply this delta to all successive events. This delta is most likely smaller than it
    // should by calculating it here but this will hopefully cause less than one frame of delay.
    base::TimeDelta eventTimestamp = base::TimeDelta::FromMilliseconds(ev->timestamp());
    if (m_eventsToNowDelta == base::TimeDelta())
        m_eventsToNowDelta = base::TimeTicks::Now() - base::TimeTicks() - eventTimestamp;
    eventTimestamp += m_eventsToNowDelta;

    // Convert each of our QTouchEvent::TouchPoint to the simpler ui::TouchEvent to
    // be able to use the same code path for both gesture recognition and WebTouchEvents.
    // It's a waste to do a double QTouchEvent -> ui::TouchEvent -> blink::WebTouchEvent
    // conversion but this should hopefully avoid a few bugs in the future.
    // FIXME: Carry Qt::TouchCancel from the event to each TouchPoint.
    Q_FOREACH (const QTouchEvent::TouchPoint& touchPoint, ev->touchPoints()) {
        // Stationary touch points are already in our accumulator.
        if (touchPoint.state() == Qt::TouchPointStationary)
            continue;

        ui::TouchEvent uiEvent(
            toUIEventType(touchPoint.state()),
            toGfxPoint((touchPoint.pos() / dpiScale()).toPoint()),
            0, // flags
            GetMappedTouch(touchPoint.id()),
            eventTimestamp,
            0, 0, // radius
            0, // angle
            touchPoint.pressure());

        blink::WebTouchPoint *point = content::UpdateWebTouchEventFromUIEvent(uiEvent, &m_accumTouchEvent);
        if (point) {
            if (m_host->ShouldForwardTouchEvent())
                // This will come back through ProcessAckedTouchEvent if the page didn't want it.
                m_host->ForwardTouchEventWithLatencyInfo(m_accumTouchEvent, ui::LatencyInfo());
            else {
                scoped_ptr<ui::GestureRecognizer::Gestures> gestures;
                gestures.reset(m_gestureRecognizer->ProcessTouchEventForGesture(uiEvent, ui::ER_UNHANDLED, this));
                ProcessGestures(gestures.get());
            }
            UpdateWebTouchEventAfterDispatch(&m_accumTouchEvent, point);
        }
    }
    RemoveExpiredMappings(ev);
}

void RenderWidgetHostViewQt::handleHoverEvent(QHoverEvent *ev)
{
    m_host->ForwardMouseEvent(WebEventFactory::toWebMouseEvent(ev, dpiScale()));
}

void RenderWidgetHostViewQt::handleFocusEvent(QFocusEvent *ev)
{
    if (ev->gotFocus()) {
        m_host->GotFocus();
        m_host->SetActive(true);
        ev->accept();
    } else if (ev->lostFocus()) {
        m_host->SetActive(false);
        m_host->Blur();
        ev->accept();
    }
}

QAccessibleInterface *RenderWidgetHostViewQt::GetQtAccessible()
{
    // Assume we have a screen reader doing stuff
    CreateBrowserAccessibilityManagerIfNeeded();
    content::BrowserAccessibilityState::GetInstance()->OnScreenReaderDetected();
    content::BrowserAccessibilityStateImpl::GetInstance()->SetAccessibilityMode(
                AccessibilityModeComplete);

    content::BrowserAccessibility *acc = GetBrowserAccessibilityManager()->GetRoot();
    content::BrowserAccessibilityQt *accQt = static_cast<content::BrowserAccessibilityQt*>(acc);
    return accQt;
}

void RenderWidgetHostViewQt::didFirstVisuallyNonEmptyLayout()
{
    m_didFirstVisuallyNonEmptyLayout = true;
}
