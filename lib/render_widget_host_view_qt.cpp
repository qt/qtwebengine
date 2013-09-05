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
#include "render_widget_host_view_qt_delegate.h"
#include "web_event_factory.h"

#include "shared/shared_globals.h"

#include "cc/trees/layer_tree_host.h"
#include "cc/trees/thread_proxy.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/compositor_frame.h"
#include "cc/resources/resource_provider.h"
#include "cc/quads/draw_quad.h"
#include "cc/quads/texture_draw_quad.h"
#include "cc/quads/tile_draw_quad.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/gpu/gpu_messages.h"
#include "third_party/WebKit/public/web/WebCursorInfo.h"
#include "ui/base/events/event.h"
#include "ui/gfx/size_conversions.h"
#include "webkit/common/cursors/webcursor.h"
#include "content/browser/aura/image_transport_factory.h"
#include "ui/compositor/compositor.h"
#include "ui/gfx/size_conversions.h"
#include "base/memory/weak_ptr.h"

#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScreen>
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
    , m_adapterClient(0)
    , m_initPending(false)
{
    ui::Compositor::Initialize();

    m_host->SetView(this);
    // dummy for Xlib
    QWindow* dummy = new QWindow;
    dummy->setGeometry(0,0,100,100);
    dummy->create();

    m_compositorDelegate.reset(new CompositorDelegateQt);
    m_compositor.reset(new ui::Compositor(m_compositorDelegate.get(), dummy->winId()));
    m_compositor->SetScaleAndSize(1, gfx::Size(100,100)); // FIXME: This is certainly not correct!
    layer_ = new ui::Layer(ui::LAYER_NOT_DRAWN); // should we use this instead???
    layer_owner_.reset(layer_);
    layer_->set_delegate(this);
    m_compositorDelegate->setCompositor(m_compositor.get());
    m_compositor->SetRootLayer(layer());

    m_texturedLayer = new ui::Layer(ui::LAYER_TEXTURED);
    layer_->Add(m_texturedLayer);

    m_texturedLayer->SetBounds(gfx::Rect(0, 0, 800, 600));
    layer_->SetBounds(gfx::Rect(0, 0, 800, 600));
}

RenderWidgetHostViewQt::~RenderWidgetHostViewQt()
{
}

void RenderWidgetHostViewQt::setDelegate(RenderWidgetHostViewQtDelegate* delegate)
{
    m_delegate.reset(delegate);
    delegate->setView(this);
}

void RenderWidgetHostViewQt::setAdapterClient(WebContentsAdapterClient *adapterClient)
{
    Q_ASSERT(!m_adapterClient);

    m_adapterClient = adapterClient;
    if (m_initPending)
        InitAsChild(0);
}

bool RenderWidgetHostViewQt::handleEvent(QEvent* event) {

    switch(event->type()) {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
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
    default:
        return false;
    }
    return true;
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

void RenderWidgetHostViewQt::InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&)
{
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

    m_compositor->SetScaleAndSize(1, gfx::Size(width,height));

    m_delegate->resize(width,height);
}

void RenderWidgetHostViewQt::SetBounds(const gfx::Rect& rect)
{
    // This is called when webkit has sent us a Move message.
    // if (IsPopup())
        // m_delegate->setGeometry(rect.x(), rect.y(), rect.width(), rect.height());
    SetSize(rect.size());
}

gfx::Size RenderWidgetHostViewQt::GetPhysicalBackingSize() const
{

    return GetViewBounds().size();
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
    return m_delegate->nativeWindowIdForCompositor();
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
    m_delegate->setKeyboardFocus();
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
    return gfx::Rect(p.x(), p.y(), p.width(), p.height());
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
    if (m_delegate->isVisible())
        return;

    m_host->WasShown();
}

void RenderWidgetHostViewQt::WasHidden()
{
    if (!m_delegate->isVisible())
        return;

    m_host->WasHidden();
}

