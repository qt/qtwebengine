include(../proxypac.pri)

proxy_pac.name = QTWEBENGINE_CHROMIUM_FLAGS
proxy_pac.value = --proxy-pac-url="qrc:///proxy.pac"
boot2qt:proxy_pac.value = "--single-process --no-sandbox --proxy-pac-url=qrc:///proxy.pac"
QT_TOOL_ENV += proxy_pac
RESOURCES+= $$PWD/../proxypac.qrc
