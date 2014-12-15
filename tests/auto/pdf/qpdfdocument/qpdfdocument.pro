CONFIG += testcase
TARGET = tst_qpdfdocument
QT += pdf printsupport testlib network
macx:CONFIG -= app_bundle
SOURCES += tst_qpdfdocument.cpp

