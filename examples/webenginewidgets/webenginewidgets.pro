QT_FOR_CONFIG += webengine

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

qtHaveModule(positioning): SUBDIRS += maps

qtConfig(webengine-spellchecker):!qtConfig(webengine-native-spellchecker):!cross_compile {
    SUBDIRS += spellchecker
} else {
    message("Spellcheck example will not be built because it depends on usage of Hunspell dictionaries.")
}

