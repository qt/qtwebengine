TEMPLATE = app

QT += quick qml quickcontrols2 webengine

cross_compile {
  posix|qnx|linux: DEFINES += QTWEBENGINE_RECIPE_BROWSER_EMBEDDED
}

SOURCES += main.cpp

RESOURCES += resources/resources.qrc

DISTFILES += \
    resources/pages/assets/3rdparty/MARKDOWN-LICENSE.txt \
    resources/pages/assets/3rdparty/MARKED-LICENSE.txt

target.path = $$[QT_INSTALL_EXAMPLES]/webengine/recipebrowser
INSTALLS += target
