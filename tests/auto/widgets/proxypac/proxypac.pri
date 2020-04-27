TEMPLATE = app
CONFIG += testcase
QT += testlib network webenginewidgets webengine
HEADERS += $$PWD/proxyserver.h
SOURCES += $$PWD/proxyserver.cpp $$PWD/tst_proxypac.cpp
