TEMPLATE = app

QT += qml quick pdf svg

SOURCES += main.cpp

RESOURCES += \
    viewer.qrc
EXAMPLE_FILES = \
    viewer.qml

target.path = $$[QT_INSTALL_EXAMPLES]/pdf/pdfviewer
INSTALLS += target

