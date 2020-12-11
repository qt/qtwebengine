
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
#include "render_widget_host_view_qt_delegate_client.h"
#include "touch_selection_controller_client_qt.h"
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
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/input/synthetic_gesture_target.h"
#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_input_event_router.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/content_switches_internal.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/cursors/webcursor.h"
#include "content/common/input_messages.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/cursor/cursor.h"
#include "ui/events/blink/blink_event_util.h"
#include "ui/events/event.h"
#include "ui/events/gesture_detection/gesture_configuration.h"
#include "ui/events/gesture_detection/gesture_provider_config_helper.h"
#include "ui/gfx/image/image_skia.h"

#if defined(USE_OZONE)
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#endif

#if defined(USE_AURA)
#include "ui/base/cursor/cursor_size.h"
#include "ui/base/cursor/cursors_aura.h"
#include "ui/base/resource/resource_bundle.h"
#endif

#include <QGuiApplication>
#include <QPixmap>
#include <QScopeGuard>
#include <QScreen>
#include <QWindow>

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

static inline ui::GestureProvider::Config QtGestureProviderConfig() {
    ui::GestureProvider::Config config = ui::GetGestureProviderConfig(ui::GestureProviderConfigType::CURRENT_PLATFORM);
    // Causes an assert in CreateWebGestureEventFromGestureEventData and we don't need them in Qt.
    config.gesture_begin_end_types_enabled = false;
    config.gesture_detector_config.swipe_enabled = false;
    config.gesture_detector_config.two_finger_tap_enabled = false;
    return config;
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

RenderWidgetHostViewQt::RenderWidgetHostViewQt(content::RenderWidgetHost *widget)
    : content::RenderWidgetHostViewBase::RenderWidgetHostViewBase(widget)
    , m_taskRunner(base::ThreadTaskRunnerHandle::Get())
    , m_gestureProvider(QtGestureProviderConfig(), this)
    , m_frameSinkId(host()->GetFrameSinkId())
    , m_delegateClient(new RenderWidgetHostViewQtDelegateClient(this))
{
    if (GetTextInputManager())
        GetTextInputManager()->AddObserver(this);

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

    if (host()->delegate() && host()->delegate()->GetInputEventRouter())
        host()->delegate()->GetInputEventRouter()->AddFrameSinkIdOwner(GetFrameSinkId(), this);

    m_touchSelectionControllerClient.reset(new TouchSelectionControllerClientQt(this));
    ui::TouchSelectionController::Config config;
    config.max_tap_duration = base::TimeDelta::FromMilliseconds(ui::GestureConfiguration::GetInstance()->long_press_time_in_ms());
    config.tap_slop = ui::GestureConfiguration::GetInstance()->max_touch_move_in_pixels_for_click();
    config.enable_longpress_drag_selection = false;
    m_touchSelectionController.reset(new ui::TouchSelectionController(m_touchSelectionControllerClient.get(), config));

    host()->render_frame_metadata_provider()->ReportAllFrameSubmissionsForTesting(true);

    host()->SetView(this);
}

RenderWidgetHostViewQt::~RenderWidgetHostViewQt()
{
    m_delegate.reset();

    QObject::disconnect(m_adapterClientDestroyedConnection);

    if (text_input_manager_)
        text_input_manager_->RemoveObserver(this);

    m_touchSelectionController.reset();
    m_touchSelectionControllerClient.reset();
}

void RenderWidgetHostViewQt::setDelegate(RenderWidgetHostViewQtDelegate* delegate)
{
    m_delegate.reset(delegate);
    if (m_deferredShow) {
        m_deferredShow = false;
        Show();
    }
    delegateClient()->visualPropertiesChanged();
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
    m_delegate->resize(sizeInDips.width(), sizeInDips.height());
}

void RenderWidgetHostViewQt::SetBounds(const gfx::Rect &windowRectInDips)
{
    DCHECK(isPopup());
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
    if (!isPopup())
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
    return toGfx(delegateClient()->viewRectInDips());
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
    else
        host()->Send(new RenderViewObserverQt_SetBackgroundColor(rvh->GetRoutingID(), color));
}

// Return value indicates whether the mouse is locked successfully or not.
blink::mojom::PointerLockResult RenderWidgetHostViewQt::LockMouse(bool request_unadjusted_movement)
{
    if (request_unadjusted_movement)
        return blink::mojom::PointerLockResult::kUnsupportedOptions;

    delegateClient()->resetPreviousMousePosition();
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

void RenderWidgetHostViewQt::UpdateCursor(const content::WebCursor &webCursor)
{
    DisplayCursor(webCursor);
}

void RenderWidgetHostViewQt::DisplayCursor(const content::WebCursor &webCursor)
{
    const ui::Cursor &cursorInfo = webCursor.cursor();
    Qt::CursorShape shape = Qt::ArrowCursor;
#if defined(USE_AURA)
    ui::mojom::CursorType auraType = ui::mojom::CursorType::kNull;
#endif
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
#if defined(USE_AURA)
    case ui::mojom::CursorType::kVerticalText:
        auraType = ui::mojom::CursorType::kVerticalText;
        break;
    case ui::mojom::CursorType::kCell:
        auraType = ui::mojom::CursorType::kCell;
        break;
    case ui::mojom::CursorType::kContextMenu:
        auraType = ui::mojom::CursorType::kContextMenu;
        break;
    case ui::mojom::CursorType::kZoomIn:
        auraType = ui::mojom::CursorType::kZoomIn;
        break;
    case ui::mojom::CursorType::kZoomOut:
        auraType = ui::mojom::CursorType::kZoomOut;
        break;
#else
    case ui::mojom::CursorType::kVerticalText:
    case ui::mojom::CursorType::kCell:
    case ui::mojom::CursorType::kContextMenu:
    case ui::mojom::CursorType::kZoomIn:
    case ui::mojom::CursorType::kZoomOut:
        // FIXME: Support on OS X
        break;
#endif
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
#if defined(USE_AURA)
    if (auraType != ui::mojom::CursorType::kNull) {
        int resourceId;
        gfx::Point hotspot;
        // GetCursorDataFor only knows hotspots for 1x and 2x cursor images, in physical pixels.
        qreal hotspotDpr = m_screenInfo.device_scale_factor <= 1.0f ? 1.0f : 2.0f;
        if (ui::GetCursorDataFor(ui::CursorSize::kNormal, auraType, hotspotDpr, &resourceId, &hotspot)) {
            if (const gfx::ImageSkia *imageSkia = ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(resourceId)) {
                QImage imageQt = toQImage(imageSkia->GetRepresentation(m_screenInfo.device_scale_factor));

                // Convert hotspot coordinates into device-independent pixels.
                qreal hotX = hotspot.x() / hotspotDpr;
                qreal hotY = hotspot.y() / hotspotDpr;

#if defined(Q_OS_LINUX)
                // QTBUG-68571: On Linux (xcb, wayland, eglfs), hotspot coordinates must be in physical pixels.
                qreal imageDpr = imageQt.devicePixelRatio();
                hotX *= imageDpr;
                hotY *= imageDpr;
#endif

                m_delegate->updateCursor(QCursor(QPixmap::fromImage(std::move(imageQt)), qRound(hotX), qRound(hotY)));
                return;
            }
        }
    }
#endif
    m_delegate->updateCursor(QCursor(shape));
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
    if (m_adapterClient)
        m_adapterClient->setToolTip(toQt(tooltip_text));
}

void RenderWidgetHostViewQt::GetScreenInfo(content::ScreenInfo *results)
{
    *results = m_screenInfo;
}

gfx::Rect RenderWidgetHostViewQt::GetBoundsInRootWindow()
{
    return toGfx(delegateClient()->windowRectInDips());
}

void RenderWidgetHostViewQt::OnUpdateTextInputStateCalled(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view, bool did_update_state)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);
    Q_UNUSED(did_update_state);

    const content::TextInputState *state = text_input_manager_->GetTextInputState();
    if (!state) {
        m_delegate->inputMethodStateChanged(false /*editorVisible*/, false /*passwordInput*/);
        m_delegate->setInputMethodHints(Qt::ImhNone);
        return;
    }

    ui::TextInputType type = getTextInputType();
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    m_delegate->setInputMethodHints(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu);
#else
    m_delegate->setInputMethodHints(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText);
#endif
    QString surroundingText = toQt(state->value);
    // Remove IME composition text from the surrounding text
    if (state->composition_start != -1 && state->composition_end != -1)
        surroundingText.remove(state->composition_start,
                               state->composition_end - state->composition_start);
    delegateClient()->setSurroundingText(surroundingText);

    // In case of text selection, the update is expected in RenderWidgetHostViewQt::selectionChanged().
    if (GetSelectedText().empty()) {
        // At this point it is unknown whether the text input state has been updated due to a text selection.
        // Keep the cursor position updated for cursor movements too.
        delegateClient()->setCursorPosition(state->selection_start);
        m_delegate->inputMethodStateChanged(type != ui::TEXT_INPUT_TYPE_NONE, type == ui::TEXT_INPUT_TYPE_PASSWORD);
    }

    if (m_imState & ImStateFlags::TextInputStateUpdated) {
        m_imState = ImStateFlags::TextInputStateUpdated;
        return;
    }

    // Ignore selection change triggered by ime composition unless it clears an actual text selection
    if (state->composition_start != -1 && delegateClient()->isPreviousSelectionEmpty()) {
        m_imState = 0;
        return;
    }

    m_imState |= ImStateFlags::TextInputStateUpdated;
    if (m_imState == ImStateFlags::AllFlags)
        delegateClient()->selectionChanged();
}

