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

#include "backing_store_qt.h"
#include "chromium_overrides.h"
#include "delegated_frame_node.h"
#include "render_widget_host_view_qt_delegate.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_event_factory.h"

#include "cc/output/compositor_frame_ack.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/common/view_messages.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "third_party/WebKit/public/web/WebCursorInfo.h"
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

static void UpdateWebTouchEventAfterDispatch(WebKit::WebTouchEvent* event, WebKit::WebTouchPoint* point) {
    if (point->state != WebKit::WebTouchPoint::StateReleased &&
        point->state != WebKit::WebTouchPoint::StateCancelled)
        return;
    --event->touchesLength;
    for (unsigned i = point - event->touches; i < event->touchesLength; ++i) {
        event->touches[i] = event->touches[i + 1];
    }
}

RenderWidgetHostViewQt::RenderWidgetHostViewQt(content::RenderWidgetHost* widget)
    : m_host(content::RenderWidgetHostImpl::From(widget))
    , m_gestureRecognizer(ui::GestureRecognizer::Create(this))
    , m_backingStore(0)
    , m_frameNodeData(new DelegatedFrameNodeData)
    , m_needsDelegatedFrameAck(false)
    , m_adapterClient(0)
    , m_anchorPositionWithinSelection(0)
    , m_cursorPositionWithinSelection(0)
    , m_initPending(false)
{
    m_host->SetView(this);
}

