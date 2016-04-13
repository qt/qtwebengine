TEMPLATE = app
TARGET = spellchecker
QT += webenginewidgets
CONFIG += c++11

HEADERS += \
    webview.h

SOURCES += \
    main.cpp \
    webview.cpp

RESOURCES += \
    data/spellchecker.qrc

DISTFILES += \
    dict/en/README.txt \
    dict/en/en-US.dic \
    dict/en/en-US.aff \
    dict/de/README.txt \
    dict/de/de-DE.dic \
    dict/de/de-DE.aff

target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/spellchecker
INSTALLS += target

qtPrepareTool(CONVERT_TOOL, qwebengine_convert_dict)

debug_and_release {
    CONFIG(debug, debug|release): DICTIONARIES_DIR = debug/qtwebengine_dictionaries
    else: DICTIONARIES_DIR = release/qtwebengine_dictionaries
} else {
    DICTIONARIES_DIR = qtwebengine_dictionaries
}

dict.files = $$PWD/dict/en/en-US.dic $$PWD/dict/de/de-DE.dic
dictoolbuild.input = dict.files
dictoolbuild.output = $${DICTIONARIES_DIR}/${QMAKE_FILE_BASE}.bdic
dictoolbuild.depends = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.aff
dictoolbuild.commands = $${CONVERT_TOOL} ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
dictoolbuild.name = Build ${QMAKE_FILE_IN_BASE}
dictoolbuild.CONFIG = no_link target_predeps
QMAKE_EXTRA_COMPILERS += dictoolbuild