void RenderWidgetHostViewQt::OnSelectionBoundsChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);

    m_imState |= ImStateFlags::TextSelectionBoundsUpdated;
    if (m_imState == ImStateFlags::AllFlags
            || (m_imState == ImStateFlags::TextSelectionFlags && getTextInputType() == ui::TEXT_INPUT_TYPE_NONE)) {
        delegateClient()->selectionChanged();
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
        delegateClient()->selectionChanged();
    }
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
        case blink::WebInputEvent::kGestureLongPress:
            m_touchSelectionController->HandleLongPressEvent(event.TimeStamp(), event.PositionInWidget());
            break;
        case blink::WebInputEvent::kGestureTap:
            m_touchSelectionController->HandleTapEvent(event.PositionInWidget(), event.data.tap.tap_count);
            break;
        case blink::WebInputEvent::kGestureScrollBegin:
            m_touchSelectionControllerClient->onScrollBegin();
            break;
        case blink::WebInputEvent::kGestureScrollEnd:
            m_touchSelectionControllerClient->onScrollEnd();
            break;
        default:
            break;
        }
    }

    host()->ForwardGestureEvent(event);
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
    synchronizeVisualProperties(metadata.local_surface_id_allocation);
}

void RenderWidgetHostViewQt::OnDidFirstVisuallyNonEmptyPaint()
{
    m_adapterClient->didFirstVisuallyNonEmptyPaint();
}

