TEMPLATE = subdirs

SUBDIRS += \
    qwebenginedefaultsurfaceformat \
    qwebenginedownloads \
    qwebenginefaviconmanager \
    qwebenginepage \
    qwebenginehistory \
    qwebenginehistoryinterface \
    qwebengineinspector \
    qwebengineprofile \
    qwebenginescript \
    qwebenginesettings \
    qwebengineview

qtConfig(accessibility) {
    SUBDIRS += qwebengineaccessibility
}

contains(WEBENGINE_CONFIG, use_spellchecker):!cross_compile {
    !contains(WEBENGINE_CONFIG, use_native_spellchecker) {
        SUBDIRS += qwebenginespellcheck
    } else {
        message("Spellcheck test will not be built because it depends on usage of Hunspell dictionaries.")
    }
}

# QTBUG-60268
boot2qt: SUBDIRS -= qwebengineaccessibility qwebenginedefaultsurfaceformat \
                    qwebenginefaviconmanager qwebenginepage qwebenginehistory \
                    qwebengineprofile qwebenginescript qwebengineview \
                    qwebenginedownloads
