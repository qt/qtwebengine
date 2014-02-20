TARGET = QtWebEngine

# For our export macros
DEFINES += QT_BUILD_WEBENGINE_LIB

QT += qml quick
QT_PRIVATE += qml-private quick-private gui-private core-private

INCLUDEPATH += $$PWD api ../core

# FIXME: all this should eventually be turned into QT += webenginecore
LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS_PRIVATE += -lQt5WebEngineCore -L$$LIBPATH
QMAKE_RPATHDIR += $$LIBPATH

#DESTDIR = $$LIBPATH

SOURCES = \
        api/qquickwebengineloadrequest.cpp \
        api/qquickwebenginenewviewrequest.cpp \
        api/qquickwebengineview.cpp \
        render_widget_host_view_qt_delegate_quick.cpp \
        ui_delegates_manager.cpp

HEADERS = \
        api/qtwebengineglobal.h \
        api/qtwebengineglobal_p.h \
        api/qquickwebengineloadrequest_p.h \
        api/qquickwebenginenewviewrequest_p.h \
        api/qquickwebengineview_p.h \
        api/qquickwebengineview_p_p.h \
        render_widget_host_view_qt_delegate_quick.h \
        ui_delegates_manager.h

load(qt_module)
