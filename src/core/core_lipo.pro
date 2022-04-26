TEMPLATE = aux

qtConfig(debug_and_release): CONFIG += debug_and_release
qtConfig(build_all): CONFIG += build_all

TARGET= QtWebEngineCore
include($${QTWEBENGINE_ROOT}/src/buildtools/config/lipo.pri)


