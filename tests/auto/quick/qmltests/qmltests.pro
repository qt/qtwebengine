include(../tests.pri)

QT += qmltest
QT_PRIVATE += quick-private

IMPORTPATH += $$PWD/data

INCLUDEPATH += $$PWD/../shared

OTHER_FILES += \
    $$PWD/data/TestWebEngineView.qml \
    $$PWD/data/favicon.html
    $$PWD/data/favicon.png
    $$PWD/data/favicon2.html
    $$PWD/data/small-favicon.png
    $$PWD/data/test1.html \
    $$PWD/data/test3.html \
    $$PWD/data/tst_favIconLoad.qml \
    $$PWD/data/tst_loadHtml.qml \
    $$PWD/data/tst_loadProgress.qml \
    $$PWD/data/tst_loadProgressSignal.qml \
    $$PWD/data/tst_loadUrl.qml \
    $$PWD/data/tst_properties.qml \
    $$PWD/data/tst_runJavaScript.qml \
    $$PWD/data/tst_titleChanged.qml

load(qt_build_paths)
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\"$$PWD$${QMAKE_DIR_SEP}data\\\"\"
DEFINES += QWP_PATH=\"\\\"$${MODULE_BASE_OUTDIR}$${QMAKE_DIR_SEP}libexec$${QMAKE_DIR_SEP}$${QTWEBENGINEPROCESS_NAME}\\\"\"
