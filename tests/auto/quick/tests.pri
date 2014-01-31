TEMPLATE = app

# FIXME: Re-enable once we want to run tests on the CI
# CONFIG += testcase

VPATH += $$_PRO_FILE_PWD_
TARGET = tst_$$TARGET

SOURCES += $${TARGET}.cpp
INCLUDEPATH += $$PWD

QT += testlib network quick
QT_PRIVATE += quick-private gui-private core-private

macx: CONFIG -= app_bundle

# This define is used by some tests to look up resources in the source tree
DEFINES += TESTS_SOURCE_DIR=\\\"$$PWD/\\\"
