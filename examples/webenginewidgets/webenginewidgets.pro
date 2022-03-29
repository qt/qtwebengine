QT_FOR_CONFIG += webenginecore webenginecore-private

TEMPLATE=subdirs

SUBDIRS += \
    minimal \
    contentmanipulation \
    cookiebrowser \
    notifications \
    simplebrowser \
    stylesheetbrowser \
    videoplayer \
    webui

qtConfig(webengine-geolocation): SUBDIRS += maps
qtConfig(webengine-webchannel): SUBDIRS += markdowneditor

qtConfig(webengine-printing-and-pdf) {
    SUBDIRS += printme html2pdf
}

qtConfig(webengine-spellchecker):!qtConfig(webengine-native-spellchecker):!cross_compile {
    SUBDIRS += spellchecker
} else {
    message("Spellchecker example will not be built because it depends on usage of Hunspell dictionaries.")
}

