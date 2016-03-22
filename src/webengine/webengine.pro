TARGET = QtWebEngine

# For our export macros
DEFINES += QT_BUILD_WEBENGINE_LIB

QT += qml quick webenginecore
QT_PRIVATE += quick-private gui-private core-private

QMAKE_DOCS = $$PWD/doc/qtwebengine.qdocconf

INCLUDEPATH += $$PWD api ../core ../core/api

SOURCES = \
        api/qquickwebenginecertificateerror.cpp \
        api/qquickwebenginecontextmenudata.cpp \
        api/qquickwebenginedownloaditem.cpp \
        api/qquickwebenginehistory.cpp \
        api/qquickwebenginefaviconprovider.cpp \
        api/qquickwebengineloadrequest.cpp \
        api/qquickwebenginenavigationrequest.cpp \
        api/qquickwebenginenewviewrequest.cpp \
        api/qquickwebengineprofile.cpp \
        api/qquickwebenginescript.cpp \
        api/qquickwebenginesettings.cpp \
        api/qquickwebenginesingleton.cpp \
        api/qquickwebengineview.cpp \
        api/qtwebengineglobal.cpp \
        render_widget_host_view_qt_delegate_quick.cpp \
        render_widget_host_view_qt_delegate_quickwindow.cpp \
        ui_delegates_manager.cpp

HEADERS = \
        api/qtwebengineglobal.h \
        api/qtwebengineglobal_p.h \
        api/qquickwebenginecertificateerror_p.h \
        api/qquickwebenginecontextmenudata_p.h \
        api/qquickwebenginedownloaditem_p.h \
        api/qquickwebenginedownloaditem_p_p.h \
        api/qquickwebenginehistory_p.h \
        api/qquickwebenginefaviconprovider_p_p.h \
        api/qquickwebengineloadrequest_p.h \
        api/qquickwebenginenavigationrequest_p.h \
        api/qquickwebenginenewviewrequest_p.h \
        api/qquickwebengineprofile.h \
        api/qquickwebengineprofile_p.h \
        api/qquickwebenginescript_p.h \
        api/qquickwebenginesettings_p.h \
        api/qquickwebenginesingleton_p.h \
        api/qquickwebengineview_p.h \
        api/qquickwebengineview_p_p.h \
        render_widget_host_view_qt_delegate_quick.h \
        render_widget_host_view_qt_delegate_quickwindow.h \
        ui_delegates_manager.h

isQMLTestSupportApiEnabled() {
    SOURCES += api/qquickwebenginetestsupport.cpp
    HEADERS += api/qquickwebenginetestsupport_p.h

    DEFINES += ENABLE_QML_TESTSUPPORT_API
}

!contains(WEBENGINE_CONFIG, no_spellcheck) {
    DEFINES += ENABLE_SPELLCHECK
}

load(qt_module)
