CONFIG += testcase
TARGET = tst_qpdfsearchmodel
QT += pdf testlib network
macos:CONFIG -= app_bundle
SOURCES += tst_qpdfsearchmodel.cpp
