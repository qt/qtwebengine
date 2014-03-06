TARGET = QtWebEngineWidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

QT += widgets network
QT_PRIVATE += widgets-private gui-private network-private core-private

# FIXME: all this should eventually be turned into QT += webenginecore
macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS_PRIVATE += -L$$LIBPATH -lQt5WebEngineCore
QMAKE_RPATHDIR += $$LIBPATH

DESTDIR = $$LIBPATH

INCLUDEPATH += $$PWD api ../core

SOURCES = \
        api/qwebenginehistory.cpp \
        api/qwebenginehistoryinterface.cpp \
        api/qwebenginepage.cpp \
        api/qwebengineview.cpp\
        render_widget_host_view_qt_delegate_popup.cpp \
        render_widget_host_view_qt_delegate_webpage.cpp

HEADERS = \
        api/qtwebenginewidgetsglobal.h \
        api/qwebenginehistory.h \
        api/qwebenginehistoryinterface.h \
        api/qwebenginepage.h \
        api/qwebengineview.h \
        api/qwebengineview_p.h \
        render_widget_host_view_qt_delegate_popup.h \
        render_widget_host_view_qt_delegate_webpage.h

load(qt_module)

# Some binutils versions configured for cross compilation only support
# sysroot-relative rpath values and can't use the rpath specified above.
# Instead, append an appropriate rpath-link to lib_qt_webenginewidgets.pri.
cross_compile:!build_pass {
    local_build_rpath_link = "QMAKE_RPATHLINKDIR += $$LIBPATH"
    # MODULE_PRI is defined in qt_module_pris.prf
    write_file($$MODULE_PRI, local_build_rpath_link, append)
}
