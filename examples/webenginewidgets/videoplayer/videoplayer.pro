TEMPLATE = app

QT += webenginewidgets

HEADERS += \
    mainwindow.h \
    fullscreenwindow.h \
    fullscreennotification.h

SOURCES += main.cpp \
    mainwindow.cpp \
    fullscreenwindow.cpp \
    fullscreennotification.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/videoplayer
INSTALLS += target
