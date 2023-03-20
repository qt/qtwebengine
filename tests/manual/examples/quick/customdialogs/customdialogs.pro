QT += webenginequick

HEADERS += \
    server.h

SOURCES += \
    main.cpp \
    server.cpp

RESOURCES += \
    customdialogs.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/webenginequick/customdialogs
INSTALLS += target

qtHaveModule(widgets) {
    QT += widgets # QApplication is required to get native styling with QtQuickControls
}
