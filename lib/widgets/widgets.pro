# Use Qt5 module system
load(qt_build_config)

TEMPLATE = lib
TARGET = QtWebEngineWidgets

MODULE = webenginewidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

QT += widgets

# FIXME: all this should eventually be turned into QT += webenginecore
macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS += -L$$LIBPATH -lQt5WebEngineCore
QMAKE_RPATHDIR += $$LIBPATH

DESTDIR = $$LIBPATH

INCLUDEPATH += ../

SOURCES = \
        Api/qwebcontentsview.cpp\
        render_widget_host_view_qt_delegate_widget.cpp

HEADERS = \
        Api/qwebcontentsview.h \
        Api/qwebcontentsview_p.h \
        render_widget_host_view_qt_delegate_widget.h

load(qt_module)