RenderWidgetHostViewQt::~RenderWidgetHostViewQt()
{
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

BackingStoreQt* RenderWidgetHostViewQt::GetBackingStore()
{
    bool force_create = !m_host->empty();
    return static_cast<BackingStoreQt*>(m_host->GetBackingStore(force_create));
}

content::BackingStore *RenderWidgetHostViewQt::AllocBackingStore(const gfx::Size &size)
{
    Q_ASSERT(m_delegate);
    return new BackingStoreQt(m_host, size, m_delegate->window());
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
    m_delegate->initAsPopup(toQt(rect));
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
         m_delegate->move(QPoint(rect.x(), rect.y()));
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
    // Since we manage the view hierarchy in Qt we can possibly
    // avoid calls to this.
    QT_NOT_USED
    return gfx::NativeView();
}

gfx::NativeViewId RenderWidgetHostViewQt::GetNativeViewId() const
{
    return 0;
}

gfx::NativeViewAccessible RenderWidgetHostViewQt::GetNativeViewAccessible()
{
    // We are not using accessibility features at this point.
    QT_NOT_USED
    return NULL;
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
    QRectF p = m_delegate->screenRect();
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

// FIXME: remove TOOLKIT_GTK related things.
#if defined(TOOLKIT_GTK)
// Gets the event for the last mouse down.
GdkEventButton* RenderWidgetHostViewQt::GetLastMouseDown()
{
    return 0;
}

gfx::NativeView RenderWidgetHostViewQt::BuildInputMethodsGtkMenu()
{
    return 0;
}
#endif  // defined(TOOLKIT_GTK)

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
    case WebKit::WebCursorInfo::TypePointer:
        shape = Qt::ArrowCursor;
        break;
    case WebKit::WebCursorInfo::TypeCross:
        shape = Qt::CrossCursor;
        break;
    case WebKit::WebCursorInfo::TypeHand:
        shape = Qt::PointingHandCursor;
        break;
    case WebKit::WebCursorInfo::TypeIBeam:
        shape = Qt::IBeamCursor;
        break;
    case WebKit::WebCursorInfo::TypeWait:
        shape = Qt::WaitCursor;
        break;
    case WebKit::WebCursorInfo::TypeHelp:
        shape = Qt::WhatsThisCursor;
        break;
    case WebKit::WebCursorInfo::TypeEastResize:
    case WebKit::WebCursorInfo::TypeWestResize:
    case WebKit::WebCursorInfo::TypeEastWestResize:
    case WebKit::WebCursorInfo::TypeEastPanning:
    case WebKit::WebCursorInfo::TypeWestPanning:
        shape = Qt::SizeHorCursor;
        break;
    case WebKit::WebCursorInfo::TypeNorthResize:
    case WebKit::WebCursorInfo::TypeSouthResize:
    case WebKit::WebCursorInfo::TypeNorthSouthResize:
    case WebKit::WebCursorInfo::TypeNorthPanning:
    case WebKit::WebCursorInfo::TypeSouthPanning:
        shape = Qt::SizeVerCursor;
        break;
    case WebKit::WebCursorInfo::TypeNorthEastResize:
    case WebKit::WebCursorInfo::TypeSouthWestResize:
    case WebKit::WebCursorInfo::TypeNorthEastSouthWestResize:
    case WebKit::WebCursorInfo::TypeNorthEastPanning:
    case WebKit::WebCursorInfo::TypeSouthWestPanning:
        shape = Qt::SizeBDiagCursor;
        break;
    case WebKit::WebCursorInfo::TypeNorthWestResize:
    case WebKit::WebCursorInfo::TypeSouthEastResize:
    case WebKit::WebCursorInfo::TypeNorthWestSouthEastResize:
    case WebKit::WebCursorInfo::TypeNorthWestPanning:
    case WebKit::WebCursorInfo::TypeSouthEastPanning:
        shape = Qt::SizeFDiagCursor;
        break;
    case WebKit::WebCursorInfo::TypeColumnResize:
        shape = Qt::SplitHCursor;
        break;
    case WebKit::WebCursorInfo::TypeRowResize:
        shape = Qt::SplitVCursor;
        break;
    case WebKit::WebCursorInfo::TypeMiddlePanning:
    case WebKit::WebCursorInfo::TypeMove:
        shape = Qt::SizeAllCursor;
        break;
    case WebKit::WebCursorInfo::TypeVerticalText:
    case WebKit::WebCursorInfo::TypeCell:
    case WebKit::WebCursorInfo::TypeContextMenu:
    case WebKit::WebCursorInfo::TypeAlias:
    case WebKit::WebCursorInfo::TypeProgress:
    case WebKit::WebCursorInfo::TypeCopy:
    case WebKit::WebCursorInfo::TypeZoomIn:
    case WebKit::WebCursorInfo::TypeZoomOut:
        // FIXME: Load from the resource bundle.
        shape = Qt::ArrowCursor;
        break;
    case WebKit::WebCursorInfo::TypeNoDrop:
    case WebKit::WebCursorInfo::TypeNotAllowed:
        shape = Qt::ForbiddenCursor;
        break;
    case WebKit::WebCursorInfo::TypeNone:
        shape = Qt::BlankCursor;
        break;
    case WebKit::WebCursorInfo::TypeGrab:
        shape = Qt::OpenHandCursor;
        break;
    case WebKit::WebCursorInfo::TypeGrabbing:
        shape = Qt::ClosedHandCursor;
        break;
    case WebKit::WebCursorInfo::TypeCustom:
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

void RenderWidgetHostViewQt::SetTooltipText(const string16&)
{
    // QT_NOT_YET_IMPLEMENTED
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

void RenderWidgetHostViewQt::AcceleratedSurfaceBuffersSwapped(const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
}

void RenderWidgetHostViewQt::AcceleratedSurfacePostSubBuffer(const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
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
    m_delegate->update();
}

void RenderWidgetHostViewQt::GetScreenInfo(WebKit::WebScreenInfo* results)
{
    QWindow* window = m_delegate->window();
    if (!window)
        return;
    GetScreenInfoFromNativeWindow(window, results);
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
    if (!m_delegate->supportsHardwareAcceleration())
        return gfx::GLSurfaceHandle();
    return gfx::GLSurfaceHandle(gfx::kNullPluginWindow, gfx::TEXTURE_TRANSPORT);
}

void RenderWidgetHostViewQt::SetHasHorizontalScrollbar(bool) { }

void RenderWidgetHostViewQt::SetScrollOffsetPinning(bool, bool) { }

void RenderWidgetHostViewQt::OnAccessibilityEvents(const std::vector<AccessibilityHostMsg_EventParams>&)
{
    // We are not using accessibility features at this point.
    QT_NOT_USED
}

void RenderWidgetHostViewQt::SelectionChanged(const string16 &text, size_t offset, const gfx::Range &range)
{
    content::RenderWidgetHostViewBase::SelectionChanged(text, offset, range);
    m_adapterClient->selectionChanged();
}

bool RenderWidgetHostViewQt::DispatchLongPressGestureEvent(ui::GestureEvent *)
{
    return false;
}

bool RenderWidgetHostViewQt::DispatchCancelTouchEvent(ui::TouchEvent *)
{
    return false;
}

void RenderWidgetHostViewQt::paint(QPainter *painter, const QRectF& boundingRect)
{
    if (m_backingStore)
        m_backingStore->paintToTarget(painter, boundingRect);
}

QSGNode *RenderWidgetHostViewQt::updatePaintNode(QSGNode *oldNode, QQuickWindow *window)
{
    DelegatedFrameNode *frameNode = static_cast<DelegatedFrameNode *>(oldNode);
    if (!frameNode)
        frameNode = new DelegatedFrameNode(window);

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

void RenderWidgetHostViewQt::fetchBackingStore()
{
    m_backingStore = GetBackingStore();
}

void RenderWidgetHostViewQt::notifyResize()
{
    GetRenderWidgetHost()->WasResized();
}

bool RenderWidgetHostViewQt::forwardEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonDblClick:
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
        (*iter)->latency()->AddLatencyNumber(ui::INPUT_EVENT_LATENCY_ACKED_COMPONENT, static_cast<int64>(ack_result), 0);
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
    QRect r(damage_rect.x(), damage_rect.y(), damage_rect.width(), damage_rect.height());
    m_delegate->update(r);
}

void RenderWidgetHostViewQt::ProcessGestures(ui::GestureRecognizer::Gestures *gestures)
{
    if (!gestures || gestures->empty())
        return;
    for (ui::GestureRecognizer::Gestures::iterator g_it = gestures->begin(); g_it != gestures->end(); ++g_it) {
        const ui::GestureEvent &uiGestureEvent = **g_it;
        WebKit::WebGestureEvent webGestureEvent = content::MakeWebGestureEventFromUIEvent(uiGestureEvent);
        if (webGestureEvent.type != WebKit::WebInputEvent::Undefined) {
            webGestureEvent.x = uiGestureEvent.x();
            webGestureEvent.y = uiGestureEvent.y();
            m_host->ForwardGestureEvent(webGestureEvent);
        }
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
    return popup_type_ != WebKit::WebPopupTypeNone;
}

void RenderWidgetHostViewQt::handleMouseEvent(QMouseEvent* event)
{
    int eventType = event->type();
    if (eventType == QEvent::MouseButtonDblClick)
        return;

    WebKit::WebMouseEvent webEvent = WebEventFactory::toWebMouseEvent(event, dpiScale());
    if (eventType == QMouseEvent::MouseButtonPress) {
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
    m_host->ForwardKeyboardEvent(WebEventFactory::toWebKeyboardEvent(ev));
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
    std::vector<WebKit::WebCompositionUnderline> underlines;

    Q_FOREACH (const QInputMethodEvent::Attribute &attribute, attributes) {
        switch (attribute.type) {
        case QInputMethodEvent::TextFormat: {
            if (preeditString.isEmpty())
                break;

            QTextCharFormat textCharFormat = attribute.value.value<QTextFormat>().toCharFormat();
            QColor qcolor = textCharFormat.underlineColor();
            WebKit::WebColor color = SkColorSetARGB(qcolor.alpha(), qcolor.red(), qcolor.green(), qcolor.blue());
            int start = qMin(attribute.start, (attribute.start + attribute.length));
            int end = qMax(attribute.start, (attribute.start + attribute.length));
            underlines.push_back(WebKit::WebCompositionUnderline(start, end, color, false));
            break;
        }
        case QInputMethodEvent::Cursor:
            if (attribute.length)
                cursorPositionInPreeditString = attribute.start;
            break;
        case QInputMethodEvent::Selection:
            selectionRange.set_start(attribute.start);
            selectionRange.set_end(selectionRange.start() + attribute.length);
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

void RenderWidgetHostViewQt::handleWheelEvent(QWheelEvent *ev)
{
    m_host->ForwardWheelEvent(WebEventFactory::toWebWheelEvent(ev, dpiScale()));
}

void RenderWidgetHostViewQt::handleTouchEvent(QTouchEvent *ev)
{
    // Convert each of our QTouchEvent::TouchPoint to the simpler ui::TouchEvent to
    // be able to use the same code path for both gesture recognition and WebTouchEvents.
    // It's a waste to do a double QTouchEvent -> ui::TouchEvent -> WebKit::WebTouchEvent
    // conversion but this should hopefully avoid a few bugs in the future.
    // FIXME: Carry Qt::TouchCancel from the event to each TouchPoint.
    base::TimeDelta timestamp = base::TimeDelta::FromMilliseconds(ev->timestamp());
    Q_FOREACH (const QTouchEvent::TouchPoint& touchPoint, ev->touchPoints()) {
        // Stationary touch points are already in our accumulator.
        if (touchPoint.state() == Qt::TouchPointStationary)
            continue;

        ui::TouchEvent uiEvent(
            toUIEventType(touchPoint.state()),
            toGfxPoint((touchPoint.pos() / dpiScale()).toPoint()),
            0, // flags
            GetMappedTouch(touchPoint.id()),
            timestamp,
            0, 0, // radius
            0, // angle
            touchPoint.pressure());

        WebKit::WebTouchPoint *point = content::UpdateWebTouchEventFromUIEvent(uiEvent, &m_accumTouchEvent);
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