Compositor::Id RenderWidgetHostViewQt::compositorId()
{
    return m_uiCompositor->frame_sink_id();
}

void RenderWidgetHostViewQt::notifyShown()
{
    // Handle possible frame eviction:
    if (!m_dfhLocalSurfaceIdAllocator.HasValidLocalSurfaceIdAllocation())
        m_dfhLocalSurfaceIdAllocator.GenerateId();
    if (m_visible)
        return;
    m_visible = true;

    host()->WasShown(base::nullopt);

    m_delegatedFrameHost->AttachToCompositor(m_uiCompositor.get());
    m_delegatedFrameHost->WasShown(GetLocalSurfaceIdAllocation().local_surface_id(),
                                   toGfx(delegateClient()->viewRectInDips().size()), base::nullopt);
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

void RenderWidgetHostViewQt::ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo &touch, content::InputEventAckState ack_result) {
    Q_UNUSED(touch);
    const bool eventConsumed = ack_result == content::INPUT_EVENT_ACK_STATE_CONSUMED;
    const bool isSetNonBlocking = content::InputEventAckStateIsSetNonBlocking(ack_result);
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
    host()->ForwardTouchEventWithLatencyInfo(touchEvent, CreateLatencyInfo(touchEvent));
}

bool RenderWidgetHostViewQt::isPopup() const
{
    return widget_type_ == content::WidgetType::kPopup;
}

