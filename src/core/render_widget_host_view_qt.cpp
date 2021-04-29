
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

#include "render_widget_host_view_qt.h"

#include "browser_accessibility_manager_qt.h"
#include "common/qt_messages.h"
#include "qtwebenginecoreglobal_p.h"
#include "render_widget_host_view_qt_delegate.h"
#include "touch_handle_drawable_client.h"
#include "touch_selection_controller_client_qt.h"
#include "touch_selection_menu_controller.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"
#include "web_event_factory.h"

#include "base/threading/thread_task_runner_handle.h"
#include "components/viz/common/features.h"
#include "components/viz/common/frame_sinks/begin_frame_source.h"
#include "components/viz/common/surfaces/frame_sink_id_allocator.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "content/browser/compositor/image_transport_factory.h"
#include "content/browser/compositor/surface_utils.h"
#include "content/browser/renderer_host/display_util.h"
#include "content/browser/renderer_host/frame_tree.h"
#include "content/browser/renderer_host/frame_tree_node.h"
#include "content/browser/renderer_host/cursor_manager.h"
#include "content/browser/renderer_host/input/synthetic_gesture_target.h"
#include "content/browser/renderer_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_input_event_router.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/content_switches_internal.h"
#include "content/common/cursors/webcursor.h"
#include "content/common/input_messages.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/cursor/cursor.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/events/blink/blink_event_util.h"
#include "ui/events/event.h"
#include "ui/events/gesture_detection/gesture_configuration.h"
#include "ui/events/gesture_detection/gesture_provider_config_helper.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/events/keycodes/dom/dom_keyboard_layout_map.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/touch_selection/touch_selection_controller.h"

#if defined(USE_OZONE)
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#endif

#if defined(USE_AURA)
#include "ui/base/cursor/cursor_size.h"
#include "ui/base/cursor/cursors_aura.h"
#endif

#if defined(Q_OS_MACOS)
#include "content/app/resources/grit/content_resources.h"
#endif

#include <private/qguiapplication_p.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformintegration.h>
#include <QEvent>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QTextFormat>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QScopeGuard>
#include <QScreen>
#include <QStyleHints>
#include <QVariant>
#include <QWheelEvent>
#include <QWindow>
#include <QtGui/private/qinputcontrol_p.h>

