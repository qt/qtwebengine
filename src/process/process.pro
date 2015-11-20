TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app
!build_pass:contains(QT_CONFIG, debug_and_release):contains(QT_CONFIG, build_all): CONFIG += release
# Needed to set LSUIElement=1
QMAKE_INFO_PLIST = Info_mac.plist

load(qt_build_paths)
contains(QT_CONFIG, qt_framework) {
    # Deploy the QtWebEngineProcess app bundle into the QtWebEngineCore framework.
    DESTDIR = $$MODULE_BASE_OUTDIR/lib/QtWebEngineCore.framework/Versions/5/Helpers
    # FIXME: remove the following workaround with proper rpath handling or
    # patching of the installed QtWebEngineProcess binary.
    # Since QtWebEngineCore is now built as a framework, we need to pull
    # in and fixup its dependencies as well.
    QT += webenginecore
    QMAKE_POST_LINK = \
        "xcrun install_name_tool -change " \
        "`xcrun otool -X -L $(TARGET) | grep QtWebEngineCore | cut -d ' ' -f 1` " \
        "@executable_path/../../../../QtWebEngineCore " \
        "$(TARGET); "
    linked_frameworks = QtQuick QtQml QtNetwork QtCore QtGui QtWebChannel
    for (current_framework, linked_frameworks) {
        QMAKE_POST_LINK += "xcrun install_name_tool -change " \
        "`xcrun otool -X -L $(TARGET) | grep $${current_framework} | cut -d ' ' -f 1` " \
        "@executable_path/../../../../../../../$${current_framework}.framework/$${current_framework} " \
        "$(TARGET);"
    }
} else {
    CONFIG -= app_bundle
    win32: DESTDIR = $$MODULE_BASE_OUTDIR/bin
    else:  DESTDIR = $$MODULE_BASE_OUTDIR/libexec

    QT_PRIVATE += webenginecore
}

msvc: QMAKE_LFLAGS *= /LARGEADDRESSAWARE

INCLUDEPATH += ../core

SOURCES = main.cpp

win32 {
    SOURCES += \
        support_win.cpp
}

contains(QT_CONFIG, qt_framework) {
    target.path = $$[QT_INSTALL_LIBS]/QtWebEngineCore.framework/Versions/5/Helpers
} else {
    target.path = $$[QT_INSTALL_LIBEXECS]
}
INSTALLS += target