bool RenderWidgetHostViewQt::updateScreenInfo()
{
    content::ScreenInfo oldScreenInfo = m_screenInfo;
    QScreen *screen = m_delegate->window() ? m_delegate->window()->screen() : nullptr;

    if (screen) {
        m_screenInfo.device_scale_factor = screen->devicePixelRatio();
        m_screenInfo.depth_per_component = 8;
        m_screenInfo.depth = screen->depth();
        m_screenInfo.is_monochrome = (m_screenInfo.depth == 1);
        m_screenInfo.rect = toGfx(screen->geometry());
        m_screenInfo.available_rect = toGfx(screen->availableGeometry());
    } else {
        m_screenInfo.device_scale_factor = qGuiApp->devicePixelRatio();
    }

    return (m_screenInfo != oldScreenInfo);
}

void RenderWidgetHostViewQt::handleWheelEvent(QWheelEvent *event)
{
    if (!m_wheelAckPending) {
        Q_ASSERT(m_pendingWheelEvents.isEmpty());
        blink::WebMouseWheelEvent webEvent = WebEventFactory::toWebWheelEvent(event);
        m_wheelAckPending = (webEvent.phase != blink::WebMouseWheelEvent::kPhaseEnded);
        GetMouseWheelPhaseHandler()->AddPhaseIfNeededAndScheduleEndEvent(webEvent, false);
        host()->ForwardWheelEvent(webEvent);
        return;
    }
    if (!m_pendingWheelEvents.isEmpty()) {
        // Try to combine with this wheel event with the last pending one.
        if (WebEventFactory::coalesceWebWheelEvent(m_pendingWheelEvents.last(), event))
            return;
    }
    m_pendingWheelEvents.append(WebEventFactory::toWebWheelEvent(event));
}

void RenderWidgetHostViewQt::WheelEventAck(const blink::WebMouseWheelEvent &event, content::InputEventAckState /*ack_result*/)
{
    if (event.phase == blink::WebMouseWheelEvent::kPhaseEnded)
        return;
    Q_ASSERT(m_wheelAckPending);
    m_wheelAckPending = false;
    while (!m_pendingWheelEvents.isEmpty() && !m_wheelAckPending) {
        blink::WebMouseWheelEvent webEvent = m_pendingWheelEvents.takeFirst();
        m_wheelAckPending = (webEvent.phase != blink::WebMouseWheelEvent::kPhaseEnded);
        m_mouseWheelPhaseHandler.AddPhaseIfNeededAndScheduleEndEvent(webEvent, false);
        host()->ForwardWheelEvent(webEvent);
    }
}

void RenderWidgetHostViewQt::GestureEventAck(const blink::WebGestureEvent &event, content::InputEventAckState ack_result)
{
    // Forward unhandled scroll events back as wheel events
    if (event.GetType() != blink::WebInputEvent::kGestureScrollUpdate)
        return;
    switch (ack_result) {
    case content::INPUT_EVENT_ACK_STATE_NOT_CONSUMED:
    case content::INPUT_EVENT_ACK_STATE_NO_CONSUMER_EXISTS:
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

content::RenderFrameHost *RenderWidgetHostViewQt::getFocusedFrameHost()
{
    content::RenderViewHostImpl *viewHost = content::RenderViewHostImpl::From(host());
    if (!viewHost)
        return nullptr;

    content::FrameTreeNode *focusedFrame = viewHost->GetDelegate()->GetFrameTree()->GetFocusedFrame();
    if (!focusedFrame)
        return nullptr;

    return focusedFrame->current_frame_host();
}

content::mojom::FrameInputHandler *RenderWidgetHostViewQt::getFrameInputHandler()
{
    content::RenderFrameHostImpl *frameHost = static_cast<content::RenderFrameHostImpl *>(getFocusedFrameHost());
    if (!frameHost)
        return nullptr;

    return frameHost->GetFrameInputHandler();
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

const viz::LocalSurfaceIdAllocation &RenderWidgetHostViewQt::GetLocalSurfaceIdAllocation() const
{
    return m_dfhLocalSurfaceIdAllocator.GetCurrentLocalSurfaceIdAllocation();
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
    content::RenderWidgetHostViewBase::OnRenderFrameMetadataChangedAfterActivation();

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

void RenderWidgetHostViewQt::synchronizeVisualProperties(const base::Optional<viz::LocalSurfaceIdAllocation> &childSurfaceId)
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
            m_uiCompositorLocalSurfaceIdAllocator.GetCurrentLocalSurfaceIdAllocation());
    m_delegatedFrameHost->EmbedSurface(
            m_dfhLocalSurfaceIdAllocator.GetCurrentLocalSurfaceIdAllocation().local_surface_id(),
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
