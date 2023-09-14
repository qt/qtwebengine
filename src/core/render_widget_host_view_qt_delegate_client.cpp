// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "render_widget_host_view_qt_delegate_client.h"

#include "render_widget_host_view_qt.h"
#include "touch_selection_controller_client_qt.h"
#include "type_conversion.h"
#include "web_contents_adapter.h"
#include "web_contents_adapter_client.h"
#include "web_event_factory.h"

#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_input_event_router.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include <QEvent>
#include <QInputMethodEvent>
#include <QSet>
#include <QStyleHints>
#include <QTextFormat>
#include <QVariant>

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/private/qinputcontrol_p.h>

namespace QtWebEngineCore {

static inline int firstAvailableId(const QMap<int, int> &map)
{
    ui::BitSet32 usedIds;
    QMap<int, int>::const_iterator end = map.end();
    for (QMap<int, int>::const_iterator it = map.begin(); it != end; ++it)
        usedIds.mark_bit(it.value());
    return usedIds.first_unmarked_bit();
}

typedef QPair<int, QTouchEvent::TouchPoint> TouchPoint;
QList<TouchPoint> RenderWidgetHostViewQtDelegateClient::mapTouchPointIds(const QList<QTouchEvent::TouchPoint> &input)
{
    QList<TouchPoint> output;
    for (int i = 0; i < input.size(); ++i) {
        const QTouchEvent::TouchPoint &point = input[i];

        int qtId = point.id();
        QMap<int, int>::const_iterator it = m_touchIdMapping.find(qtId);
        if (it == m_touchIdMapping.end()) {
            Q_ASSERT_X(m_touchIdMapping.size() <= 16, "", "Number of mapped ids can't exceed 16 for velocity tracker");
            it = m_touchIdMapping.insert(qtId, firstAvailableId(m_touchIdMapping));
        }

        output.append(qMakePair(it.value(), point));
    }

    Q_ASSERT(output.size() == std::accumulate(output.cbegin(), output.cend(), QSet<int>(),
                 [] (QSet<int> s, const TouchPoint &p) { s.insert(p.second.id()); return s; }).size());

    for (auto &&point : std::as_const(input))
        if (point.state() == QEventPoint::Released)
            m_touchIdMapping.remove(point.id());

    return output;
}

static uint32_t s_eventId = 0;
class MotionEventQt : public ui::MotionEvent
{
public:
    MotionEventQt(const QList<TouchPoint> &touchPoints,
                  const base::TimeTicks &eventTime, Action action,
                  const Qt::KeyboardModifiers modifiers, int index = -1)
        : touchPoints(touchPoints)
        , eventTime(eventTime)
        , action(action)
        , eventId(++s_eventId)
        , flags(flagsFromModifiers(modifiers))
        , index(index)
    {
        // index is only valid for ACTION_DOWN and ACTION_UP and should correspond to the point causing it
        // see blink_event_util.cc:ToWebTouchPointState for details
        Q_ASSERT_X((action != Action::POINTER_DOWN && action != Action::POINTER_UP && index == -1)
                || (action == Action::POINTER_DOWN && index >= 0 && touchPoint(index).state() == QEventPoint::Pressed)
                || (action == Action::POINTER_UP && index >= 0 && touchPoint(index).state() == QEventPoint::Released),
                "MotionEventQt", qPrintable(QString("action: %1, index: %2, state: %3").arg(int(action)).arg(index).arg(touchPoint(index).state())));
    }

    uint32_t GetUniqueEventId() const override { return eventId; }
    Action GetAction() const override { return action; }
    int GetActionIndex() const override { return index; }
    size_t GetPointerCount() const override { return touchPoints.size(); }
    int GetPointerId(size_t pointer_index) const override
    {
        return touchPoints[pointer_index].first;
    }
    float GetX(size_t pointer_index) const override
    {
        return touchPoint(pointer_index).position().x();
    }
    float GetY(size_t pointer_index) const override
    {
        return touchPoint(pointer_index).position().y();
    }
    float GetRawX(size_t pointer_index) const override
    {
        return touchPoint(pointer_index).globalPosition().x();
    }
    float GetRawY(size_t pointer_index) const override
    {
        return touchPoint(pointer_index).globalPosition().y();
    }
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
    float GetOrientation(size_t pointer_index) const override { return 0; }
    int GetFlags() const override { return flags; }
    float GetPressure(size_t pointer_index) const override
    {
        return touchPoint(pointer_index).pressure();
    }
    float GetTiltX(size_t pointer_index) const override { return 0; }
    float GetTiltY(size_t pointer_index) const override { return 0; }
    float GetTwist(size_t) const override { return 0; }
    float GetTangentialPressure(size_t) const override { return 0; }
    base::TimeTicks GetEventTime() const override { return eventTime; }

