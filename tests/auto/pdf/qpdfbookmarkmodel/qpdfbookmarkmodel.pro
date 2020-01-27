CONFIG += testcase
TARGET = tst_qpdfbookmarkmodel
QT += pdf testlib network
macos:CONFIG -= app_bundle
SOURCES += tst_qpdfbookmarkmodel.cpp