namespace QtWebEngineCore {

enum ImStateFlags {
    TextInputStateUpdated = 1 << 0,
    TextSelectionUpdated = 1 << 1,
    TextSelectionBoundsUpdated = 1 << 2,
    TextSelectionFlags = TextSelectionUpdated | TextSelectionBoundsUpdated,
    AllFlags = TextInputStateUpdated | TextSelectionUpdated | TextSelectionBoundsUpdated
};

static inline ui::LatencyInfo CreateLatencyInfo(const blink::WebInputEvent& event) {
  ui::LatencyInfo latency_info;
  // The latency number should only be added if the timestamp is valid.
  if (!event.TimeStamp().is_null()) {
    latency_info.AddLatencyNumberWithTimestamp(
        ui::INPUT_EVENT_LATENCY_ORIGINAL_COMPONENT,
        event.TimeStamp());
  }
  return latency_info;
}

static inline Qt::InputMethodHints toQtInputMethodHints(ui::TextInputType inputType)
{
    switch (inputType) {
    case ui::TEXT_INPUT_TYPE_TEXT:
        return Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_SEARCH:
        return Qt::ImhPreferLowercase | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_PASSWORD:
        return Qt::ImhSensitiveData | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase | Qt::ImhHiddenText;
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

static inline int firstAvailableId(const QMap<int, int> &map)
{
    ui::BitSet32 usedIds;
    QMap<int, int>::const_iterator end = map.end();
    for (QMap<int, int>::const_iterator it = map.begin(); it != end; ++it)
        usedIds.mark_bit(it.value());
    return usedIds.first_unmarked_bit();
}

static inline ui::GestureProvider::Config QtGestureProviderConfig() {
    ui::GestureProvider::Config config = ui::GetGestureProviderConfig(ui::GestureProviderConfigType::CURRENT_PLATFORM);
    // Causes an assert in CreateWebGestureEventFromGestureEventData and we don't need them in Qt.
    config.gesture_begin_end_types_enabled = false;
    config.gesture_detector_config.swipe_enabled = false;
    config.gesture_detector_config.two_finger_tap_enabled = false;
    return config;
}

static inline bool isCommonTextEditShortcut(const QKeyEvent *ke)
{
    return QInputControl::isCommonTextEditShortcut(ke);
}

static uint32_t s_eventId = 0;
class MotionEventQt : public ui::MotionEvent {
public:
    MotionEventQt(const QList<QPair<int, QTouchEvent::TouchPoint>> &points, const base::TimeTicks &eventTime,
                  Action action, const Qt::KeyboardModifiers modifiers, int index = -1)
        : touchPoints(points)
        , eventTime(eventTime)
        , action(action)
        , eventId(++s_eventId)
        , flags(flagsFromModifiers(modifiers))
        , index(index)
    {
        // index is only valid for ACTION_DOWN and ACTION_UP and should correspond to the point causing it
        // see blink_event_util.cc:ToWebTouchPointState for details
        Q_ASSERT_X((action != Action::POINTER_DOWN && action != Action::POINTER_UP && index == -1)
                || (action == Action::POINTER_DOWN && index >= 0 && touchPoint(index).state() == Qt::TouchPointPressed)
                || (action == Action::POINTER_UP && index >= 0 && touchPoint(index).state() == Qt::TouchPointReleased),
                "MotionEventQt", qPrintable(QString("action: %1, index: %2, state: %3").arg(int(action)).arg(index).arg(touchPoint(index).state())));
    }

    uint32_t GetUniqueEventId() const override { return eventId; }
    Action GetAction() const override { return action; }
    int GetActionIndex() const override { return index; }
    size_t GetPointerCount() const override { return touchPoints.size(); }
    int GetPointerId(size_t pointer_index) const override { return touchPoints[pointer_index].first; }
    float GetX(size_t pointer_index) const override { return touchPoint(pointer_index).pos().x(); }
    float GetY(size_t pointer_index) const override { return touchPoint(pointer_index).pos().y(); }
    float GetRawX(size_t pointer_index) const override { return touchPoint(pointer_index).screenPos().x(); }
    float GetRawY(size_t pointer_index) const override { return touchPoint(pointer_index).screenPos().y(); }
    float GetTouchMajor(size_t pointer_index) const override
    {
        QSizeF diams = touchPoint(pointer_index).ellipseDiameters();
        return std::max(diams.height(), diams.width());
    }
    float GetTouchMinor(size_t pointer_index) const override
    {
        QSizeF diams = touchPoint(pointer_index).ellipseDiameters();
        return std::min(diams.height(), diams.width());
    }
    float GetOrientation(size_t pointer_index) const override
    {
        return 0;
    }
    int GetFlags() const override { return flags; }
    float GetPressure(size_t pointer_index) const override { return touchPoint(pointer_index).pressure(); }
    float GetTiltX(size_t pointer_index) const override { return 0; }
    float GetTiltY(size_t pointer_index) const override { return 0; }
    float GetTwist(size_t) const override { return 0; }
    float GetTangentialPressure(size_t) const override { return 0; }
    base::TimeTicks GetEventTime() const override { return eventTime; }

    size_t GetHistorySize() const override { return 0; }
    base::TimeTicks GetHistoricalEventTime(size_t historical_index) const override { return base::TimeTicks(); }
    float GetHistoricalTouchMajor(size_t pointer_index, size_t historical_index) const override { return 0; }
    float GetHistoricalX(size_t pointer_index, size_t historical_index) const override { return 0; }
    float GetHistoricalY(size_t pointer_index, size_t historical_index) const override { return 0; }
    ToolType GetToolType(size_t pointer_index) const override {
        bool isPen = touchPoint(pointer_index).flags() & QTouchEvent::TouchPoint::InfoFlag::Pen;
        return isPen ? ui::MotionEvent::ToolType::STYLUS : ui::MotionEvent::ToolType::FINGER;
    }
    int GetButtonState() const override { return 0; }

private:
    QList<QPair<int, QTouchEvent::TouchPoint>> touchPoints;
    base::TimeTicks eventTime;
    Action action;
    const uint32_t eventId;
    int flags;
    int index;
    const QTouchEvent::TouchPoint& touchPoint(size_t i) const { return touchPoints[i].second; }
};

extern display::Display toDisplayDisplay(int id, const QScreen *screen);

static blink::ScreenInfo screenInfoFromQScreen(QScreen *screen)
{
    blink::ScreenInfo r;
    if (!screen)
        screen = qApp->primaryScreen();
    if (screen)
        content::DisplayUtil::DisplayToScreenInfo(&r, toDisplayDisplay(0, screen));
    else
        r.device_scale_factor = qGuiApp->devicePixelRatio();
    return r;
}

// An minimal override to support progressing flings
class FlingingCompositor : public ui::Compositor
{
    RenderWidgetHostViewQt *m_rwhv;
public:
    FlingingCompositor(RenderWidgetHostViewQt *rwhv,
                       const viz::FrameSinkId &frame_sink_id,
                       ui::ContextFactory *context_factory,
                       scoped_refptr<base::SingleThreadTaskRunner> task_runner,
                       bool enable_pixel_canvas,
                       bool use_external_begin_frame_control = false,
                       bool force_software_compositor = false)
        : ui::Compositor(frame_sink_id, context_factory,
                         task_runner, enable_pixel_canvas,
                         use_external_begin_frame_control,
                         force_software_compositor)
        , m_rwhv(rwhv)
    {}

    void BeginMainFrame(const viz::BeginFrameArgs &args) override
    {
        if (args.type != viz::BeginFrameArgs::MISSED && !m_rwhv->is_currently_scrolling_viewport())
            m_rwhv->host()->ProgressFlingIfNeeded(args.frame_time);
        ui::Compositor::BeginMainFrame(args);
    }
};

class GuestInputEventObserverQt : public content::RenderWidgetHost::InputEventObserver
{
public:
    GuestInputEventObserverQt(RenderWidgetHostViewQt *rwhv)
        : m_rwhv(rwhv)
    {
    }
    ~GuestInputEventObserverQt() {}

    void OnInputEvent(const blink::WebInputEvent&) override {}
    void OnInputEventAck(blink::mojom::InputEventResultSource,
                         blink::mojom::InputEventResultState state,
                         const blink::WebInputEvent &event) override
    {
        if (event.GetType() == blink::WebInputEvent::Type::kMouseWheel)
            m_rwhv->WheelEventAck(static_cast<const blink::WebMouseWheelEvent &>(event), state);
    }

private:
    RenderWidgetHostViewQt *m_rwhv;
};

RenderWidgetHostViewQt::RenderWidgetHostViewQt(content::RenderWidgetHost *widget)
    : content::RenderWidgetHostViewBase::RenderWidgetHostViewBase(widget)
    , m_taskRunner(base::ThreadTaskRunnerHandle::Get())
    , m_gestureProvider(QtGestureProviderConfig(), this)
    , m_sendMotionActionDown(false)
    , m_touchMotionStarted(false)
    , m_guestInputEventObserver(new GuestInputEventObserverQt(this))
    , m_visible(false)
    , m_loadVisuallyCommittedState(NotCommitted)
    , m_adapterClient(0)
    , m_imeInProgress(false)
    , m_receivedEmptyImeEvent(false)
    , m_isMouseLocked(false)
    , m_imState(0)
    , m_anchorPositionWithinSelection(-1)
    , m_cursorPositionWithinSelection(-1)
    , m_cursorPosition(0)
    , m_emptyPreviousSelection(true)
    , m_wheelAckPending(false)
    , m_mouseWheelPhaseHandler(this)
    , m_frameSinkId(host()->GetFrameSinkId())
{
    if (GetTextInputManager())
        GetTextInputManager()->AddObserver(this);

    const QPlatformInputContext *context = QGuiApplicationPrivate::platformIntegration()->inputContext();
    m_imeHasHiddenTextCapability = context && context->hasCapability(QPlatformInputContext::HiddenTextCapability);

    m_rootLayer.reset(new ui::Layer(ui::LAYER_SOLID_COLOR));
    m_rootLayer->SetColor(SK_ColorTRANSPARENT);

    m_delegatedFrameHost.reset(new content::DelegatedFrameHost(
                                       host()->GetFrameSinkId(),
                                       &m_delegatedFrameHostClient,
                                       true /* should_register_frame_sink_id */));

    content::ImageTransportFactory *imageTransportFactory = content::ImageTransportFactory::GetInstance();
    ui::ContextFactory *contextFactory = imageTransportFactory->GetContextFactory();
    m_uiCompositor.reset(new FlingingCompositor(
                                 this,
                                 contextFactory->AllocateFrameSinkId(),
                                 contextFactory,
                                 m_taskRunner,
                                 false /* enable_pixel_canvas */));
    m_uiCompositor->SetAcceleratedWidget(gfx::kNullAcceleratedWidget); // null means offscreen
    m_uiCompositor->SetRootLayer(m_rootLayer.get());

    m_displayFrameSink = DisplayFrameSink::findOrCreate(m_uiCompositor->frame_sink_id());
    m_displayFrameSink->connect(this);

    if (host()->delegate() && host()->delegate()->GetInputEventRouter())
        host()->delegate()->GetInputEventRouter()->AddFrameSinkIdOwner(GetFrameSinkId(), this);

    m_cursorManager.reset(new content::CursorManager(this));

    m_touchSelectionControllerClient.reset(new TouchSelectionControllerClientQt(this));
    ui::TouchSelectionController::Config config;
    config.max_tap_duration = base::TimeDelta::FromMilliseconds(ui::GestureConfiguration::GetInstance()->long_press_time_in_ms());
    config.tap_slop = ui::GestureConfiguration::GetInstance()->max_touch_move_in_pixels_for_click();
    config.enable_longpress_drag_selection = false;
    m_touchSelectionController.reset(new ui::TouchSelectionController(m_touchSelectionControllerClient.get(), config));

    host()->render_frame_metadata_provider()->AddObserver(this);
    host()->render_frame_metadata_provider()->ReportAllFrameSubmissionsForTesting(true);

    host()->SetView(this);
}

RenderWidgetHostViewQt::~RenderWidgetHostViewQt()
{
    m_delegate.reset();

    QObject::disconnect(m_adapterClientDestroyedConnection);

    m_displayFrameSink->disconnect(this);

    if (text_input_manager_)
        text_input_manager_->RemoveObserver(this);

    m_touchSelectionController.reset();
    m_touchSelectionControllerClient.reset();

    host()->render_frame_metadata_provider()->RemoveObserver(this);
    host()->ViewDestroyed();
}

void RenderWidgetHostViewQt::setDelegate(RenderWidgetHostViewQtDelegate* delegate)
{
    m_delegate.reset(delegate);
    if (m_deferredShow) {
        m_deferredShow = false;
        Show();
    }
    visualPropertiesChanged();
}

void RenderWidgetHostViewQt::setAdapterClient(WebContentsAdapterClient *adapterClient)
{
    Q_ASSERT(!m_adapterClient);

    m_adapterClient = adapterClient;
    QObject::disconnect(m_adapterClientDestroyedConnection);
    m_adapterClientDestroyedConnection = QObject::connect(adapterClient->holdingQObject(),
                                                          &QObject::destroyed, [this] {
                                                            m_adapterClient = nullptr; });
}

void RenderWidgetHostViewQt::setGuest(content::RenderWidgetHostImpl *rwh)
{
    rwh->AddInputEventObserver(m_guestInputEventObserver.get());
}

void RenderWidgetHostViewQt::InitAsChild(gfx::NativeView)
{
}

void RenderWidgetHostViewQt::InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect& rect)
{
    m_delegate->initAsPopup(toQt(rect));
}

void RenderWidgetHostViewQt::InitAsFullscreen(content::RenderWidgetHostView*)
{
}

void RenderWidgetHostViewQt::SetSize(const gfx::Size &sizeInDips)
{
    if (!m_delegate)
        return;

    m_delegate->resize(sizeInDips.width(), sizeInDips.height());
}

void RenderWidgetHostViewQt::SetBounds(const gfx::Rect &windowRectInDips)
{
    DCHECK(IsPopup());
    m_delegate->move(toQt(windowRectInDips.origin()));
    m_delegate->resize(windowRectInDips.width(), windowRectInDips.height());
}

gfx::NativeView RenderWidgetHostViewQt::GetNativeView()
{
    // gfx::NativeView is a typedef to a platform specific view
    // pointer (HWND, NSView*, GtkWidget*) and other ports use
    // this function in the renderer_host layer when setting up
    // the view hierarchy and for generating snapshots in tests.
    // Since we manage the view hierarchy in Qt its value hasn't
    // been meaningful.
    return gfx::NativeView();
}

gfx::NativeViewAccessible RenderWidgetHostViewQt::GetNativeViewAccessible()
{
    return 0;
}

content::BrowserAccessibilityManager* RenderWidgetHostViewQt::CreateBrowserAccessibilityManager(content::BrowserAccessibilityDelegate* delegate, bool for_root_frame)
{
    Q_UNUSED(for_root_frame); // FIXME
#if QT_CONFIG(accessibility)
    return new content::BrowserAccessibilityManagerQt(
        m_adapterClient->accessibilityParentObject(),
        content::BrowserAccessibilityManagerQt::GetEmptyDocument(),
        delegate);
#else
    return 0;
#endif // QT_CONFIG(accessibility)
}

// Set focus to the associated View component.
void RenderWidgetHostViewQt::Focus()
{
    if (!IsPopup())
        m_delegate->setKeyboardFocus();
    host()->Focus();
}

bool RenderWidgetHostViewQt::HasFocus()
{
    return m_delegate->hasKeyboardFocus();
}

bool RenderWidgetHostViewQt::IsMouseLocked()
{
    return m_isMouseLocked;
}

viz::FrameSinkId RenderWidgetHostViewQt::GetRootFrameSinkId()
{
    return m_uiCompositor->frame_sink_id();
}

bool RenderWidgetHostViewQt::IsSurfaceAvailableForCopy()
{
    return m_delegatedFrameHost->CanCopyFromCompositingSurface();
}

void RenderWidgetHostViewQt::CopyFromSurface(const gfx::Rect &src_rect,
                                             const gfx::Size &output_size,
                                             base::OnceCallback<void(const SkBitmap &)> callback)
{
    m_delegatedFrameHost->CopyFromCompositingSurface(src_rect, output_size, std::move(callback));
}

void RenderWidgetHostViewQt::Show()
{
    if (m_delegate)
        m_delegate->show();
    else
        m_deferredShow = true;
}

void RenderWidgetHostViewQt::Hide()
{
    Q_ASSERT(m_delegate);
    m_delegate->hide();
}

bool RenderWidgetHostViewQt::IsShowing()
{
    Q_ASSERT(m_delegate);
    return m_delegate->isVisible();
}

// Retrieve the bounds of the View, in screen coordinates.
gfx::Rect RenderWidgetHostViewQt::GetViewBounds()
{
    return m_viewRectInDips;
}

void RenderWidgetHostViewQt::UpdateBackgroundColor()
{
    DCHECK(GetBackgroundColor());
    SkColor color = *GetBackgroundColor();

    m_delegate->setClearColor(toQt(color));

    bool opaque = SkColorGetA(color) == SK_AlphaOPAQUE;
    m_rootLayer->SetFillsBoundsOpaquely(opaque);
    m_rootLayer->SetColor(color);
    m_uiCompositor->SetBackgroundColor(color);

    content::RenderViewHost *rvh = content::RenderViewHost::From(host());
    if (color == SK_ColorTRANSPARENT)
        host()->owner_delegate()->SetBackgroundOpaque(false);
}

// Return value indicates whether the mouse is locked successfully or not.
blink::mojom::PointerLockResult RenderWidgetHostViewQt::LockMouse(bool request_unadjusted_movement)
{
    if (request_unadjusted_movement)
        return blink::mojom::PointerLockResult::kUnsupportedOptions;

    m_previousMousePosition = QCursor::pos();
    m_delegate->lockMouse();
    m_isMouseLocked = true;
    qApp->setOverrideCursor(Qt::BlankCursor);
    return blink::mojom::PointerLockResult::kSuccess;
}

blink::mojom::PointerLockResult RenderWidgetHostViewQt::ChangeMouseLock(bool request_unadjusted_movement)
{
    if (request_unadjusted_movement)
        return blink::mojom::PointerLockResult::kUnsupportedOptions;
    return blink::mojom::PointerLockResult::kSuccess;
}

void RenderWidgetHostViewQt::UnlockMouse()
{
    m_delegate->unlockMouse();
    qApp->restoreOverrideCursor();
    m_isMouseLocked = false;
    host()->LostMouseLock();
}

bool RenderWidgetHostViewQt::updateCursorFromResource(ui::mojom::CursorType type)
{
    int resourceId;
    // GetCursorDataFor only knows hotspots for 1x and 2x cursor images, in physical pixels.
    qreal hotspotDpr = m_screenInfo.device_scale_factor <= 1.0f ? 1.0f : 2.0f;
    qreal hotX;
    qreal hotY;

#if defined(USE_AURA)
    gfx::Point hotspot;
    if (!ui::GetCursorDataFor(ui::CursorSize::kNormal, type, hotspotDpr, &resourceId, &hotspot))
        return false;
    hotX = hotspot.x();
    hotY = hotspot.y();
#elif defined(Q_OS_MACOS)
    // See chromium/content/common/cursors/webcursor_mac.mm
    switch (type) {
    case ui::mojom::CursorType::kVerticalText:
        // TODO: [NSCursor IBeamCursorForVerticalLayout]
        return false;
    case ui::mojom::CursorType::kCell:
        resourceId = IDR_CELL_CURSOR;
        hotX = 7;
        hotY = 7;
        break;
    case ui::mojom::CursorType::kContextMenu:
        // TODO: [NSCursor contextualMenuCursor]
        return false;
    case ui::mojom::CursorType::kZoomIn:
        resourceId = IDR_ZOOMIN_CURSOR;
        hotX = 7;
        hotY = 7;
        break;
    case ui::mojom::CursorType::kZoomOut:
        resourceId = IDR_ZOOMOUT_CURSOR;
        hotX = 7;
        hotY = 7;
        break;
    default:
        Q_UNREACHABLE();
        return false;
    }
#else
    Q_UNREACHABLE();
    return false;
#endif

    const gfx::ImageSkia *imageSkia = ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(resourceId);
    if (!imageSkia)
        return false;

    QImage imageQt = toQImage(imageSkia->GetRepresentation(m_screenInfo.device_scale_factor));

    // Convert hotspot coordinates into device-independent pixels.
    hotX /= hotspotDpr;
    hotY /= hotspotDpr;

#if defined(Q_OS_LINUX)
    // QTBUG-68571: On Linux (xcb, wayland, eglfs), hotspot coordinates must be in physical pixels.
    qreal imageDpr = imageQt.devicePixelRatio();
    hotX *= imageDpr;
    hotY *= imageDpr;
#endif

    m_delegate->updateCursor(QCursor(QPixmap::fromImage(std::move(imageQt)), qRound(hotX), qRound(hotY)));
    return true;
}

void RenderWidgetHostViewQt::UpdateCursor(const content::WebCursor &webCursor)
{
    DisplayCursor(webCursor);
}

void RenderWidgetHostViewQt::DisplayCursor(const content::WebCursor &webCursor)
{
    const ui::Cursor &cursorInfo = webCursor.cursor();
    Qt::CursorShape shape = Qt::ArrowCursor;
    switch (cursorInfo.type()) {
    case ui::mojom::CursorType::kNull:
    case ui::mojom::CursorType::kPointer:
        shape = Qt::ArrowCursor;
        break;
    case ui::mojom::CursorType::kCross:
        shape = Qt::CrossCursor;
        break;
    case ui::mojom::CursorType::kHand:
        shape = Qt::PointingHandCursor;
        break;
    case ui::mojom::CursorType::kIBeam:
        shape = Qt::IBeamCursor;
        break;
    case ui::mojom::CursorType::kWait:
        shape = Qt::WaitCursor;
        break;
    case ui::mojom::CursorType::kHelp:
        shape = Qt::WhatsThisCursor;
        break;
    case ui::mojom::CursorType::kEastResize:
    case ui::mojom::CursorType::kWestResize:
    case ui::mojom::CursorType::kEastWestResize:
    case ui::mojom::CursorType::kEastPanning:
    case ui::mojom::CursorType::kWestPanning:
    case ui::mojom::CursorType::kMiddlePanningHorizontal:
        shape = Qt::SizeHorCursor;
        break;
    case ui::mojom::CursorType::kNorthResize:
    case ui::mojom::CursorType::kSouthResize:
    case ui::mojom::CursorType::kNorthSouthResize:
    case ui::mojom::CursorType::kNorthPanning:
    case ui::mojom::CursorType::kSouthPanning:
    case ui::mojom::CursorType::kMiddlePanningVertical:
        shape = Qt::SizeVerCursor;
        break;
    case ui::mojom::CursorType::kNorthEastResize:
    case ui::mojom::CursorType::kSouthWestResize:
    case ui::mojom::CursorType::kNorthEastSouthWestResize:
    case ui::mojom::CursorType::kNorthEastPanning:
    case ui::mojom::CursorType::kSouthWestPanning:
        shape = Qt::SizeBDiagCursor;
        break;
    case ui::mojom::CursorType::kNorthWestResize:
    case ui::mojom::CursorType::kSouthEastResize:
    case ui::mojom::CursorType::kNorthWestSouthEastResize:
    case ui::mojom::CursorType::kNorthWestPanning:
    case ui::mojom::CursorType::kSouthEastPanning:
        shape = Qt::SizeFDiagCursor;
        break;
    case ui::mojom::CursorType::kColumnResize:
        shape = Qt::SplitHCursor;
        break;
    case ui::mojom::CursorType::kRowResize:
        shape = Qt::SplitVCursor;
        break;
    case ui::mojom::CursorType::kMiddlePanning:
    case ui::mojom::CursorType::kMove:
        shape = Qt::SizeAllCursor;
        break;
    case ui::mojom::CursorType::kProgress:
        shape = Qt::BusyCursor;
        break;
    case ui::mojom::CursorType::kDndNone:
    case ui::mojom::CursorType::kDndMove:
        shape = Qt::DragMoveCursor;
        break;
    case ui::mojom::CursorType::kDndCopy:
    case ui::mojom::CursorType::kCopy:
        shape = Qt::DragCopyCursor;
        break;
    case ui::mojom::CursorType::kDndLink:
    case ui::mojom::CursorType::kAlias:
        shape = Qt::DragLinkCursor;
        break;
    case ui::mojom::CursorType::kVerticalText:
    case ui::mojom::CursorType::kCell:
    case ui::mojom::CursorType::kContextMenu:
    case ui::mojom::CursorType::kZoomIn:
    case ui::mojom::CursorType::kZoomOut:
        if (updateCursorFromResource(cursorInfo.type()))
            return;
        break;
    case ui::mojom::CursorType::kNoDrop:
    case ui::mojom::CursorType::kNotAllowed:
        shape = Qt::ForbiddenCursor;
        break;
    case ui::mojom::CursorType::kNone:
        shape = Qt::BlankCursor;
        break;
    case ui::mojom::CursorType::kGrab:
        shape = Qt::OpenHandCursor;
        break;
    case ui::mojom::CursorType::kGrabbing:
        shape = Qt::ClosedHandCursor;
        break;
    case ui::mojom::CursorType::kCustom:
        if (cursorInfo.custom_bitmap().colorType() == SkColorType::kN32_SkColorType) {
            QImage cursor = toQImage(cursorInfo.custom_bitmap(), QImage::Format_ARGB32);
            m_delegate->updateCursor(QCursor(QPixmap::fromImage(cursor), cursorInfo.custom_hotspot().x(), cursorInfo.custom_hotspot().y()));
            return;
        }
        break;
    }
    m_delegate->updateCursor(QCursor(shape));
}

content::CursorManager *RenderWidgetHostViewQt::GetCursorManager()
{
    return m_cursorManager.get();
}

void RenderWidgetHostViewQt::SetIsLoading(bool)
{
    // We use WebContentsDelegateQt::LoadingStateChanged to notify about loading state.
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

void RenderWidgetHostViewQt::RenderProcessGone()
{
    Destroy();
}

bool RenderWidgetHostViewQt::TransformPointToCoordSpaceForView(const gfx::PointF &point,
                                                               content::RenderWidgetHostViewBase *target_view,
                                                               gfx::PointF *transformed_point)
{
    if (target_view == this) {
        *transformed_point = point;
        return true;
    }

    return target_view->TransformPointToLocalCoordSpace(point, GetCurrentSurfaceId(), transformed_point);
}

void RenderWidgetHostViewQt::Destroy()
{
    delete this;
}

void RenderWidgetHostViewQt::SetTooltipText(const base::string16 &tooltip_text)
{
    DisplayTooltipText(tooltip_text);
}

void RenderWidgetHostViewQt::DisplayTooltipText(const base::string16 &tooltip_text)
{
    if (host()->delegate() && m_adapterClient)
        m_adapterClient->setToolTip(toQt(tooltip_text));
}

void RenderWidgetHostViewQt::GetScreenInfo(blink::ScreenInfo *results)
{
    *results = m_screenInfo;
}

gfx::Rect RenderWidgetHostViewQt::GetBoundsInRootWindow()
{
    return m_windowRectInDips;
}

void RenderWidgetHostViewQt::OnUpdateTextInputStateCalled(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view, bool did_update_state)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);
    Q_UNUSED(did_update_state);

    const ui::mojom::TextInputState *state = text_input_manager_->GetTextInputState();
    if (!state) {
        // Do not reset input method state here because an editable node might be still focused and
        // this would hide the virtual keyboard if a child of the focused node is removed.
        return;
    }

    ui::TextInputType type = getTextInputType();
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    m_delegate->setInputMethodHints(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu);
#else
    m_delegate->setInputMethodHints(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText);
#endif
    m_surroundingText = toQt(state->value);
    // Remove IME composition text from the surrounding text
    if (state->composition.has_value())
        m_surroundingText.remove(state->composition->start(), state->composition->end() - state->composition->start());

    // In case of text selection, the update is expected in RenderWidgetHostViewQt::selectionChanged().
    if (GetSelectedText().empty()) {
        // At this point it is unknown whether the text input state has been updated due to a text selection.
        // Keep the cursor position updated for cursor movements too.
        m_cursorPosition = state->selection.start();
        m_delegate->inputMethodStateChanged(type != ui::TEXT_INPUT_TYPE_NONE, type == ui::TEXT_INPUT_TYPE_PASSWORD);
    }

    if (m_imState & ImStateFlags::TextInputStateUpdated) {
        m_imState = ImStateFlags::TextInputStateUpdated;
        return;
    }

    // Ignore selection change triggered by ime composition unless it clears an actual text selection
    if (state->composition.has_value() && m_emptyPreviousSelection) {
        m_imState = 0;
        return;
    }

    m_imState |= ImStateFlags::TextInputStateUpdated;
    if (m_imState == ImStateFlags::AllFlags)
        selectionChanged();
}

void RenderWidgetHostViewQt::OnSelectionBoundsChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);

    m_imState |= ImStateFlags::TextSelectionBoundsUpdated;
    if (m_imState == ImStateFlags::AllFlags
            || (m_imState == ImStateFlags::TextSelectionFlags && getTextInputType() == ui::TEXT_INPUT_TYPE_NONE)) {
        selectionChanged();
    }
}

