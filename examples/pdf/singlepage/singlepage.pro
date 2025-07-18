TEMPLATE = app

QT += qml quick pdf svg

SOURCES += main.cpp

RESOURCES += \
    viewer.qrc
EXAMPLE_FILES = \
    Viewer.qml

target.path = $$[QT_INSTALL_EXAMPLES]/pdf/singlepage
INSTALLS += target