    size_t GetHistorySize() const override { return 0; }
    base::TimeTicks GetHistoricalEventTime(size_t historical_index) const override
    {
        return base::TimeTicks();
    }
    float GetHistoricalTouchMajor(size_t pointer_index, size_t historical_index) const override
    {
        return 0;
    }
    float GetHistoricalX(size_t pointer_index, size_t historical_index) const override { return 0; }
    float GetHistoricalY(size_t pointer_index, size_t historical_index) const override { return 0; }
    ToolType GetToolType(size_t pointer_index) const override
    {
        return ui::MotionEvent::ToolType::FINGER;
    }

    int GetButtonState() const override { return 0; }

private:
    QList<TouchPoint> touchPoints;
    base::TimeTicks eventTime;
    Action action;
    const uint32_t eventId;
    int flags;
    int index;
    const QTouchEvent::TouchPoint& touchPoint(size_t i) const { return touchPoints[i].second; }
};

RenderWidgetHostViewQtDelegateClient::RenderWidgetHostViewQtDelegateClient(
        RenderWidgetHostViewQt *rwhv)
    : m_rwhv(rwhv)
{
    Q_ASSERT(rwhv);
}

Compositor::Id RenderWidgetHostViewQtDelegateClient::compositorId()
{
    return m_rwhv->compositorId();
}

void RenderWidgetHostViewQtDelegateClient::notifyShown()
{
    m_rwhv->notifyShown();
}

void RenderWidgetHostViewQtDelegateClient::notifyHidden()
{
    m_rwhv->notifyHidden();
}

void RenderWidgetHostViewQtDelegateClient::visualPropertiesChanged()
{
    RenderWidgetHostViewQtDelegate *delegate = m_rwhv->delegate();
    if (!delegate)
        return;

    QRect oldViewRect = m_viewRectInDips;
    m_viewRectInDips = delegate->viewGeometry().toAlignedRect();

    QRect oldWindowRect = m_windowRectInDips;
    m_windowRectInDips = delegate->windowGeometry();

    bool screenInfoChanged = m_rwhv->updateScreenInfo();

    if (m_viewRectInDips != oldViewRect || m_windowRectInDips != oldWindowRect)
        m_rwhv->host()->SendScreenRects();

    if (m_viewRectInDips.size() != oldViewRect.size() || screenInfoChanged)
        m_rwhv->synchronizeVisualProperties(absl::nullopt);
}

bool RenderWidgetHostViewQtDelegateClient::forwardEvent(QEvent *event)
{
    Q_ASSERT(m_rwhv->host()->GetView());

    switch (event->type()) {
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        event->ignore();
        auto acceptKeyOutOfInputField = [](QKeyEvent *keyEvent) -> bool {
#ifdef Q_OS_MACOS
            // Check if a shortcut is registered for this key sequence.
            QKeySequence sequence = QKeySequence((keyEvent->modifiers() | keyEvent->key())
                                                 & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier));
            if (QGuiApplicationPrivate::instance()->shortcutMap.hasShortcutForKeySequence(sequence))
                return false;

            // The following shortcuts are handled out of input field too but
            // disabled on macOS to let the blinking menu handling to the
            // embedder application (see kKeyboardCodeKeyDownEntries in
            // third_party/WebKit/Source/core/editing/EditingBehavior.cpp).
            // Let them pass on macOS to generate the corresponding edit command.
            return keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Paste)
                    || keyEvent->matches(QKeySequence::Cut)
                    || keyEvent->matches(QKeySequence::SelectAll);
#else
            return false;
#endif
        };