void RenderWidgetHostViewQt::OnTextSelectionChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);

    const content::TextInputManager::TextSelection *selection = GetTextInputManager()->GetTextSelection(updated_view);
    if (!selection)
        return;

#if defined(USE_OZONE)
    if (!selection->selected_text().empty() && selection->user_initiated()) {
        // Set the CLIPBOARD_TYPE_SELECTION to the ui::Clipboard.
        ui::ScopedClipboardWriter clipboard_writer(ui::ClipboardBuffer::kSelection);
        clipboard_writer.WriteText(selection->selected_text());
    }
#endif // defined(USE_OZONE)

    m_imState |= ImStateFlags::TextSelectionUpdated;
    if (m_imState == ImStateFlags::AllFlags
            || (m_imState == ImStateFlags::TextSelectionFlags && getTextInputType() == ui::TEXT_INPUT_TYPE_NONE)) {
        selectionChanged();
    }
}

void RenderWidgetHostViewQt::selectionChanged()
{
    // Reset input manager state
    m_imState = 0;
    ui::TextInputType type = getTextInputType();

    // Handle text selection out of an input field
    if (type == ui::TEXT_INPUT_TYPE_NONE) {
        if (GetSelectedText().empty() && m_emptyPreviousSelection)
            return;

        // Reset position values to emit selectionChanged signal when clearing text selection
        // by clicking into an input field. These values are intended to be used by inputMethodQuery
        // so they are not expected to be valid when selection is out of an input field.
        m_anchorPositionWithinSelection = -1;
        m_cursorPositionWithinSelection = -1;

        m_emptyPreviousSelection = GetSelectedText().empty();
        m_adapterClient->selectionChanged();
        return;
    }

    if (GetSelectedText().empty()) {
        // RenderWidgetHostViewQt::OnUpdateTextInputStateCalled() does not update the cursor position
        // if the selection is cleared because TextInputState changes before the TextSelection change.
        Q_ASSERT(text_input_manager_->GetTextInputState());
        m_cursorPosition = text_input_manager_->GetTextInputState()->selection.start();
        m_delegate->inputMethodStateChanged(true /*editorVisible*/, type == ui::TEXT_INPUT_TYPE_PASSWORD);

        m_anchorPositionWithinSelection = m_cursorPosition;
        m_cursorPositionWithinSelection = m_cursorPosition;

        if (!m_emptyPreviousSelection) {
            m_emptyPreviousSelection = true;
            m_adapterClient->selectionChanged();
        }

        return;
    }

    const content::TextInputManager::TextSelection *selection = text_input_manager_->GetTextSelection();
    if (!selection)
        return;

    if (!selection->range().IsValid())
        return;

    int newAnchorPositionWithinSelection = 0;
    int newCursorPositionWithinSelection = 0;

    if (text_input_manager_->GetSelectionRegion()->anchor.type() == gfx::SelectionBound::RIGHT) {
        newAnchorPositionWithinSelection = selection->range().GetMax() - selection->offset();
        newCursorPositionWithinSelection = selection->range().GetMin() - selection->offset();
    } else {
        newAnchorPositionWithinSelection = selection->range().GetMin() - selection->offset();
        newCursorPositionWithinSelection = selection->range().GetMax() - selection->offset();
    }

    if (m_anchorPositionWithinSelection == newAnchorPositionWithinSelection && m_cursorPositionWithinSelection == newCursorPositionWithinSelection)
        return;

    m_anchorPositionWithinSelection = newAnchorPositionWithinSelection;
    m_cursorPositionWithinSelection = newCursorPositionWithinSelection;

    if (!selection->selected_text().empty())
        m_cursorPosition = newCursorPositionWithinSelection;

    m_emptyPreviousSelection = selection->selected_text().empty();
    m_delegate->inputMethodStateChanged(true /*editorVisible*/, type == ui::TEXT_INPUT_TYPE_PASSWORD);
    m_adapterClient->selectionChanged();
}

