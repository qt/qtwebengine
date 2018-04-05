QT_FOR_CONFIG += webengine

TEMPLATE = subdirs

SUBDIRS += \
    origins \
    qwebenginedefaultsurfaceformat \
    qwebenginedownloads \
    qwebenginefaviconmanager \
    qwebenginepage \
    qwebenginehistory \
    qwebengineinspector \
    qwebengineprofile \
    qwebengineschemes \
    qwebenginescript \
    qwebenginesettings \
    qwebengineshutdown \
    qwebengineview

qtConfig(accessibility) {
    SUBDIRS += qwebengineaccessibility
}

qtConfig(webengine-spellchecker):!cross_compile {
    !qtConfig(webengine-native-spellchecker) {
        SUBDIRS += qwebenginespellcheck
    } else {
        message("Spellcheck test will not be built because it depends on usage of Hunspell dictionaries.")
    }
}

# QTBUG-60268
boot2qt: SUBDIRS -= qwebengineaccessibility qwebenginedefaultsurfaceformat \
                    qwebenginefaviconmanager qwebenginepage qwebenginehistory \
                    qwebengineprofile qwebengineschemes qwebenginescript \
                    qwebengineview qwebenginedownloads qwebenginesettings \
                    origins
