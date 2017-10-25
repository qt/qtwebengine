QT_FOR_CONFIG += webengine-private

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

qtConfig(spellchecker):!qtConfig(native-spellchecker):!cross_compile {
    SUBDIRS += spellchecker
} else {
    message("Spellcheck example will not be built because it depends on usage of Hunspell dictionaries.")
}