void RenderWidgetHostViewQt::OnGestureEvent(const ui::GestureEventData& gesture)
{
    if ((gesture.type() == ui::ET_GESTURE_PINCH_BEGIN
         || gesture.type() == ui::ET_GESTURE_PINCH_UPDATE
         || gesture.type() == ui::ET_GESTURE_PINCH_END)
        && !content::IsPinchToZoomEnabled()) {
        return;
    }

    blink::WebGestureEvent event = ui::CreateWebGestureEventFromGestureEventData(gesture);

    if (m_touchSelectionController && m_touchSelectionControllerClient) {
        switch (event.GetType()) {
        case blink::WebInputEvent::Type::kGestureLongPress:
            m_touchSelectionController->HandleLongPressEvent(event.TimeStamp(), event.PositionInWidget());
            break;
        case blink::WebInputEvent::Type::kGestureTap:
            m_touchSelectionController->HandleTapEvent(event.PositionInWidget(), event.data.tap.tap_count);
            break;
        case blink::WebInputEvent::Type::kGestureScrollBegin:
            m_touchSelectionControllerClient->onScrollBegin();
            break;
        case blink::WebInputEvent::Type::kGestureScrollEnd:
            m_touchSelectionControllerClient->onScrollEnd();
            break;
        default:
            break;
        }
    }

    if (host()->delegate() && host()->delegate()->GetInputEventRouter())
        host()->delegate()->GetInputEventRouter()->RouteGestureEvent(this, &event, ui::LatencyInfo());
}

void RenderWidgetHostViewQt::DidStopFlinging()
{
    m_touchSelectionControllerClient->DidStopFlinging();
}

viz::ScopedSurfaceIdAllocator RenderWidgetHostViewQt::DidUpdateVisualProperties(const cc::RenderFrameMetadata &metadata)
{
    base::OnceCallback<void()> allocation_task =
        base::BindOnce(&RenderWidgetHostViewQt::OnDidUpdateVisualPropertiesComplete,
                       base::Unretained(this), metadata);
    return viz::ScopedSurfaceIdAllocator(&m_dfhLocalSurfaceIdAllocator, std::move(allocation_task));
}

void RenderWidgetHostViewQt::OnDidUpdateVisualPropertiesComplete(const cc::RenderFrameMetadata &metadata)
{
    synchronizeVisualProperties(metadata.local_surface_id);
}

void RenderWidgetHostViewQt::OnDidFirstVisuallyNonEmptyPaint()
{
    if (m_loadVisuallyCommittedState == NotCommitted) {
        m_loadVisuallyCommittedState = DidFirstVisuallyNonEmptyPaint;
    } else if (m_loadVisuallyCommittedState == DidFirstCompositorFrameSwap) {
        m_adapterClient->loadVisuallyCommitted();
        m_loadVisuallyCommittedState = NotCommitted;
    }
}

void RenderWidgetHostViewQt::scheduleUpdate()
{
    m_taskRunner->PostTask(
            FROM_HERE,
            base::BindOnce(&RenderWidgetHostViewQt::callUpdate, m_weakPtrFactory.GetWeakPtr()));
}

void RenderWidgetHostViewQt::callUpdate()
{
    m_delegate->update();

    if (m_loadVisuallyCommittedState == NotCommitted) {
        m_loadVisuallyCommittedState = DidFirstCompositorFrameSwap;
    } else if (m_loadVisuallyCommittedState == DidFirstVisuallyNonEmptyPaint) {
        m_adapterClient->loadVisuallyCommitted();
        m_loadVisuallyCommittedState = NotCommitted;
    }
}

