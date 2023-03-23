TEMPLATE = app
TARGET = recipebrowser
QT += webenginewidgets

HEADERS += \
    mainwindow.h \
    stylesheetdialog.h \
    document.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    stylesheetdialog.cpp \
    document.cpp

FORMS += \
    mainwindow.ui \
    stylesheetdialog.ui

RESOURCES += recipebrowser.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/recipebrowser
INSTALLS += target
