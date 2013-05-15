#include "web_event_factory.h"

#include <QMouseEvent>
#include <QKeyEvent>

using namespace WebKit;

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


WebMouseEvent WebEventFactory::toWebMouseEvent(QMouseEvent *ev)
{
    WebMouseEvent webKitEvent;
    webKitEvent.timeStampSeconds = ev->timestamp() / 1000.0;
    webKitEvent.button = mouseButtonForEvent(ev);
    webKitEvent.modifiers = modifiersForEvent(ev->modifiers());

    webKitEvent.x = webKitEvent.windowX = ev->x();
    webKitEvent.y = webKitEvent.windowY = ev->y();
    webKitEvent.globalX = ev->globalX();
    webKitEvent.globalY = ev->globalY();

    webKitEvent.clickCount = 0;
    switch (ev->type()) {
    case QEvent::MouseButtonPress:
        webKitEvent.clickCount = 1;
        webKitEvent.type = WebInputEvent::MouseDown;
        break;
    case QEvent::MouseMove:
        webKitEvent.type = WebInputEvent::MouseMove;
        break;
    case QEvent::MouseButtonRelease:
        webKitEvent.type = WebInputEvent::MouseUp;
        break;
    case QEvent::MouseButtonDblClick:
        webKitEvent.clickCount = 2;
    default:
        Q_ASSERT(false);
    };

    return webKitEvent;
}

content::NativeWebKeyboardEvent WebEventFactory::toWebKeyboardEvent(QKeyEvent *ev)
{
    content::NativeWebKeyboardEvent webKitEvent;
    webKitEvent.timeStampSeconds = ev->timestamp() / 1000.0;
    webKitEvent.modifiers = modifiersForEvent(ev->modifiers());
    switch (ev->type()) {
    case QEvent::KeyPress:
        webKitEvent.type = WebInputEvent::KeyDown;
        break;
    case QEvent::KeyRelease:
        webKitEvent.type = WebInputEvent::KeyUp;
        break;
    }

    webKitEvent.nativeKeyCode = ev->nativeVirtualKey();
    // FIXME: need Windows keycode mapping from WebCore...
    memcpy(&webKitEvent.text, ev->text().utf16(), qMin(sizeof(webKitEvent.text), sizeof(ev->text().utf16())));
    return webKitEvent;
}