QSGNode *RenderWidgetHostViewQt::updatePaintNode(QSGNode *oldNode)
{
    return m_displayFrameSink->updatePaintNode(oldNode, m_delegate.get());
}

void RenderWidgetHostViewQt::notifyShown()
{
    // Handle possible frame eviction:
    if (!m_dfhLocalSurfaceIdAllocator.HasValidLocalSurfaceId())
        m_dfhLocalSurfaceIdAllocator.GenerateId();
    if (m_visible)
        return;
    m_visible = true;

    host()->WasShown(nullptr);

    m_delegatedFrameHost->AttachToCompositor(m_uiCompositor.get());
    m_delegatedFrameHost->WasShown(GetLocalSurfaceId(),
                                   m_viewRectInDips.size(),
                                   nullptr);
}

void RenderWidgetHostViewQt::notifyHidden()
{
    if (!m_visible)
        return;
    m_visible = false;
    host()->WasHidden();
    m_delegatedFrameHost->WasHidden(content::DelegatedFrameHost::HiddenCause::kOther);
    m_delegatedFrameHost->DetachFromCompositor();
}

void RenderWidgetHostViewQt::visualPropertiesChanged()
{
    if (!m_delegate)
        return;

    gfx::Rect oldViewRect = m_viewRectInDips;
    m_viewRectInDips = toGfx(m_delegate->viewGeometry().toAlignedRect());

    gfx::Rect oldWindowRect = m_windowRectInDips;
    m_windowRectInDips = toGfx(m_delegate->windowGeometry());

    QWindow *window = m_delegate->window();
    blink::ScreenInfo oldScreenInfo = m_screenInfo;
    m_screenInfo = screenInfoFromQScreen(window ? window->screen() : nullptr);

    if (m_viewRectInDips != oldViewRect || m_windowRectInDips != oldWindowRect)
        host()->SendScreenRects();

    if (m_viewRectInDips.size() != oldViewRect.size() || m_screenInfo != oldScreenInfo)
        synchronizeVisualProperties(base::nullopt);
}

bool RenderWidgetHostViewQt::forwardEvent(QEvent *event)
{
    Q_ASSERT(host()->GetView());

    switch (event->type()) {
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        auto acceptKeyOutOfInputField = [](QKeyEvent *keyEvent) -> bool {
#ifdef Q_OS_MACOS
            // Check if a shortcut is registered for this key sequence.
            QKeySequence sequence = QKeySequence (
                         (keyEvent->modifiers() | keyEvent->key()) &
                         ~(Qt::KeypadModifier | Qt::GroupSwitchModifier));
            if (QGuiApplicationPrivate::instance()->shortcutMap.hasShortcutForKeySequence(sequence))
                return false;

            // The following shortcuts are handled out of input field too but
            // disabled on macOS to let the blinking menu handling to the
            // embedder application (see kKeyboardCodeKeyDownEntries in
            // third_party/WebKit/Source/core/editing/EditingBehavior.cpp).
            // Let them pass on macOS to generate the corresponding edit command.
            return keyEvent->matches(QKeySequence::Copy)
                    || keyEvent->matches(QKeySequence::Paste)
                    || keyEvent->matches(QKeySequence::Cut)
                    || keyEvent->matches(QKeySequence::SelectAll);
#else
            return false;
#endif
        };

        if (!inputMethodQuery(Qt::ImEnabled).toBool() && !(inputMethodQuery(Qt::ImHints).toInt() & Qt::ImhHiddenText) && !acceptKeyOutOfInputField(keyEvent))
            return false;

        Q_ASSERT(m_editCommand.empty());
        if (WebEventFactory::getEditCommand(keyEvent, &m_editCommand)
                || isCommonTextEditShortcut(keyEvent)) {
            event->accept();
            return true;
        }

        return false;
    }
    case QEvent::MouseButtonPress:
        Focus();
        Q_FALLTHROUGH();
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        // Skip second MouseMove event when a window is being adopted, so that Chromium
        // can properly handle further move events.
        // Also make sure the adapter client exists to prevent a null pointer dereference,
        // because it's possible for a QWebEnginePagePrivate (adapter) instance to be destroyed,
        // and then the OS (observed on Windows) might still send mouse move events to a still
        // existing popup RWHVQDW instance.
        if (m_adapterClient && m_adapterClient->isBeingAdopted())
            return false;
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
        Focus();
        Q_FALLTHROUGH();
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        handleTouchEvent(static_cast<QTouchEvent*>(event));
        break;
#if QT_CONFIG(tabletevent)
    case QEvent::TabletPress:
        Focus();
        Q_FALLTHROUGH();
    case QEvent::TabletRelease:
    case QEvent::TabletMove:
        handleTabletEvent(static_cast<QTabletEvent*>(event));
        break;
#endif
#ifndef QT_NO_GESTURES
    case QEvent::NativeGesture:
        handleGestureEvent(static_cast<QNativeGestureEvent *>(event));
        break;
#endif // QT_NO_GESTURES
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
    case QEvent::InputMethodQuery:
        handleInputMethodQueryEvent(static_cast<QInputMethodQueryEvent*>(event));
        break;
    case QEvent::Leave:
#ifdef Q_OS_WIN
        if (m_mouseButtonPressed > 0)
            return false;
#endif
    case QEvent::HoverLeave: {
            if (host()->delegate() && host()->delegate()->GetInputEventRouter()) {
                auto webEvent = WebEventFactory::toWebMouseEvent(event);
                host()->delegate()->GetInputEventRouter()->RouteMouseEvent(this, &webEvent, ui::LatencyInfo());
            }
        }
        break;
    default:
        return false;
    }
    return true;
}

QVariant RenderWidgetHostViewQt::inputMethodQuery(Qt::InputMethodQuery query)
{
    switch (query) {
    case Qt::ImEnabled: {
        ui::TextInputType type = getTextInputType();
        bool editorVisible = type != ui::TEXT_INPUT_TYPE_NONE;
        // IME manager should disable composition on input fields with ImhHiddenText hint if supported
        if (m_imeHasHiddenTextCapability)
            return QVariant(editorVisible);

        bool passwordInput = type == ui::TEXT_INPUT_TYPE_PASSWORD;
        return QVariant(editorVisible && !passwordInput);
    }
    case Qt::ImFont:
        // TODO: Implement this
        return QVariant();
    case Qt::ImCursorRectangle: {
        if (text_input_manager_) {
            if (auto *region = text_input_manager_->GetSelectionRegion()) {
                if (region->focus.GetHeight() > 0) {
                    gfx::Rect caretRect = gfx::RectBetweenSelectionBounds(region->anchor, region->focus);
                    if (caretRect.width() == 0)
                        caretRect.set_width(1); // IME API on Windows expects a width > 0
                    return toQt(caretRect);
                }
            }
        }
        return QVariant();
    }
    case Qt::ImCursorPosition:
        return m_cursorPosition;
    case Qt::ImAnchorPosition:
        return GetSelectedText().empty() ? m_cursorPosition : m_anchorPositionWithinSelection;
    case Qt::ImSurroundingText:
        return m_surroundingText;
    case Qt::ImCurrentSelection:
        return toQt(GetSelectedText());
    case Qt::ImMaximumTextLength:
        // TODO: Implement this
        return QVariant(); // No limit.
    case Qt::ImHints:
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        return int(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu);
#else
        return int(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText);
#endif
    default:
        return QVariant();
    }
}

void RenderWidgetHostViewQt::closePopup()
{
    // We notify the popup to be closed by telling it that it lost focus. WebKit does the rest
    // (hiding the widget and automatic memory cleanup via
    // RenderWidget::CloseWidgetSoon() -> RenderWidgetHostImpl::ShutdownAndDestroyWidget(true).
    host()->SetActive(false);
    host()->LostFocus();
}

void RenderWidgetHostViewQt::ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo &touch, blink::mojom::InputEventResultState ack_result)
{
    Q_UNUSED(touch);
    const bool eventConsumed = ack_result == blink::mojom::InputEventResultState::kConsumed;
    const bool isSetNonBlocking = content::InputEventResultStateIsSetNonBlocking(ack_result);
    m_gestureProvider.OnTouchEventAck(touch.event.unique_touch_event_id, eventConsumed, isSetNonBlocking);
}

void RenderWidgetHostViewQt::processMotionEvent(const ui::MotionEvent &motionEvent)
{
    auto result = m_gestureProvider.OnTouchEvent(motionEvent);
    if (!result.succeeded)
        return;
    blink::WebTouchEvent touchEvent = ui::CreateWebTouchEventFromMotionEvent(motionEvent,
                                                                             result.moved_beyond_slop_region,
                                                                             false /*hovering, FIXME ?*/);
    if (host()->delegate() && host()->delegate()->GetInputEventRouter())
        host()->delegate()->GetInputEventRouter()->RouteTouchEvent(this, &touchEvent, CreateLatencyInfo(touchEvent));
}

QList<RenderWidgetHostViewQt::TouchPoint> RenderWidgetHostViewQt::mapTouchPoints(const QList<QTouchEvent::TouchPoint> &input)
{
    QList<TouchPoint> output;
    for (int i = 0; i < input.size(); ++i) {
        const QTouchEvent::TouchPoint &point = input[i];

        int qtId = point.id();
        QMap<int, int>::const_iterator it = m_touchIdMapping.find(qtId);
        if (it == m_touchIdMapping.end())
            it = m_touchIdMapping.insert(qtId, firstAvailableId(m_touchIdMapping));

        output.append(qMakePair(it.value(), point));
    }

    Q_ASSERT(output.size() == std::accumulate(output.cbegin(), output.cend(), QSet<int>(),
                 [] (QSet<int> s, const TouchPoint &p) { s.insert(p.second.id()); return s; }).size());

    for (auto &&point : qAsConst(input))
        if (point.state() == Qt::TouchPointReleased)
            m_touchIdMapping.remove(point.id());

    return output;
}

