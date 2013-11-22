QT      +=  webenginewidgets

macx: CONFIG -= app_bundle
qnx: QMAKE_LFLAGS += $$QMAKE_LFLAGS_RPATHLINK$$getOutDir()/$$getConfigDir()/lib

HEADERS =   mainwindow.h
SOURCES =   main.cpp \
            mainwindow.cpp
RESOURCES = jquery.qrc

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/webkitwidgets/fancybrowser
#INSTALLS += target
