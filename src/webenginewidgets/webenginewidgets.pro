# Use Qt5 module system
load(qt_build_config)

TEMPLATE = lib
TARGET = QtWebEngineWidgets

MODULE = webenginewidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

QT += widgets
QT_PRIVATE += widgets-private gui-private core-private

# FIXME: all this should eventually be turned into QT += webenginecore
macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS_PRIVATE += -lQt5WebEngineCore -L$$LIBPATH
QMAKE_RPATHDIR += $$LIBPATH

DESTDIR = $$LIBPATH

INCLUDEPATH += api ../core

SOURCES = \
        api/qwebenginehistory.cpp \
        api/qwebenginepage.cpp \
        api/qwebengineview.cpp\
        render_widget_host_view_qt_delegate_widget.cpp

HEADERS = \
        api/qtwebenginewidgetsglobal.h \
        api/qwebenginehistory.h \
        api/qwebenginepage.h \
        api/qwebengineview.h \
        api/qwebengineview_p.h \
        render_widget_host_view_qt_delegate_widget.h

load(qt_module)

# QNX ld only supports staging-relative rpath values and can't use the rpath specified above.
# Instead, append an appropriate rpath-link to lib_qt_webenginewidgets.pri.
qnx:!build_pass {
    local_build_rpath_link = "QMAKE_RPATHLINKDIR += $$LIBPATH"
    # MODULE_PRI is defined in qt_module_pris.prf
    write_file($$MODULE_PRI, local_build_rpath_link, append)
}