bool RenderWidgetHostViewQt::IsPopup() const
{
    return widget_type_ == content::WidgetType::kPopup;
}

void RenderWidgetHostViewQt::handleMouseEvent(QMouseEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
        m_mouseButtonPressed++;
    if (event->type() == QEvent::MouseButtonRelease)
        m_mouseButtonPressed--;

    // Don't forward mouse events synthesized by the system, which are caused by genuine touch
    // events. Chromium would then process for e.g. a mouse click handler twice, once due to the
    // system synthesized mouse event, and another time due to a touch-to-gesture-to-mouse
    // transformation done by Chromium.
    if (event->source() == Qt::MouseEventSynthesizedBySystem)
        return;
    handlePointerEvent<QMouseEvent>(event);
}

void RenderWidgetHostViewQt::handleKeyEvent(QKeyEvent *ev)
{
    if (IsMouseLocked() && ev->key() == Qt::Key_Escape && ev->type() == QEvent::KeyRelease)
        UnlockMouse();

    if (m_receivedEmptyImeEvent) {
        // IME composition was not finished with a valid commit string.
        // We're getting the composition result in a key event.
        if (ev->key() != 0) {
            // The key event is not a result of an IME composition. Cancel IME.
            host()->ImeCancelComposition();
            m_receivedEmptyImeEvent = false;
        } else {
            if (ev->type() == QEvent::KeyRelease) {
                host()->ImeCommitText(toString16(ev->text()),
                                      std::vector<ui::ImeTextSpan>(),
                                      gfx::Range::InvalidRange(),
                                      0);
                m_receivedEmptyImeEvent = false;
                m_imeInProgress = false;
            }
            return;
        }
    }

    // Ignore autorepeating KeyRelease events so that the generated web events
    // conform to the spec, which requires autorepeat to result in a sequence of
    // keypress events and only one final keyup event:
    // https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent#Auto-repeat_handling
    // https://w3c.github.io/uievents/#dom-keyboardevent-repeat
    if (ev->type() == QEvent::KeyRelease && ev->isAutoRepeat())
        return;

    content::NativeWebKeyboardEvent webEvent = WebEventFactory::toWebKeyboardEvent(ev);
    if (webEvent.GetType() == blink::WebInputEvent::Type::kRawKeyDown && !m_editCommand.empty()) {
        ui::LatencyInfo latency;
        latency.set_source_event_type(ui::SourceEventType::KEY_PRESS);
        std::vector<blink::mojom::EditCommandPtr> commands;
        commands.emplace_back(blink::mojom::EditCommand::New(m_editCommand, ""));
        m_editCommand.clear();
        GetFocusedWidget()->ForwardKeyboardEventWithCommands(webEvent, latency, std::move(commands), nullptr);
        return;
    }

    bool keyDownTextInsertion = webEvent.GetType() == blink::WebInputEvent::Type::kRawKeyDown && webEvent.text[0];
    webEvent.skip_in_browser = keyDownTextInsertion;
    GetFocusedWidget()->ForwardKeyboardEvent(webEvent);

    if (keyDownTextInsertion) {
        // Blink won't consume the RawKeyDown, but rather the Char event in this case.
        // The RawKeyDown is skipped on the way back (see above).
        // The same os_event will be set on both NativeWebKeyboardEvents.
        webEvent.skip_in_browser = false;
        webEvent.SetType(blink::WebInputEvent::Type::kChar);
        GetFocusedWidget()->ForwardKeyboardEvent(webEvent);
    }
}

void RenderWidgetHostViewQt::handleInputMethodEvent(QInputMethodEvent *ev)
{
    // Reset input manager state
    m_imState = 0;

    if (!host())
        return;

    QString commitString = ev->commitString();
    QString preeditString = ev->preeditString();

    int cursorPositionInPreeditString = -1;
    gfx::Range selectionRange = gfx::Range::InvalidRange();

    const QList<QInputMethodEvent::Attribute> &attributes = ev->attributes();
    std::vector<ui::ImeTextSpan> underlines;
    bool hasSelection = false;

    for (const auto &attribute : attributes) {
        switch (attribute.type) {
        case QInputMethodEvent::TextFormat: {
            if (preeditString.isEmpty())
                break;

            int start = qMin(attribute.start, (attribute.start + attribute.length));
            int end = qMax(attribute.start, (attribute.start + attribute.length));

            // Blink does not support negative position values. Adjust start and end positions
            // to non-negative values.
            if (start < 0) {
                start = 0;
                end = qMax(0, start + end);
            }

            underlines.push_back(ui::ImeTextSpan(ui::ImeTextSpan::Type::kComposition, start, end,
                                                 ui::ImeTextSpan::Thickness::kThin, ui::ImeTextSpan::UnderlineStyle::kSolid,
                                                 SK_ColorTRANSPARENT));

            QTextCharFormat format = qvariant_cast<QTextFormat>(attribute.value).toCharFormat();
            if (format.underlineStyle() != QTextCharFormat::NoUnderline)
                underlines.back().underline_color = toSk(format.underlineColor());

            break;
        }
        case QInputMethodEvent::Cursor:
            // Always set the position of the cursor, even if it's marked invisible by Qt, otherwise
            // there is no way the user will know which part of the composition string will be
            // changed, when performing an IME-specific action (like selecting a different word
            // suggestion).
            cursorPositionInPreeditString = attribute.start;
            break;
        case QInputMethodEvent::Selection:
            hasSelection = true;

            // Cancel IME composition
            if (preeditString.isEmpty() && attribute.start + attribute.length == 0) {
                selectionRange.set_start(0);
                selectionRange.set_end(0);
                break;
            }

            selectionRange.set_start(qMin(attribute.start, (attribute.start + attribute.length)));
            selectionRange.set_end(qMax(attribute.start, (attribute.start + attribute.length)));
            break;
        default:
            break;
        }
    }

    if (!selectionRange.IsValid()) {
        // We did not receive a valid selection range, hence the range is going to mark the
        // cursor position.
        int newCursorPosition =
                (cursorPositionInPreeditString < 0) ? preeditString.length()
                                                    : cursorPositionInPreeditString;
        selectionRange.set_start(newCursorPosition);
        selectionRange.set_end(newCursorPosition);
    }

    if (hasSelection) {
        if (auto *frameWidgetInputHandler = getFrameWidgetInputHandler())
            frameWidgetInputHandler->SetEditableSelectionOffsets(selectionRange.start(), selectionRange.end());
    }

    int replacementLength = ev->replacementLength();
    gfx::Range replacementRange = gfx::Range::InvalidRange();

    if (replacementLength > 0)
    {
        int replacementStart = ev->replacementStart() < 0 ? m_cursorPosition + ev->replacementStart() : ev->replacementStart();
        if (replacementStart >= 0 && replacementStart < m_surroundingText.length())
            replacementRange = gfx::Range(replacementStart, replacementStart + replacementLength);
    }

    // There are so-far two known cases, when an empty QInputMethodEvent is received.
    // First one happens when backspace is used to remove the last character in the pre-edit
    // string, thus signaling the end of the composition.
    // The second one happens (on Windows) when a Korean char gets composed, but instead of
    // the event having a commit string, both strings are empty, and the actual char is received
    // as a QKeyEvent after the QInputMethodEvent is processed.
    // In lieu of the second case, we can't simply cancel the composition on an empty event,
    // and then add the Korean char when QKeyEvent is received, because that leads to text
    // flickering in the textarea (or any other element).
    // Instead we postpone the processing of the empty QInputMethodEvent by posting it
    // to the same focused object, and cancelling the composition on the next event loop tick.
    if (commitString.isEmpty() && preeditString.isEmpty() && replacementLength == 0) {
        if (!m_receivedEmptyImeEvent && m_imeInProgress && !hasSelection) {
            m_receivedEmptyImeEvent = true;
            QInputMethodEvent *eventCopy = new QInputMethodEvent(*ev);
            QGuiApplication::postEvent(qApp->focusObject(), eventCopy);
        } else {
            m_receivedEmptyImeEvent = false;
            if (m_imeInProgress) {
                m_imeInProgress = false;
                host()->ImeCancelComposition();
            }
        }

        return;
    }

    m_receivedEmptyImeEvent = false;

    // Finish compostion: insert or erase text.
    if (!commitString.isEmpty() || replacementLength > 0) {
        host()->ImeCommitText(toString16(commitString),
                              underlines,
                              replacementRange,
                              0);
        m_imeInProgress = false;
    }

    // Update or start new composition.
    // Be aware of that, we might get a commit string and a pre-edit string in a single event and
    // this means a new composition.
    if (!preeditString.isEmpty()) {
        host()->ImeSetComposition(toString16(preeditString),
                                  underlines,
                                  replacementRange,
                                  selectionRange.start(),
                                  selectionRange.end());
        m_imeInProgress = true;
    }
}

void RenderWidgetHostViewQt::handleInputMethodQueryEvent(QInputMethodQueryEvent *ev)
{
    Qt::InputMethodQueries queries = ev->queries();
    for (uint i = 0; i < 32; ++i) {
        Qt::InputMethodQuery query = (Qt::InputMethodQuery)(int)(queries & (1<<i));
        if (query) {
            QVariant v = inputMethodQuery(query);
            ev->setValue(query, v);
        }
    }
    ev->accept();
}

