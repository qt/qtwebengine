include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += webenginecore webenginecore-private

# NOTE: The TARGET, QT, QT_PRIVATE variables are used in both core_module.pro and core_gyp_generator.pro
# gyp/ninja will take care of the compilation, qmake/make will finish with linking and install.

TARGET = QtWebEngineCore
QT += qml-private quick-private gui-private core-private
QT_PRIVATE += webenginecoreheaders-private

qtConfig(webengine-geolocation): QT += positioning
qtConfig(webengine-webchannel): QT += webchannel

# LTO does not work for Chromium at the moment, so disable it completely for core.
CONFIG -= ltcg

# Chromium requires C++14
CONFIG += c++14

#QTBUG-73216 ci has to be updated with latest yocto
boot2qt: CONFIG -= use_gold_linker

