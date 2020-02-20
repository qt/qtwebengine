include(../proxypac.pri)

proxy_pac.name = QTWEBENGINE_CHROMIUM_FLAGS
win32:proxy_pac.value = --proxy-pac-url="file:///$$PWD/../proxy.pac"
else:proxy_pac.value = --proxy-pac-url="file://$$PWD/../proxy.pac"
boot2qt:proxy_pac.value = "--single-process --no-sandbox --proxy-pac-url=file://$$PWD/../proxy.pac"

QT_TOOL_ENV += proxy_pac

