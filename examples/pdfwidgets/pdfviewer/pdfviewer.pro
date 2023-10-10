TEMPLATE = app
TARGET = pdfviewer
QT += core gui widgets pdfwidgets

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    zoomselector.cpp

HEADERS += \
    mainwindow.h \
    zoomselector.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/pdfwidgets/pdfviewer
INSTALLS += target
