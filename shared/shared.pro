# This is a dummy .pro file used to extract some aspects of the used configuration and feed them to gyp
# We want the gyp generation step to happen after all the other config steps. For that we need to prepend
# our gyp_generator.prf feature to the CONFIG variable since it is processed backwards
CONFIG = gyp_generator $$CONFIG
GYPINCLUDES += ../qtwebengine.gypi

TEMPLATE = lib
CONFIG += static

TARGET = qtwebengine_shared

# Defining keywords such as 'signal' clashes with the chromium code base.
DEFINES += QT_NO_KEYWORDS \
           Q_FORWARD_DECLARE_OBJC_CLASS=QT_FORWARD_DECLARE_CLASS

# something fishy with qmake in 5.2 ?
INCLUDEPATH += $$[QT_INSTALL_HEADERS]

# We need a way to tap into gyp´s Debug vs. Release configuration
PER_CONFIG_DEFINES = QTWEBENGINEPROCESS_PATH=\\\"$$getOutDir()/%config/$$QTWEBENGINEPROCESS_NAME\\\"

# Keep Skia happy
CONFIG(release, debug|release): DEFINES += NDEBUG

QT += gui
QT_PRIVATE += gui-private

SOURCES = \
        resource_bundle_qt.cpp \
        shared_globals.cpp

HEADERS = \
        shared_globals.h
