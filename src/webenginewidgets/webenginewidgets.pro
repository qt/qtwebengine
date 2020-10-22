include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += webenginecore-private

TARGET = QtWebEngineWidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

QT += webenginecore widgets network quick
QT_PRIVATE += quick-private gui-private core-private widgets-private quickwidgets webenginecore-private

INCLUDEPATH += $$PWD api ../core ../core/api ../webengine/api

SOURCES = \
        api/qwebenginenotificationpresenter.cpp \
        api/qwebengineview.cpp \
        render_widget_host_view_qt_delegate_widget.cpp

HEADERS = \
        api/qtwebenginewidgetsglobal.h \
        api/qwebenginenotificationpresenter_p.h \
        api/qwebengineview.h \
        api/qwebengineview_p.h \
        render_widget_host_view_qt_delegate_widget.h

load(qt_module)