void RenderWidgetHostViewQt::MovePluginWindows(const gfx::Vector2d&, const std::vector<content::WebPluginGeometry>&)
{
    // QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::Blur()
{
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

void RenderWidgetHostViewQt::TextInputTypeChanged(ui::TextInputType, bool, ui::TextInputMode)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::ImeCancelComposition()
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::ImeCompositionRangeChanged(const ui::Range&, const std::vector<gfx::Rect>&)
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

void RenderWidgetHostViewQt::SelectionBoundsChanged(const ViewHostMsg_SelectionBounds_Params&)
{
    QT_NOT_YET_IMPLEMENTED
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

void RenderWidgetHostViewQt::GetScreenInfo(WebKit::WebScreenInfo* results)
{
    QWindow* window = m_delegate->window();
    if (!window)
        return;
    GetScreenInfoFromNativeWindow(window, results);
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
  if (shared_surface_handle_.is_null()) {
      content::ImageTransportFactory* factory = content::ImageTransportFactory::GetInstance();
    shared_surface_handle_ = factory->CreateSharedSurfaceHandle();
    factory->AddObserver(this);
  }
  return shared_surface_handle_;
}

void RenderWidgetHostViewQt::SetHasHorizontalScrollbar(bool) { }

void RenderWidgetHostViewQt::SetScrollOffsetPinning(bool, bool) { }

void RenderWidgetHostViewQt::OnAccessibilityNotifications(const std::vector<AccessibilityHostMsg_NotificationParams>&)
{
    // We are not using accessibility features at this point.
    QT_NOT_USED
}

bool RenderWidgetHostViewQt::DispatchLongPressGestureEvent(ui::GestureEvent *)
{
    return false;
}

bool RenderWidgetHostViewQt::DispatchCancelTouchEvent(ui::TouchEvent *)
{
    return false;
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

bool RenderWidgetHostViewQt::IsPopup() const
{
    return popup_type_ != WebKit::WebPopupTypeNone;
}

void RenderWidgetHostViewQt::handleMouseEvent(QMouseEvent* ev)
{
    m_host->ForwardMouseEvent(WebEventFactory::toWebMouseEvent(ev));
}

void RenderWidgetHostViewQt::handleKeyEvent(QKeyEvent *ev)
{
    m_host->ForwardKeyboardEvent(WebEventFactory::toWebKeyboardEvent(ev));
}

void RenderWidgetHostViewQt::handleWheelEvent(QWheelEvent *ev)
{
    m_host->ForwardWheelEvent(WebEventFactory::toWebWheelEvent(ev));
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
            toGfxPoint(touchPoint.pos().toPoint()),
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
    m_host->ForwardMouseEvent(WebEventFactory::toWebMouseEvent(ev));
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

void RenderWidgetHostViewQt::SwapDelegatedFrame(
    uint32 output_surface_id,
    scoped_ptr<cc::DelegatedFrameData> frame_data,
    float frame_device_scale_factor,
    const ui::LatencyInfo& latency_info) {
    gfx::Size frame_size_in_dip;
    if (!frame_data->render_pass_list.empty())
        frame_size_in_dip = gfx::ToFlooredSize(gfx::ScaleSize(frame_data->render_pass_list.back()->output_rect.size(), 1.f/frame_device_scale_factor));

    // if (ShouldSkipFrame(frame_size_in_dip)) {
    //     cc::CompositorFrameAck ack;
    //     ack.resources.swap(frame_data->resource_list);
    //     RenderWidgetHostImpl::SendSwapCompositorFrameAck(
    //         host_->GetRoutingID(), output_surface_id,
    //         host_->GetProcess()->GetID(), ack);
    //     return;
    // }

    m_texturedLayer->SetDelegatedFrame(frame_data.Pass(), frame_size_in_dip);
    // released_front_lock_ = NULL;
    // current_frame_size_ = frame_size_in_dip;
    // CheckResizeLock();

    // if (paint_observer_)
    //     paint_observer_->OnUpdateCompositorContent();

    if (!m_compositor) {
        fprintf(stderr, "    - Directly sending DelegatedFrameAck\n");
        SendDelegatedFrameAck(output_surface_id);
    } else {
        m_compositor->SetLatencyInfo(latency_info);
        fprintf(stderr, "    - Registering SendDelegatedFrameAck callback\n");
        AddOnCommitCallbackAndDisableLocks(base::Bind(&RenderWidgetHostViewQt::SendDelegatedFrameAck, AsWeakPtr(), output_surface_id));
    }
}

// Swap ack for the renderer when kCompositeToMailbox is enabled.
void SendCompositorFrameAck(
    int32 route_id,
    uint32 output_surface_id,
    int renderer_host_id,
    const gpu::Mailbox& received_mailbox,
    const gfx::Size& received_size,
    bool skip_frame,
    const scoped_refptr<ui::Texture>& texture_to_produce) {
  cc::CompositorFrameAck ack;
  ack.gl_frame_data.reset(new cc::GLFrameData());
  DCHECK(!texture_to_produce.get() || !skip_frame);
  if (texture_to_produce.get()) {
    std::string mailbox_name = texture_to_produce->Produce();
    std::copy(mailbox_name.data(),
              mailbox_name.data() + mailbox_name.length(),
              reinterpret_cast<char*>(ack.gl_frame_data->mailbox.name));
    ack.gl_frame_data->size = texture_to_produce->size();
    ack.gl_frame_data->sync_point =
        content::ImageTransportFactory::GetInstance()->InsertSyncPoint();
  } else if (skip_frame) {
    // Skip the frame, i.e. tell the producer to reuse the same buffer that
    // we just received.
    ack.gl_frame_data->size = received_size;
    ack.gl_frame_data->mailbox = received_mailbox;
  }

  content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(route_id, output_surface_id, renderer_host_id, ack);
}

void RenderWidgetHostViewQt::OnSwapCompositorFrame(uint32 output_surface_id, scoped_ptr<cc::CompositorFrame> frame)
{
    if (frame->delegated_frame_data) {
        fprintf(stderr, "******************SwapDelegatedFrame*********************\n");

        SwapDelegatedFrame(output_surface_id,
                           frame->delegated_frame_data.Pass(),
                           frame->metadata.device_scale_factor,
                           frame->metadata.latency_info);

        m_compositor->layer_tree_host()->SetNeedsCommit();
        return;
    }

    if (frame->software_frame_data) {
        fprintf(stderr, "***********THIS IS A SOFTWARE FRAME AND NEEDS TO BE HANDLED SEPERATELY***************\n");
    // SwapSoftwareFrame(output_surface_id,
    //                   frame->software_frame_data.Pass(),
    //                   frame->metadata.device_scale_factor,
    //                   frame->metadata.latency_info);
    //     return;
    }

    fprintf(stderr, "***********THIS IS A UNKNOWN FRAME AND NEEDS TO BE HANDLED SEPERATELY***************\n");
    if (!frame->gl_frame_data || frame->gl_frame_data->mailbox.IsZero())
        return;

    BufferPresentedCallback ack_callback = base::Bind(
        &SendCompositorFrameAck,
        m_host->GetRoutingID(), output_surface_id, m_host->GetProcess()->GetID(),
        frame->gl_frame_data->mailbox, frame->gl_frame_data->size);

    if (!frame->gl_frame_data->sync_point) {
        LOG(ERROR) << "CompositorFrame without sync point. Skipping frame...";
        ack_callback.Run(true, scoped_refptr<ui::Texture>());
        return;
    }

    content::ImageTransportFactory* factory = content::ImageTransportFactory::GetInstance();
    factory->WaitSyncPoint(frame->gl_frame_data->sync_point);

    // std::string mailbox_name(
    //     reinterpret_cast<const char*>(frame->gl_frame_data->mailbox.name),
    //     sizeof(frame->gl_frame_data->mailbox.name));
    // BuffersSwapped(frame->gl_frame_data->size,
    //              frame->gl_frame_data->sub_buffer_rect,
    //              frame->metadata.device_scale_factor,
    //              mailbox_name,
    //              frame->metadata.latency_info,
    //              ack_callback);

}

void RenderWidgetHostViewQt::RunOnCommitCallbacks() {
  for (std::vector<base::Closure>::const_iterator
      it = on_compositing_did_commit_callbacks_.begin();
      it != on_compositing_did_commit_callbacks_.end(); ++it) {
    it->Run();
  }
  on_compositing_did_commit_callbacks_.clear();
}

void RenderWidgetHostViewQt::SendDelegatedFrameAck(uint32 output_surface_id) {
  cc::CompositorFrameAck ack;
  layer()->TakeUnusedResourcesForChildCompositor(&ack.resources);
  content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(
      m_host->GetRoutingID(), output_surface_id,
      m_host->GetProcess()->GetID(), ack);
}

void RenderWidgetHostViewQt::AddOnCommitCallbackAndDisableLocks(const base::Closure& callback) {
  if (!m_compositor->HasObserver(this))
    m_compositor->AddObserver(this);

  // can_lock_compositor_ = NO_PENDING_COMMIT;
  on_compositing_did_commit_callbacks_.push_back(callback);
}

void RenderWidgetHostViewQt::OnCompositingDidCommit(ui::Compositor* compositor)
{
    fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
  // if (can_lock_compositor_ == NO_PENDING_COMMIT) {
  //   can_lock_compositor_ = YES;
  //   if (resize_lock_.get() && resize_lock_->GrabDeferredLock())
  //     can_lock_compositor_ = YES_DID_LOCK;
  // }
  RunOnCommitCallbacks();
  // if (resize_lock_ && resize_lock_->expected_size() == current_frame_size_) {
  //   resize_lock_.reset();
  //   host_->WasResized();
  //   // We may have had a resize while we had the lock (e.g. if the lock expired,
  //   // or if the UI still gave us some resizes), so make sure we grab a new lock
  //   // if necessary.
  //   MaybeCreateResizeLock();
  // }
}

