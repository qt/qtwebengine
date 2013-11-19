TARGET = QtWebEngine
MODULE = webengine

# For our export macros
DEFINES += QT_BUILD_WEBENGINE_LIB

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
        render_widget_host_view_qt_delegate_quick.cpp

HEADERS = \
        qtwebengineglobal.h \
        qtwebengineglobal_p.h \
        qquickwebengineview_p.h \
        qquickwebengineview_p_p.h \
        render_widget_host_view_qt_delegate_quick.h

load(qt_module)
