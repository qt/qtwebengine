# include(../tests.pri)

TEMPLATE = app

CONFIG += testcase
VPATH += $$_PRO_FILE_PWD_
TARGET = tst_$$TARGET

#SOURCES += $${TARGET}.cpp
INCLUDEPATH += $$PWD

QT += testlib quick qml webengine

macx: CONFIG -= app_bundle

# This define is used by some tests to look up resources in the source tree
DEFINES += TESTS_SOURCE_DIR=\\\"$$PWD/\\\"

HEADERS += util.h

SOURCES += tst_qmltests.cpp \
           util.cpp

TARGET = tst_qmltests_WebEngineView
OBJECTS_DIR = .obj_WebEngineView


QT += qmltest

# Test the QML files under WebView in the source repository.
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\"$$PWD$${QMAKE_DIR_SEP}WebView\\\"\"
DEFINES += IMPORT_DIR=\"\\\"$${ROOT_BUILD_DIR}$${QMAKE_DIR_SEP}imports\\\"\"
DEFINES += QWP_PATH=\\\"$${ROOT_BUILD_DIR}/bin\\\"

OTHER_FILES += \
    WebEngineView/* \
    common/*
