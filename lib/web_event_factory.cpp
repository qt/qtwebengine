#include "web_event_factory.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QWheelEvent>

using namespace WebKit;

static inline double currentTimeForEvent(const QInputEvent* event)
{
    Q_ASSERT(event);

    if (event->timestamp())
        return static_cast<double>(event->timestamp()) / 1000;

    static QElapsedTimer timer;
    if (!timer.isValid())
        timer.start();
    return static_cast<double>(timer.elapsed()) / 1000;
}

static WebMouseEvent::Button mouseButtonForEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || (event->buttons() & Qt::LeftButton))
        return WebMouseEvent::ButtonLeft;
    else if (event->button() == Qt::RightButton || (event->buttons() & Qt::RightButton))
        return WebMouseEvent::ButtonRight;
    else if (event->button() == Qt::MidButton || (event->buttons() & Qt::MidButton))
        return WebMouseEvent::ButtonMiddle;
    return WebMouseEvent::ButtonNone;
}

static inline WebInputEvent::Modifiers modifiersForEvent(Qt::KeyboardModifiers modifiers)
{
    unsigned result = 0;
    if (modifiers & Qt::ShiftModifier)
        result |= WebInputEvent::ShiftKey;
    if (modifiers & Qt::ControlModifier)
        result |= WebInputEvent::ControlKey;
    if (modifiers & Qt::AltModifier)
        result |= WebInputEvent::AltKey;
    if (modifiers & Qt::MetaModifier)
        result |= WebInputEvent::MetaKey;
    return (WebInputEvent::Modifiers)result;
}

static WebInputEvent::Type webEventTypeForEvent(const QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return WebInputEvent::MouseDown;
    case QEvent::MouseButtonRelease:
        return WebInputEvent::MouseUp;
    case QEvent::MouseMove:
        return WebInputEvent::MouseMove;
    case QEvent::Wheel:
        return WebInputEvent::MouseWheel;
    case QEvent::KeyPress:
        return WebInputEvent::KeyDown;
    case QEvent::KeyRelease:
        return WebInputEvent::KeyUp;
    case QEvent::TouchBegin:
        return WebInputEvent::TouchStart;
    case QEvent::TouchUpdate:
        return WebInputEvent::TouchMove;
    case QEvent::TouchEnd:
        return WebInputEvent::TouchEnd;
    case QEvent::TouchCancel:
        return WebInputEvent::TouchCancel;
    case QEvent::MouseButtonDblClick:
        return WebInputEvent::Undefined;
    default:
        Q_ASSERT(false);
        return WebInputEvent::MouseMove;
    }
}


WebMouseEvent WebEventFactory::toWebMouseEvent(QMouseEvent *ev)
{
    WebMouseEvent webKitEvent;
    webKitEvent.timeStampSeconds = currentTimeForEvent(ev);
    webKitEvent.button = mouseButtonForEvent(ev);
    webKitEvent.modifiers = modifiersForEvent(ev->modifiers());

    webKitEvent.x = webKitEvent.windowX = ev->x();
    webKitEvent.y = webKitEvent.windowY = ev->y();
    webKitEvent.globalX = ev->globalX();
    webKitEvent.globalY = ev->globalY();

    webKitEvent.type = webEventTypeForEvent(ev);

    switch (ev->type()) {
    case QEvent::MouseButtonPress:
        webKitEvent.clickCount = 1;
        break;
    case QEvent::MouseButtonDblClick:
        webKitEvent.clickCount = 2;
        break;
    default:
        webKitEvent.clickCount = 0;
        break;
    };

    return webKitEvent;
}

WebKit::WebMouseWheelEvent WebEventFactory::toWebWheelEvent(QWheelEvent *ev)
{
    WebMouseWheelEvent webEvent;
    webEvent.type = webEventTypeForEvent(ev);
    webEvent.deltaX = 0;
    webEvent.deltaY = 0;
    webEvent.wheelTicksX = 0;
    webEvent.wheelTicksY = 0;
    webEvent.modifiers = modifiersForEvent(ev->modifiers());
    webEvent.timeStampSeconds = currentTimeForEvent(ev);

    if (ev->orientation() == Qt::Horizontal)
        webEvent.wheelTicksX = ev->delta() / 120.0f;
    else
        webEvent.wheelTicksY = ev->delta() / 120.0f;


    // Since we report the scroll by the pixel, convert the delta to pixel distance using standard scroll step.
    // Use the same single scroll step as QTextEdit (in QTextEditPrivate::init [h,v]bar->setSingleStep)
    static const float cDefaultQtScrollStep = 20.f;
    // ### FIXME: Default from QtGui. Should use Qt platform theme API once configurable.
    const int wheelScrollLines = 3;
    webEvent.deltaX = webEvent.wheelTicksX * wheelScrollLines * cDefaultQtScrollStep;
    webEvent.deltaY = webEvent.wheelTicksY * wheelScrollLines * cDefaultQtScrollStep;

    webEvent.x = webEvent.windowX = ev->x();
    webEvent.y = webEvent.windowY = ev->y();
    webEvent.globalX = ev->globalX();
    webEvent.globalY = ev->globalY();
    return webEvent;
}

content::NativeWebKeyboardEvent WebEventFactory::toWebKeyboardEvent(QKeyEvent *ev)
{
    content::NativeWebKeyboardEvent webKitEvent;
    webKitEvent.timeStampSeconds = currentTimeForEvent(ev);
    webKitEvent.modifiers = modifiersForEvent(ev->modifiers());
    webKitEvent.type = webEventTypeForEvent(ev);

    webKitEvent.nativeKeyCode = ev->nativeVirtualKey();
    // FIXME: need Windows keycode mapping from WebCore...
    memcpy(&webKitEvent.text, ev->text().utf16(), qMin(sizeof(webKitEvent.text), sizeof(ev->text().utf16())));
    return webKitEvent;
}
