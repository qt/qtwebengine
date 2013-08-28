# Use Qt5 module system
load(qt_build_config)

TEMPLATE = lib
TARGET = QtWebEngineWidgets

MODULE = webenginewidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

CONFIG += c++11

QT += widgets
QT_PRIVATE += widgets-private gui-private core-private

# FIXME: all this should eventually be turned into QT += webenginecore
macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS += -L$$LIBPATH -lQt5WebEngineCore
QMAKE_RPATHDIR += $$LIBPATH

DESTDIR = $$LIBPATH

INCLUDEPATH += Api ../

SOURCES = \
        Api/qwebenginehistory.cpp \
        Api/qwebenginepage.cpp \
        Api/qwebengineview.cpp\
        render_widget_host_view_qt_delegate_widget.cpp

HEADERS = \
        Api/qtwebenginewidgetsglobal.h \
        Api/qwebenginehistory.h \
        Api/qwebenginepage.h \
        Api/qwebengineview.h \
        Api/qwebengineview_p.h \
        render_widget_host_view_qt_delegate_widget.h

load(qt_module)