        if (!inputMethodQuery(Qt::ImEnabled).toBool()
            && !(inputMethodQuery(Qt::ImHints).toInt() & Qt::ImhHiddenText)
            && !acceptKeyOutOfInputField(keyEvent))
            return false;

        Q_ASSERT(m_editCommand.empty());
        if (WebEventFactory::getEditCommand(keyEvent, &m_editCommand)
            || QInputControl::isCommonTextEditShortcut(keyEvent)) {
            event->accept();
            return true;
        }

        return false;
    }
    case QEvent::MouseButtonPress:
        m_rwhv->Focus();
        Q_FALLTHROUGH();
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        // Skip second MouseMove event when a window is being adopted, so that Chromium
        // can properly handle further move events.
        // Also make sure the adapter client exists to prevent a null pointer dereference,
        // because it's possible for a QWebEnginePagePrivate (adapter) instance to be destroyed,
        // and then the OS (observed on Windows) might still send mouse move events to a still
        // existing popup RWHVQDW instance.
        if (m_rwhv->adapterClient() && m_rwhv->adapterClient()->isBeingAdopted())
            return false;
        handleMouseEvent(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        handleKeyEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::Wheel:
        m_rwhv->handleWheelEvent(static_cast<QWheelEvent *>(event));
        break;
    case QEvent::TouchBegin:
        m_rwhv->Focus();
        Q_FALLTHROUGH();
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        handleTouchEvent(static_cast<QTouchEvent *>(event));
        break;
#if QT_CONFIG(tabletevent)
    case QEvent::TabletPress:
        m_rwhv->Focus();
        Q_FALLTHROUGH();
    case QEvent::TabletRelease:
    case QEvent::TabletMove:
        handleTabletEvent(static_cast<QTabletEvent *>(event));
        break;
#endif
#if QT_CONFIG(gestures)
    case QEvent::NativeGesture:
        handleGestureEvent(static_cast<QNativeGestureEvent *>(event));
        break;
#endif
    case QEvent::HoverMove:
        handleHoverEvent(static_cast<QHoverEvent *>(event));
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut: {
        // Focus in/out events for popup event do not mean 'parent' focus change
        // and should not be handled by Chromium
        QFocusEvent *e = static_cast<QFocusEvent *>(event);
        if (e->reason() != Qt::PopupFocusReason)
            handleFocusEvent(e);
    } break;
    case QEvent::InputMethod:
        handleInputMethodEvent(static_cast<QInputMethodEvent *>(event));
        break;
    case QEvent::InputMethodQuery:
        handleInputMethodQueryEvent(static_cast<QInputMethodQueryEvent *>(event));
        break;
    case QEvent::Leave:
#ifdef Q_OS_WIN
        if (m_mouseButtonPressed > 0)
            return false;
#endif
    case QEvent::HoverLeave:
        if (m_rwhv->host()->delegate() && m_rwhv->host()->delegate()->GetInputEventRouter()) {
            auto webEvent = WebEventFactory::toWebMouseEvent(event);
            m_rwhv->host()->delegate()->GetInputEventRouter()->RouteMouseEvent(m_rwhv, &webEvent, ui::LatencyInfo());
        }
        break;
    default:
        return false;
    }
    return true;
}

