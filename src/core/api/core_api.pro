TARGET = qtwebenginecoreapi
DESTDIR = $$OUT_PWD/$$getConfigDir()

TEMPLATE = lib

CONFIG += staticlib c++11
QT += network

# Don't create .prl file for this intermediate library because
# their contents get used when linking against them, breaking
# "-Wl,-whole-archive -lqtwebenginecoreapi --Wl,-no-whole-archive"
CONFIG -= create_prl

# Copy this logic from qt_module.prf so that the intermediate library can be
# created to the same rules as the final module linking in core_module.pro.
!host_build:if(win32|mac):!macx-xcode {
    contains(QT_CONFIG, debug_and_release):CONFIG += debug_and_release
    contains(QT_CONFIG, build_all):CONFIG += build_all
}

DEFINES += \
    BUILDING_CHROMIUM \
    NOMINMAX

CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
INCLUDEPATH += $$QTWEBENGINE_ROOT/src/core \
               $$CHROMIUM_SRC_DIR

linux-g++*: QMAKE_CXXFLAGS += -Wno-unused-parameter

HEADERS = \
    qwebenginecallback.h \
    qwebenginecallback_p.h \
    qtwebenginecoreglobal.h \
    qtwebenginecoreglobal_p.h \
    qwebenginecookiestoreclient.h \
    qwebenginecookiestoreclient_p.h \

SOURCES = \
    qwebenginecookiestoreclient.cpp \
