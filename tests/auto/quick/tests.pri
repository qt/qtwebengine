TEMPLATE = app

CONFIG += testcase
VPATH += $$_PRO_FILE_PWD_
TARGET = tst_$$TARGET

SOURCES += $${TARGET}.cpp
INCLUDEPATH += \
    $$PWD \
    $$PWD/../Api

QT += testlib network quick
QT_PRIVATE += quick-private gui-private core-private

include($$QTWEBENGINE_ROOT/common.pri)

# This define is used by some tests to look up resources in the source tree
DEFINES += TESTS_SOURCE_DIR=\\\"$$PWD/\\\"
