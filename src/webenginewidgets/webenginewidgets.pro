TARGET = QtWebEngineWidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

QT += webengine widgets network quick
QT_PRIVATE += webenginecore widgets-private quick-private gui-private network-private core-private

QMAKE_DOCS = $$PWD/doc/qtwebenginewidgets.qdocconf

INCLUDEPATH += $$PWD api ../core ../webengine/api

SOURCES = \
        api/qtwebenginewidgetsglobal.cpp \
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
