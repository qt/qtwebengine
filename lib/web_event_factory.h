#ifndef WEB_EVENT_FACTORY_H
#define WEB_EVENT_FACTORY_H

#include "content/public/browser/native_web_keyboard_event.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebInputEvent.h"

class QMouseEvent;
class QKeyEvent;
class QWheelEvent;

class WebEventFactory {

public:
    static WebKit::WebMouseEvent toWebMouseEvent(QMouseEvent*);
    static WebKit::WebMouseWheelEvent toWebWheelEvent(QWheelEvent*);
    static content::NativeWebKeyboardEvent toWebKeyboardEvent(QKeyEvent*);

};


#endif