QVariant RenderWidgetHostViewQtDelegateClient::inputMethodQuery(Qt::InputMethodQuery query)
{
    switch (query) {
    case Qt::ImEnabled:
        return QVariant(m_rwhv->getTextInputType() != ui::TEXT_INPUT_TYPE_NONE);
    case Qt::ImFont:
        // TODO: Implement this
        return QVariant();
    case Qt::ImCursorRectangle: {
        if (m_rwhv->GetTextInputManager()) {
            if (auto *region = m_rwhv->GetTextInputManager()->GetSelectionRegion()) {
                if (region->focus.GetHeight() > 0) {
                    gfx::Rect caretRect =
                            gfx::RectBetweenSelectionBounds(region->anchor, region->focus);
                    if (caretRect.width() == 0)
                        caretRect.set_width(1); // IME API on Windows expects a width > 0
                    return toQt(caretRect);
                }
            }
        }
        return QVariant();
    }
    case Qt::ImAbsolutePosition:
    case Qt::ImCursorPosition:
        return m_cursorPosition;
    case Qt::ImAnchorPosition:
        return m_rwhv->GetSelectedText().empty() ? m_cursorPosition
                                                 : m_anchorPositionWithinSelection;
    case Qt::ImSurroundingText:
        return m_surroundingText;
    case Qt::ImCurrentSelection:
        return toQt(m_rwhv->GetSelectedText());
    case Qt::ImMaximumTextLength:
        // TODO: Implement this
        return QVariant(); // No limit.
    case Qt::ImHints:
        return int(toQtInputMethodHints(m_rwhv->getTextInputType()) | Qt::ImhNoPredictiveText
                   | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu);
    default:
        return QVariant();
    }
}

void RenderWidgetHostViewQtDelegateClient::closePopup()
{
    // We notify the popup to be closed by telling it that it lost focus. WebKit does the rest
    // (hiding the widget and automatic memory cleanup via
    // RenderWidget::CloseWidgetSoon() -> RenderWidgetHostImpl::ShutdownAndDestroyWidget(true).
    m_rwhv->host()->SetActive(false);
    m_rwhv->host()->LostFocus();
}

template<class T>
void RenderWidgetHostViewQtDelegateClient::handlePointerEvent(T *event)
{
    // Currently WebMouseEvent is a subclass of WebPointerProperties, so basically
    // tablet events are mouse events with extra properties.
    blink::WebMouseEvent webEvent = WebEventFactory::toWebMouseEvent(event);
    if ((webEvent.GetType() == blink::WebInputEvent::Type::kMouseDown
         || webEvent.GetType() == blink::WebInputEvent::Type::kMouseUp)
        && webEvent.button == blink::WebMouseEvent::Button::kNoButton) {
        // Blink can only handle the 5 main mouse-buttons and may assert when processing mouse-down
        // for no button.
        LOG(INFO) << "Unhandled mouse button";
        return;
    }

    if (webEvent.GetType() == blink::WebInputEvent::Type::kMouseDown) {
        if (event->button() != m_clickHelper.lastPressButton
            || (event->timestamp() - m_clickHelper.lastPressTimestamp
                > static_cast<ulong>(qGuiApp->styleHints()->mouseDoubleClickInterval()))
            || (event->position() - m_clickHelper.lastPressPosition).manhattanLength()
                    > qGuiApp->styleHints()->startDragDistance()
            || m_clickHelper.clickCounter >= 3)
            m_clickHelper.clickCounter = 0;

        m_clickHelper.lastPressTimestamp = event->timestamp();
        webEvent.click_count = ++m_clickHelper.clickCounter;
        m_clickHelper.lastPressButton = event->button();
        m_clickHelper.lastPressPosition = event->position().toPoint();
    }

    if (webEvent.GetType() == blink::WebInputEvent::Type::kMouseUp)
        webEvent.click_count = m_clickHelper.clickCounter;

    webEvent.movement_x = event->globalPosition().x() - m_previousMousePosition.x();
    webEvent.movement_y = event->globalPosition().y() - m_previousMousePosition.y();
    webEvent.is_raw_movement_event = true;

    if (m_rwhv->IsMouseLocked())
        QCursor::setPos(m_previousMousePosition);
    else
        m_previousMousePosition = event->globalPosition().toPoint();

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

    if (m_rwhv->host()->delegate() && m_rwhv->host()->delegate()->GetInputEventRouter())
        m_rwhv->host()->delegate()->GetInputEventRouter()->RouteMouseEvent(m_rwhv, &webEvent, ui::LatencyInfo());
}

void RenderWidgetHostViewQtDelegateClient::handleMouseEvent(QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
        m_mouseButtonPressed++;
    if (event->type() == QEvent::MouseButtonRelease)
        m_mouseButtonPressed--;

    handlePointerEvent<QMouseEvent>(event);
}

