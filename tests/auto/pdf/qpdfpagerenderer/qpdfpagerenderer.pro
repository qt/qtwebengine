CONFIG += testcase
TARGET = tst_qpdfpagerenderer
QT += pdf testlib network
macos:CONFIG -= app_bundle
SOURCES += tst_qpdfpagerenderer.cpp
