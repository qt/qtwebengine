include(../tests.pri)
QT += webengine
HEADERS += proxyserver.h
SOURCES += proxyserver.cpp

proxy_pac.name = QTWEBENGINE_CHROMIUM_FLAGS
proxy_pac.value = --proxy-pac-url="file://$$PWD/proxy.pac"

QT_TOOL_ENV += proxy_pac

