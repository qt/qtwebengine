QT      +=  webenginewidgets

macx: CONFIG -= app_bundle

# This is needed when std::tr1:bind is being used
QMAKE_CXXFLAGS -= -fno-rtti

HEADERS =   mainwindow.h
SOURCES =   main.cpp \
            mainwindow.cpp
RESOURCES = jquery.qrc

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/webkitwidgets/fancybrowser
#INSTALLS += target