void RenderWidgetHostViewQt::handleWheelEvent(QWheelEvent *ev)
{
    if (!m_wheelAckPending) {
        Q_ASSERT(m_pendingWheelEvents.isEmpty());
        blink::WebMouseWheelEvent webEvent = WebEventFactory::toWebWheelEvent(ev);
        m_wheelAckPending = (webEvent.phase != blink::WebMouseWheelEvent::kPhaseEnded);
        m_mouseWheelPhaseHandler.AddPhaseIfNeededAndScheduleEndEvent(webEvent, true);
        if (host()->delegate() && host()->delegate()->GetInputEventRouter())
            host()->delegate()->GetInputEventRouter()->RouteMouseWheelEvent(this, &webEvent, ui::LatencyInfo());
        return;
    }
    if (!m_pendingWheelEvents.isEmpty()) {
        // Try to combine with this wheel event with the last pending one.
        if (WebEventFactory::coalesceWebWheelEvent(m_pendingWheelEvents.last(), ev))
            return;
    }
    m_pendingWheelEvents.append(WebEventFactory::toWebWheelEvent(ev));
}

void RenderWidgetHostViewQt::WheelEventAck(const blink::WebMouseWheelEvent &event, blink::mojom::InputEventResultState /*ack_result*/)
{
    if (event.phase == blink::WebMouseWheelEvent::kPhaseEnded)
        return;
    Q_ASSERT(m_wheelAckPending);
    m_wheelAckPending = false;
    while (!m_pendingWheelEvents.isEmpty() && !m_wheelAckPending) {
        blink::WebMouseWheelEvent webEvent = m_pendingWheelEvents.takeFirst();
        m_wheelAckPending = (webEvent.phase != blink::WebMouseWheelEvent::kPhaseEnded);
        m_mouseWheelPhaseHandler.AddPhaseIfNeededAndScheduleEndEvent(webEvent, true);
        if (host()->delegate() && host()->delegate()->GetInputEventRouter())
            host()->delegate()->GetInputEventRouter()->RouteMouseWheelEvent(this, &webEvent, ui::LatencyInfo());
    }
}

void RenderWidgetHostViewQt::GestureEventAck(const blink::WebGestureEvent &event, blink::mojom::InputEventResultState ack_result)
{
    // Forward unhandled scroll events back as wheel events
    if (event.GetType() != blink::WebInputEvent::Type::kGestureScrollUpdate)
        return;
    switch (ack_result) {
    case blink::mojom::InputEventResultState::kNotConsumed:
    case blink::mojom::InputEventResultState::kNoConsumerExists:
        WebEventFactory::sendUnhandledWheelEvent(event, delegate());
        break;
    default:
        break;
    }
}

content::MouseWheelPhaseHandler *RenderWidgetHostViewQt::GetMouseWheelPhaseHandler()
{
    return &m_mouseWheelPhaseHandler;
}

#ifndef QT_NO_GESTURES
void RenderWidgetHostViewQt::handleGestureEvent(QNativeGestureEvent *ev)
{
    const Qt::NativeGestureType type = ev->gestureType();
    // These are the only supported gestures by Chromium so far.
    if (type == Qt::ZoomNativeGesture || type == Qt::SmartZoomNativeGesture) {
        if (host()->delegate() && host()->delegate()->GetInputEventRouter()) {
            auto webEvent = WebEventFactory::toWebGestureEvent(ev);
            host()->delegate()->GetInputEventRouter()->RouteGestureEvent(this, &webEvent, ui::LatencyInfo());
        }
    }
}
#endif

void RenderWidgetHostViewQt::handleTouchEvent(QTouchEvent *ev)
{
    // On macOS instead of handling touch events, we use the OS provided QNativeGestureEvents.
#ifdef Q_OS_MACOS
    if (ev->spontaneous()) {
        return;
    } else {
        VLOG(1)
            << "Sending simulated touch events to Chromium does not work properly on macOS. "
               "Consider using QNativeGestureEvents or QMouseEvents.";
    }
#endif

    // Chromium expects the touch event timestamps to be comparable to base::TimeTicks::Now().
    // Most importantly we also have to preserve the relative time distance between events.
    // Calculate a delta between event timestamps and Now() on the first received event, and
    // apply this delta to all successive events. This delta is most likely smaller than it
    // should by calculating it here but this will hopefully cause less than one frame of delay.
    base::TimeTicks eventTimestamp = base::TimeTicks() + base::TimeDelta::FromMilliseconds(ev->timestamp());
    if (m_eventsToNowDelta == base::TimeDelta())
        m_eventsToNowDelta = base::TimeTicks::Now() - eventTimestamp;
    eventTimestamp += m_eventsToNowDelta;

    auto touchPoints = mapTouchPoints(ev->touchPoints());
    // Make sure that POINTER_DOWN action is delivered before MOVE, and MOVE before POINTER_UP
    std::sort(touchPoints.begin(), touchPoints.end(), [] (const TouchPoint &l, const TouchPoint &r) {
        return l.second.state() < r.second.state();
    });

    auto sc = qScopeGuard([&] () {
        switch (ev->type()) {
            case QEvent::TouchCancel:
                for (auto &&it : qAsConst(touchPoints))
                    m_touchIdMapping.remove(it.second.id());
                Q_FALLTHROUGH();

            case QEvent::TouchEnd:
                m_previousTouchPoints.clear();
                m_touchMotionStarted = false;
                break;

            default:
                m_previousTouchPoints = touchPoints;
                break;
        }
    });

    ui::MotionEvent::Action action;
    // Check first if the touch event should be routed to the selectionController
    if (!touchPoints.isEmpty()) {
        switch (touchPoints[0].second.state()) {
        case Qt::TouchPointPressed:
            action = ui::MotionEvent::Action::DOWN;
            break;
        case Qt::TouchPointMoved:
            action = ui::MotionEvent::Action::MOVE;
            break;
        case Qt::TouchPointReleased:
            action = ui::MotionEvent::Action::UP;
            break;
        default:
            action = ui::MotionEvent::Action::NONE;
            break;
        }
    } else {
        // An empty touchPoints always corresponds to a TouchCancel event.
        // We can't forward touch cancellations without a previously processed touch event,
        // as Chromium expects the previous touchPoints for Action::CANCEL.
        // If both are empty that means the TouchCancel was sent without an ongoing touch,
        // so there's nothing to cancel anyway.
        Q_ASSERT(ev->type() == QEvent::TouchCancel);
        touchPoints = m_previousTouchPoints;
        if (touchPoints.isEmpty())
            return;

        action = ui::MotionEvent::Action::CANCEL;
    }

    MotionEventQt me(touchPoints, eventTimestamp, action, ev->modifiers());
    if (m_touchSelectionController->WillHandleTouchEvent(me))
        return;

    switch (ev->type()) {
    case QEvent::TouchBegin:
        m_sendMotionActionDown = true;
        m_touchMotionStarted = true;
        m_touchSelectionControllerClient->onTouchDown();
        break;
    case QEvent::TouchUpdate:
        m_touchMotionStarted = true;
        break;
    case QEvent::TouchCancel:
    {
        // Only process TouchCancel events received following a TouchBegin or TouchUpdate event
        if (m_touchMotionStarted) {
            MotionEventQt cancelEvent(touchPoints, eventTimestamp, ui::MotionEvent::Action::CANCEL, ev->modifiers());
            processMotionEvent(cancelEvent);
        }

        return;
    }
    case QEvent::TouchEnd:
        m_touchSelectionControllerClient->onTouchUp();
        break;
    default:
        break;
    }

    if (m_imeInProgress && ev->type() == QEvent::TouchBegin) {
        m_imeInProgress = false;
        // Tell input method to commit the pre-edit string entered so far, and finish the
        // composition operation.
#ifdef Q_OS_WIN
        // Yes the function name is counter-intuitive, but commit isn't actually implemented
        // by the Windows QPA, and reset does exactly what is necessary in this case.
        qApp->inputMethod()->reset();
#else
        qApp->inputMethod()->commit();
#endif
    }

    // MEMO for the basis of this logic look into:
    //      * blink_event_util.cc:ToWebTouchPointState: which is used later to forward touch event
    //        composed from motion event after gesture recognition
    //      * gesture_detector.cc:GestureDetector::OnTouchEvent: contains logic for every motion
    //        event action and corresponding gesture recognition routines
    //      * input_router_imp.cc:InputRouterImp::SetMovementXYForTouchPoints: expectation about
    //        touch event content like number of points for different states

    int lastPressIndex = -1;
    while ((lastPressIndex + 1) < touchPoints.size() && touchPoints[lastPressIndex + 1].second.state() == Qt::TouchPointPressed)
        ++lastPressIndex;

    switch (ev->type()) {
        case QEvent::TouchBegin:
            processMotionEvent(MotionEventQt(touchPoints.mid(lastPressIndex),
                                             eventTimestamp, ui::MotionEvent::Action::DOWN, ev->modifiers()));
            --lastPressIndex;
            Q_FALLTHROUGH();

        case QEvent::TouchUpdate:
            for (; lastPressIndex >= 0; --lastPressIndex) {
                Q_ASSERT(touchPoints[lastPressIndex].second.state() == Qt::TouchPointPressed);
                MotionEventQt me(touchPoints.mid(lastPressIndex), eventTimestamp, ui::MotionEvent::Action::POINTER_DOWN, ev->modifiers(), 0);
                processMotionEvent(me);
            }

            if (ev->touchPointStates() & Qt::TouchPointMoved)
                processMotionEvent(MotionEventQt(touchPoints, eventTimestamp, ui::MotionEvent::Action::MOVE, ev->modifiers()));

            Q_FALLTHROUGH();

        case QEvent::TouchEnd:
            while (!touchPoints.isEmpty() && touchPoints.back().second.state() == Qt::TouchPointReleased) {
                auto action = touchPoints.size() > 1 ? ui::MotionEvent::Action::POINTER_UP : ui::MotionEvent::Action::UP;
                int index = action == ui::MotionEvent::Action::POINTER_UP ? touchPoints.size() - 1 : -1;
                processMotionEvent(MotionEventQt(touchPoints, eventTimestamp, action, ev->modifiers(), index));
                touchPoints.pop_back();
            }
            break;

        default:
            Q_ASSERT_X(false, __FUNCTION__, "Other event types are expected to be already handled.");
            break;
    }
}

