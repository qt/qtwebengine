include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
QT_FOR_CONFIG += buildtools-private

LIPO_OUT_FILE = $$OUT_PWD/$$getConfigDir()/$${TARGET}.a
!static {
   QMAKE_LFLAGS += $${LIPO_OUT_FILE}
} else {
   LIBS_PRIVATE += $${LIPO_OUT_FILE}
}

LIBS_PRIVATE += @$$OUT_PWD/$$QT_ARCH/$$getConfigDir()/$${TARGET}_libs.rsp

qtConfig(webengine-noexecstack): QMAKE_LFLAGS += -Wl,-z,noexecstack

POST_TARGETDEPS += $$LIPO_OUT_FILE
