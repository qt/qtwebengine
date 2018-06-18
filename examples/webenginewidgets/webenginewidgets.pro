include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginecore

TEMPLATE=subdirs

SUBDIRS += \
    minimal \
    contentmanipulation \
    cookiebrowser \
    html2pdf \
    markdowneditor \
    simplebrowser \
    stylesheetbrowser \
    videoplayer

qtConfig(webengine-geolocation): SUBDIRS += maps

qtConfig(webengine-spellchecker):!qtConfig(webengine-native-spellchecker):!cross_compile {
    SUBDIRS += spellchecker
} else {
    message("Spellcheck example will not be built because it depends on usage of Hunspell dictionaries.")
}