#if QT_CONFIG(tabletevent)
void RenderWidgetHostViewQt::handleTabletEvent(QTabletEvent *event)
{
    handlePointerEvent<QTabletEvent>(event);
}
#endif

template<class T>
void RenderWidgetHostViewQt::handlePointerEvent(T *event)
{
    // Currently WebMouseEvent is a subclass of WebPointerProperties, so basically
    // tablet events are mouse events with extra properties.
    blink::WebMouseEvent webEvent = WebEventFactory::toWebMouseEvent(event);
    if ((webEvent.GetType() == blink::WebInputEvent::Type::kMouseDown || webEvent.GetType() == blink::WebInputEvent::Type::kMouseUp)
            && webEvent.button == blink::WebMouseEvent::Button::kNoButton) {
        // Blink can only handle the 5 main mouse-buttons and may assert when processing mouse-down for no button.
        LOG(INFO) << "Unhandled mouse button";
        return;
    }

    if (webEvent.GetType() == blink::WebInputEvent::Type::kMouseDown) {
        if (event->button() != m_clickHelper.lastPressButton
            || (event->timestamp() - m_clickHelper.lastPressTimestamp > static_cast<ulong>(qGuiApp->styleHints()->mouseDoubleClickInterval()))
            || (event->pos() - m_clickHelper.lastPressPosition).manhattanLength() > qGuiApp->styleHints()->startDragDistance()
            || m_clickHelper.clickCounter >= 3)
            m_clickHelper.clickCounter = 0;

        m_clickHelper.lastPressTimestamp = event->timestamp();
        webEvent.click_count = ++m_clickHelper.clickCounter;
        m_clickHelper.lastPressButton = event->button();
        m_clickHelper.lastPressPosition = QPointF(event->pos()).toPoint();
    }

    if (webEvent.GetType() == blink::WebInputEvent::Type::kMouseUp)
        webEvent.click_count = m_clickHelper.clickCounter;

    webEvent.movement_x = event->globalX() - m_previousMousePosition.x();
    webEvent.movement_y = event->globalY() - m_previousMousePosition.y();

    if (IsMouseLocked())
        QCursor::setPos(m_previousMousePosition);
    else
        m_previousMousePosition = event->globalPos();

    if (m_imeInProgress && webEvent.GetType() == blink::WebInputEvent::Type::kMouseDown) {
        m_imeInProgress = false;
        // Tell input method to commit the pre-edit string entered so far, and finish the
        // composition operation.
#ifdef Q_OS_WIN
        // Yes the function name is counter-intuitive, but commit isn't actually implemented
        // by the Windows QPA, and reset does exactly what is necessary in this case.
        qApp->inputMethod()->reset();
#else
        qApp->inputMethod()->commit();
#endif
    }

    if (host()->delegate() && host()->delegate()->GetInputEventRouter())
        host()->delegate()->GetInputEventRouter()->RouteMouseEvent(this, &webEvent, ui::LatencyInfo());
}

void RenderWidgetHostViewQt::handleHoverEvent(QHoverEvent *ev)
{
    if (host()->delegate() && host()->delegate()->GetInputEventRouter()) {
        auto webEvent = WebEventFactory::toWebMouseEvent(ev);
        host()->delegate()->GetInputEventRouter()->RouteMouseEvent(this, &webEvent, ui::LatencyInfo());
    }
}

void RenderWidgetHostViewQt::handleFocusEvent(QFocusEvent *ev)
{
    if (ev->gotFocus()) {
        host()->GotFocus();
        host()->SetActive(true);
        content::RenderViewHostImpl *viewHost = content::RenderViewHostImpl::From(host());
        Q_ASSERT(viewHost);
        if (ev->reason() == Qt::TabFocusReason)
            viewHost->SetInitialFocus(false);
        else if (ev->reason() == Qt::BacktabFocusReason)
            viewHost->SetInitialFocus(true);
        ev->accept();

        m_adapterClient->webContentsAdapter()->handlePendingMouseLockPermission();
    } else if (ev->lostFocus()) {
        host()->SetActive(false);
        host()->LostFocus();
        ev->accept();
    }
}

blink::mojom::FrameWidgetInputHandler *RenderWidgetHostViewQt::getFrameWidgetInputHandler()
{
    auto *focused_widget = GetFocusedWidget();
    if (!focused_widget)
        return nullptr;

    return focused_widget->GetFrameWidgetInputHandler();
}

ui::TextInputType RenderWidgetHostViewQt::getTextInputType() const
{
    if (text_input_manager_ && text_input_manager_->GetTextInputState())
        return text_input_manager_->GetTextInputState()->type;

    return ui::TEXT_INPUT_TYPE_NONE;
}

viz::SurfaceId RenderWidgetHostViewQt::GetCurrentSurfaceId() const
{
    return m_delegatedFrameHost->GetCurrentSurfaceId();
}

const viz::FrameSinkId &RenderWidgetHostViewQt::GetFrameSinkId() const
{
    return m_delegatedFrameHost->frame_sink_id();
}

const viz::LocalSurfaceId &RenderWidgetHostViewQt::GetLocalSurfaceId() const
{
    return m_dfhLocalSurfaceIdAllocator.GetCurrentLocalSurfaceId();
}

void RenderWidgetHostViewQt::FocusedNodeChanged(bool is_editable_node, const gfx::Rect& node_bounds_in_screen)
{
    Q_UNUSED(node_bounds_in_screen);
    if (!is_editable_node) {
        m_delegate->inputMethodStateChanged(false /*editorVisible*/, false /*passwordInput*/);
        m_delegate->setInputMethodHints(Qt::ImhNone);
    }
}

base::flat_map<std::string, std::string> RenderWidgetHostViewQt::GetKeyboardLayoutMap()
{
    return ui::GenerateDomKeyboardLayoutMap();
}

void RenderWidgetHostViewQt::TakeFallbackContentFrom(content::RenderWidgetHostView *view)
{
    DCHECK(!static_cast<RenderWidgetHostViewBase*>(view)->IsRenderWidgetHostViewChildFrame());
    RenderWidgetHostViewQt *viewQt = static_cast<RenderWidgetHostViewQt *>(view);
    base::Optional<SkColor> color = viewQt->GetBackgroundColor();
    if (color)
        SetBackgroundColor(*color);
    m_delegatedFrameHost->TakeFallbackContentFrom(viewQt->m_delegatedFrameHost.get());
    host()->GetContentRenderingTimeoutFrom(viewQt->host());
}

void RenderWidgetHostViewQt::EnsureSurfaceSynchronizedForWebTest()
{
    NOTIMPLEMENTED();
}

uint32_t RenderWidgetHostViewQt::GetCaptureSequenceNumber() const
{
    return 0;
}

void RenderWidgetHostViewQt::ResetFallbackToFirstNavigationSurface()
{
}

void RenderWidgetHostViewQt::OnRenderFrameMetadataChangedAfterActivation()
{
    const cc::RenderFrameMetadata &metadata = host()->render_frame_metadata_provider()->LastRenderFrameMetadata();
    if (metadata.selection.start != m_selectionStart || metadata.selection.end != m_selectionEnd) {
        m_selectionStart = metadata.selection.start;
        m_selectionEnd = metadata.selection.end;
        m_touchSelectionControllerClient->UpdateClientSelectionBounds(m_selectionStart, m_selectionEnd);
    }

    gfx::Vector2dF scrollOffset = metadata.root_scroll_offset.value_or(gfx::Vector2dF());
    gfx::SizeF contentsSize = metadata.root_layer_size;
    std::swap(m_lastScrollOffset, scrollOffset);
    std::swap(m_lastContentsSize, contentsSize);
    if (m_adapterClient && scrollOffset != m_lastScrollOffset)
        m_adapterClient->updateScrollPosition(toQt(m_lastScrollOffset));
    if (m_adapterClient && contentsSize != m_lastContentsSize)
        m_adapterClient->updateContentsSize(toQt(m_lastContentsSize));
}

void RenderWidgetHostViewQt::synchronizeVisualProperties(const base::Optional<viz::LocalSurfaceId> &childSurfaceId)
{
    if (childSurfaceId)
        m_dfhLocalSurfaceIdAllocator.UpdateFromChild(*childSurfaceId);
    else
        m_dfhLocalSurfaceIdAllocator.GenerateId();

    gfx::Size viewSizeInDips = GetRequestedRendererSize();
    gfx::Size viewSizeInPixels = GetCompositorViewportPixelSize();
    m_rootLayer->SetBounds(gfx::Rect(gfx::Point(), viewSizeInPixels));
    m_uiCompositorLocalSurfaceIdAllocator.GenerateId();
    m_uiCompositor->SetScaleAndSize(
            m_screenInfo.device_scale_factor,
            viewSizeInPixels,
            m_uiCompositorLocalSurfaceIdAllocator.GetCurrentLocalSurfaceId());
    m_delegatedFrameHost->EmbedSurface(
            m_dfhLocalSurfaceIdAllocator.GetCurrentLocalSurfaceId(),
            viewSizeInDips,
            cc::DeadlinePolicy::UseDefaultDeadline());

    host()->SynchronizeVisualProperties();
}

std::unique_ptr<content::SyntheticGestureTarget> RenderWidgetHostViewQt::CreateSyntheticGestureTarget()
{
    return nullptr;
}

ui::Compositor *RenderWidgetHostViewQt::GetCompositor()
{
    return m_uiCompositor.get();
}

} // namespace QtWebEngineCore
