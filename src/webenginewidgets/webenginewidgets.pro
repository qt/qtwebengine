TARGET = QtWebEngineWidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

QT += widgets network
QT_PRIVATE += webenginecore widgets-private gui-private network-private core-private

QMAKE_DOCS = $$PWD/doc/qtwebenginewidgets.qdocconf

INCLUDEPATH += $$PWD api ../core

SOURCES = \
        api/qwebenginehistory.cpp \
        api/qwebenginepage.cpp \
        api/qwebengineview.cpp\
        render_widget_host_view_qt_delegate_popup.cpp \
        render_widget_host_view_qt_delegate_webpage.cpp

HEADERS = \
        api/qtwebenginewidgetsglobal.h \
        api/qwebenginehistory.h \
        api/qwebenginepage.h \
        api/qwebengineview.h \
        api/qwebengineview_p.h \
        render_widget_host_view_qt_delegate_popup.h \
        render_widget_host_view_qt_delegate_webpage.h

load(qt_module)
