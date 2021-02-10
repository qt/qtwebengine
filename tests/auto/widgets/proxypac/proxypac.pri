TEMPLATE = app
CONFIG += testcase
QT += testlib network webenginewidgets webenginecore
HEADERS += $$PWD/proxyserver.h
SOURCES += $$PWD/proxyserver.cpp $$PWD/tst_proxypac.cpp
