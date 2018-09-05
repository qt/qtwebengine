# NOTE: The TARGET, QT, QT_PRIVATE variables are used in both core_module.pro and core_gyp_generator.pro
# gyp/ninja will take care of the compilation, qmake/make will finish with linking and install.

TARGET = QtWebEngineCore
QT += qml quick webchannel
QT_PRIVATE += quick-private gui-private core-private webenginecoreheaders-private

qtHaveModule(positioning):QT += positioning

# LTO does not work for Chromium at the moment, so disable it completely for core.
CONFIG -= ltcg
