QT_FOR_CONFIG += webenginecore webenginecore-private network-private

TEMPLATE=subdirs

SUBDIRS += \
    contentmanipulation \
    cookiebrowser \
    notifications \
    simplebrowser \
    push-notifications \
    videoplayer

qtConfig(webengine-geolocation): SUBDIRS += maps
qtConfig(webengine-webchannel): SUBDIRS += recipebrowser

qtConfig(webengine-printing-and-pdf) {
    SUBDIRS += printme html2pdf
}

qtConfig(webengine-spellchecker):!qtConfig(webengine-native-spellchecker):!cross_compile {
    SUBDIRS += spellchecker
} else {
    message("Spellchecker example will not be built because it depends on usage of Hunspell dictionaries.")
}

qtConfig(ssl): SUBDIRS += clientcertificate
