HEADERS += $$PWD/httpserver.h $$PWD/httpreqrep.h
SOURCES += $$PWD/httpserver.cpp $$PWD/httpreqrep.cpp
INCLUDEPATH += $$PWD
DEFINES += TESTS_SHARED_DATA_DIR=\\\"$$re_escape($$PWD$${QMAKE_DIR_SEP}data)\\\"
