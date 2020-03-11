CONFIG += testcase
TARGET = tst_qpdfpagenavigation
QT += pdf testlib network
macos:CONFIG -= app_bundle
SOURCES += tst_qpdfpagenavigation.cpp
