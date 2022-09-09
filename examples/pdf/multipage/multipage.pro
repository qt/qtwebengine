TEMPLATE = app

QT += qml quick pdf svg

SOURCES += main.cpp pdfapplication.cpp

RESOURCES += \
    viewer.qrc
EXAMPLE_FILES = \
    viewer.qml

target.path = $$[QT_INSTALL_EXAMPLES]/pdf/multipage
INSTALLS += target

macos:QMAKE_INFO_PLIST = resources/macos/Info.plist
macos:ICON = resources/multipage.icns