void RenderWidgetHostViewQtDelegateClient::handleKeyEvent(QKeyEvent *event)
{
    if (m_rwhv->IsMouseLocked() && event->key() == Qt::Key_Escape
        && event->type() == QEvent::KeyRelease)
        m_rwhv->UnlockMouse();

    if (m_receivedEmptyImeEvent) {
        // IME composition was not finished with a valid commit string.
        // We're getting the composition result in a key event.
        if (event->key() != 0) {
            // The key event is not a result of an IME composition. Cancel IME.
            m_rwhv->host()->ImeCancelComposition();
            m_receivedEmptyImeEvent = false;
        } else {
            if (event->type() == QEvent::KeyRelease) {
                m_rwhv->host()->ImeCommitText(toString16(event->text()),
                                              std::vector<ui::ImeTextSpan>(),
                                              gfx::Range::InvalidRange(), 0);
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
    if (event->type() == QEvent::KeyRelease && event->isAutoRepeat())
        return;

    if (!m_rwhv->GetFocusedWidget())
        return;

    content::NativeWebKeyboardEvent webEvent = WebEventFactory::toWebKeyboardEvent(event);
    if (webEvent.GetType() == blink::WebInputEvent::Type::kRawKeyDown && !m_editCommand.empty()) {
        ui::LatencyInfo latency;
        latency.set_source_event_type(ui::SourceEventType::KEY_PRESS);
        std::vector<blink::mojom::EditCommandPtr> commands;
        commands.emplace_back(blink::mojom::EditCommand::New(m_editCommand, ""));
        m_editCommand.clear();
        m_rwhv->GetFocusedWidget()->ForwardKeyboardEventWithCommands(webEvent, latency, std::move(commands), nullptr);
        return;
    }

    bool keyDownTextInsertion =
            webEvent.GetType() == blink::WebInputEvent::Type::kRawKeyDown && webEvent.text[0];
    webEvent.skip_in_browser = keyDownTextInsertion;
    m_rwhv->GetFocusedWidget()->ForwardKeyboardEvent(webEvent);

    if (keyDownTextInsertion) {
        // Blink won't consume the RawKeyDown, but rather the Char event in this case.
        // The RawKeyDown is skipped on the way back (see above).
        // The same os_event will be set on both NativeWebKeyboardEvents.
        webEvent.skip_in_browser = false;
        webEvent.SetType(blink::WebInputEvent::Type::kChar);
        m_rwhv->GetFocusedWidget()->ForwardKeyboardEvent(webEvent);
    }
}

void RenderWidgetHostViewQtDelegateClient::handleTouchEvent(QTouchEvent *event)
{
    // On macOS instead of handling touch events, we use the OS provided QNativeGestureEvents.
#ifdef Q_OS_MACOS
    if (event->spontaneous()) {
        return;
    } else {
        VLOG(1) << "Sending simulated touch events to Chromium does not work properly on macOS. "
                   "Consider using QNativeGestureEvents or QMouseEvents.";
    }
#endif

    // Chromium expects the touch event timestamps to be comparable to base::TimeTicks::Now().
    // Most importantly we also have to preserve the relative time distance between events.
    // Calculate a delta between event timestamps and Now() on the first received event, and
    // apply this delta to all successive events. This delta is most likely smaller than it
    // should by calculating it here but this will hopefully cause less than one frame of delay.
    base::TimeTicks eventTimestamp = base::TimeTicks() + base::Milliseconds(event->timestamp());
    if (m_eventsToNowDelta == 0)
        m_eventsToNowDelta = (base::TimeTicks::Now() - eventTimestamp).InMicroseconds();
    eventTimestamp += base::Microseconds(m_eventsToNowDelta);

    auto touchPoints = mapTouchPointIds(event->points());
    // Make sure that POINTER_DOWN action is delivered before MOVE, and MOVE before POINTER_UP
    std::sort(touchPoints.begin(), touchPoints.end(), [] (const TouchPoint &l, const TouchPoint &r) {
        return l.second.state() < r.second.state();
    });

    auto sc = qScopeGuard([&] () {
        switch (event->type()) {
            case QEvent::TouchCancel:
                for (auto &&it : std::as_const(touchPoints))
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
        case QEventPoint::Pressed:
            action = ui::MotionEvent::Action::DOWN;
            break;
        case QEventPoint::Updated:
            action = ui::MotionEvent::Action::MOVE;
            break;
        case QEventPoint::Released:
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
        Q_ASSERT(event->type() == QEvent::TouchCancel);
        touchPoints = m_previousTouchPoints;
        if (touchPoints.isEmpty())
            return;

        action = ui::MotionEvent::Action::CANCEL;
    }

    MotionEventQt me(touchPoints, eventTimestamp, action, event->modifiers());
    if (m_rwhv->getTouchSelectionController()->WillHandleTouchEvent(me))
        return;

    switch (event->type()) {
    case QEvent::TouchBegin:
        m_sendMotionActionDown = true;
        m_touchMotionStarted = true;
        m_rwhv->getTouchSelectionControllerClient()->onTouchDown();
        break;
    case QEvent::TouchUpdate:
        m_touchMotionStarted = true;
        break;
    case QEvent::TouchCancel:
    {
        // Only process TouchCancel events received following a TouchBegin or TouchUpdate event
        if (m_touchMotionStarted) {
            MotionEventQt cancelEvent(touchPoints, eventTimestamp, ui::MotionEvent::Action::CANCEL, event->modifiers());
            m_rwhv->processMotionEvent(cancelEvent);
        }

        return;
    }
    case QEvent::TouchEnd:
        m_rwhv->getTouchSelectionControllerClient()->onTouchUp();
        break;
    default:
        break;
    }

    if (m_imeInProgress && event->type() == QEvent::TouchBegin) {
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
    while ((lastPressIndex + 1) < touchPoints.size() && touchPoints[lastPressIndex + 1].second.state() == QEventPoint::Pressed)
        ++lastPressIndex;

    switch (event->type()) {
        case QEvent::TouchBegin:
            m_rwhv->processMotionEvent(MotionEventQt(touchPoints.mid(lastPressIndex),
                                                     eventTimestamp, ui::MotionEvent::Action::DOWN, event->modifiers()));
            --lastPressIndex;
            Q_FALLTHROUGH();

        case QEvent::TouchUpdate:
            for (; lastPressIndex >= 0; --lastPressIndex) {
                Q_ASSERT(touchPoints[lastPressIndex].second.state() == QEventPoint::Pressed);
                MotionEventQt me(touchPoints.mid(lastPressIndex), eventTimestamp, ui::MotionEvent::Action::POINTER_DOWN, event->modifiers(), 0);
                m_rwhv->processMotionEvent(me);
            }

            if (event->touchPointStates() & Qt::TouchPointMoved)
                m_rwhv->processMotionEvent(MotionEventQt(touchPoints, eventTimestamp, ui::MotionEvent::Action::MOVE, event->modifiers()));

            Q_FALLTHROUGH();

        case QEvent::TouchEnd:
            while (!touchPoints.isEmpty() && touchPoints.back().second.state() == QEventPoint::Released) {
                auto action = touchPoints.size() > 1 ? ui::MotionEvent::Action::POINTER_UP : ui::MotionEvent::Action::UP;
                int index = action == ui::MotionEvent::Action::POINTER_UP ? touchPoints.size() - 1 : -1;
                m_rwhv->processMotionEvent(MotionEventQt(touchPoints, eventTimestamp, action, event->modifiers(), index));
                touchPoints.pop_back();
            }
            break;

        default:
            Q_ASSERT_X(false, __FUNCTION__, "Other event types are expected to be already handled.");
            break;
    }
}

#if QT_CONFIG(tabletevent)
void RenderWidgetHostViewQtDelegateClient::handleTabletEvent(QTabletEvent *event)
{
    handlePointerEvent<QTabletEvent>(event);
}
#endif

#if QT_CONFIG(gestures)
void RenderWidgetHostViewQtDelegateClient::handleGestureEvent(QNativeGestureEvent *event)
{
    const Qt::NativeGestureType type = event->gestureType();
    // These are the only supported gestures by Chromium so far.
    if (type == Qt::ZoomNativeGesture || type == Qt::SmartZoomNativeGesture
            || type == Qt::BeginNativeGesture || type == Qt::EndNativeGesture) {
        auto *hostDelegate = m_rwhv->host()->delegate();
        if (hostDelegate && hostDelegate->GetInputEventRouter()) {
            auto webEvent = WebEventFactory::toWebGestureEvent(event);
            hostDelegate->GetInputEventRouter()->RouteGestureEvent(m_rwhv, &webEvent, ui::LatencyInfo());
        }
    }
}
#endif

void RenderWidgetHostViewQtDelegateClient::handleHoverEvent(QHoverEvent *event)
{
    auto *hostDelegate = m_rwhv->host()->delegate();
    if (hostDelegate && hostDelegate->GetInputEventRouter()) {
        auto webEvent = WebEventFactory::toWebMouseEvent(event);
        hostDelegate->GetInputEventRouter()->RouteMouseEvent(m_rwhv, &webEvent, ui::LatencyInfo());
    }
}

void RenderWidgetHostViewQtDelegateClient::handleFocusEvent(QFocusEvent *event)
{
    if (event->gotFocus()) {
        m_rwhv->host()->GotFocus();
        m_rwhv->host()->SetActive(true);
        content::RenderViewHostImpl *rvh = content::RenderViewHostImpl::From(m_rwhv->host());
        Q_ASSERT(rvh);
        if (event->reason() == Qt::TabFocusReason)
            rvh->SetInitialFocus(false);
        else if (event->reason() == Qt::BacktabFocusReason)
            rvh->SetInitialFocus(true);
        event->accept();

        m_rwhv->adapterClient()->webContentsAdapter()->handlePendingMouseLockPermission();
    } else if (event->lostFocus()) {
        m_rwhv->host()->SetActive(false);
        m_rwhv->host()->LostFocus();
        event->accept();
    }
}

void RenderWidgetHostViewQtDelegateClient::handleInputMethodEvent(QInputMethodEvent *event)
{
    m_rwhv->resetInputManagerState();

    if (!m_rwhv->host())
        return;

    QString commitString = event->commitString();
    QString preeditString = event->preeditString();

    int cursorPositionInPreeditString = -1;
    gfx::Range selectionRange = gfx::Range::InvalidRange();

    const QList<QInputMethodEvent::Attribute> &attributes = event->attributes();
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
                                                 ui::ImeTextSpan::Thickness::kThin,
                                                 ui::ImeTextSpan::UnderlineStyle::kSolid,
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
        int newCursorPosition = (cursorPositionInPreeditString < 0) ? preeditString.length()
                                                                    : cursorPositionInPreeditString;
        selectionRange.set_start(newCursorPosition);
        selectionRange.set_end(newCursorPosition);
    }

    if (hasSelection) {
        if (auto *frameWidgetInputHandler = m_rwhv->getFrameWidgetInputHandler())
            frameWidgetInputHandler->SetEditableSelectionOffsets(selectionRange.start(), selectionRange.end());
    }

    int replacementLength = event->replacementLength();
    gfx::Range replacementRange = gfx::Range::InvalidRange();

    if (replacementLength > 0) {
        int replacementStart = event->replacementStart() < 0
                ? m_cursorPosition + event->replacementStart()
                : event->replacementStart();
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
            QGuiApplication::postEvent(qApp->focusObject(), event->clone());
        } else {
            m_receivedEmptyImeEvent = false;
            if (m_imeInProgress) {
                m_imeInProgress = false;
                m_rwhv->host()->ImeCancelComposition();
            }
        }

        return;
    }

    m_receivedEmptyImeEvent = false;

    // Finish compostion: insert or erase text.
    if (!commitString.isEmpty() || replacementLength > 0) {
        m_rwhv->host()->ImeCommitText(toString16(commitString), underlines, replacementRange, 0);
        m_imeInProgress = false;
    }

    // Update or start new composition.
    // Be aware of that, we might get a commit string and a pre-edit string in a single event and
    // this means a new composition.
    if (!preeditString.isEmpty()) {
        m_rwhv->host()->ImeSetComposition(toString16(preeditString), underlines, replacementRange,
                                          selectionRange.start(), selectionRange.end());
        m_imeInProgress = true;
    }
}

void RenderWidgetHostViewQtDelegateClient::handleInputMethodQueryEvent(
        QInputMethodQueryEvent *event)
{
    Qt::InputMethodQueries queries = event->queries();
    for (uint i = 0; i < 32; ++i) {
        Qt::InputMethodQuery query = (Qt::InputMethodQuery)(int)(queries & (1 << i));
        if (query) {
            QVariant v = inputMethodQuery(query);
            event->setValue(query, v);
        }
    }
    event->accept();
}

void RenderWidgetHostViewQtDelegateClient::clearPreviousTouchMotionState()
{
    m_previousTouchPoints.clear();
    m_touchMotionStarted = false;
}

void RenderWidgetHostViewQtDelegateClient::selectionChanged()
{
    m_rwhv->resetInputManagerState();
    ui::TextInputType type = m_rwhv->getTextInputType();
    content::TextInputManager *text_input_manager = m_rwhv->GetTextInputManager();

    // Handle text selection out of an input field
    if (type == ui::TEXT_INPUT_TYPE_NONE) {
        if (m_rwhv->GetSelectedText().empty() && m_emptyPreviousSelection)
            return;

        // Reset position values to emit selectionChanged signal when clearing text selection
        // by clicking into an input field. These values are intended to be used by inputMethodQuery
        // so they are not expected to be valid when selection is out of an input field.
        m_anchorPositionWithinSelection = -1;
        m_cursorPositionWithinSelection = -1;

        m_emptyPreviousSelection = m_rwhv->GetSelectedText().empty();
        m_rwhv->adapterClient()->selectionChanged();
        return;
    }

    if (m_rwhv->GetSelectedText().empty()) {
        // RenderWidgetHostViewQt::OnUpdateTextInputStateCalled() does not update the cursor
        // position if the selection is cleared because TextInputState changes before the
        // TextSelection change.
        Q_ASSERT(text_input_manager->GetTextInputState());
        m_cursorPosition = text_input_manager->GetTextInputState()->selection.start();
        m_rwhv->delegate()->inputMethodStateChanged(true /*editorVisible*/,
                                                    type == ui::TEXT_INPUT_TYPE_PASSWORD);

        m_anchorPositionWithinSelection = m_cursorPosition;
        m_cursorPositionWithinSelection = m_cursorPosition;

        if (!m_emptyPreviousSelection) {
            m_emptyPreviousSelection = true;
            m_rwhv->adapterClient()->selectionChanged();
        }

        return;
    }

    const content::TextInputManager::TextSelection *selection =
            text_input_manager->GetTextSelection();
    if (!selection)
        return;

    if (!selection->range().IsValid())
        return;

    int newAnchorPositionWithinSelection = 0;
    int newCursorPositionWithinSelection = 0;

    if (text_input_manager->GetSelectionRegion()->anchor.type() == gfx::SelectionBound::RIGHT) {
        newAnchorPositionWithinSelection = selection->range().GetMax() - selection->offset();
        newCursorPositionWithinSelection = selection->range().GetMin() - selection->offset();
    } else {
        newAnchorPositionWithinSelection = selection->range().GetMin() - selection->offset();
        newCursorPositionWithinSelection = selection->range().GetMax() - selection->offset();
    }

    if (m_anchorPositionWithinSelection == newAnchorPositionWithinSelection
        && m_cursorPositionWithinSelection == newCursorPositionWithinSelection) {
        return;
    }

    m_anchorPositionWithinSelection = newAnchorPositionWithinSelection;
    m_cursorPositionWithinSelection = newCursorPositionWithinSelection;

    if (!selection->selected_text().empty())
        m_cursorPosition = newCursorPositionWithinSelection;

    m_emptyPreviousSelection = selection->selected_text().empty();
    m_rwhv->delegate()->inputMethodStateChanged(true /*editorVisible*/,
                                                type == ui::TEXT_INPUT_TYPE_PASSWORD);
    m_rwhv->adapterClient()->selectionChanged();
}

} // namespace QtWebEngineCore
