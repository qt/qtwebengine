CXX_MODULE = qml
TARGET = qtwebengineplugin
TARGETPATH = QtWebEngine
IMPORT_VERSION = 1.0

QT += qml quick
QT_PRIVATE += qml-private quick-private gui-private core-private

# Remove this as soon as we have a hard-dependency on Qt 5.2
qtHaveModule(v8): QT_PRIVATE += v8-private

INCLUDEPATH += ../

# FIXME: all this should eventually be turned into QT += webenginecore
macx:LIBPATH = $$getOutDir()/$$getConfigDir()
else:LIBPATH = $$getOutDir()/$$getConfigDir()/lib
LIBS_PRIVATE += -lQt5WebEngineCore -L$$LIBPATH
QMAKE_RPATHDIR += $$LIBPATH

#DESTDIR = $$LIBPATH

SOURCES = \
        qquickwebengineview.cpp \
        plugin.cpp \
        render_widget_host_view_qt_delegate_quick.cpp

HEADERS = \
        qquickwebengineview_p.h \
        qquickwebengineview_p_p.h \
        render_widget_host_view_qt_delegate_quick.h

load(qml_plugin)
