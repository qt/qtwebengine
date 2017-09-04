TEMPLATE = subdirs

SUBDIRS += \
    qwebengineaccessibility \
    qwebenginedefaultsurfaceformat \
    qwebenginefaviconmanager \
    qwebenginepage \
    qwebenginehistory \
    # Skipped to due issue in 58-based. Restore once 60-based is in.
    #qwebenginehistoryinterface \
    qwebengineinspector \
    qwebengineprofile \
    qwebenginescript \
    qwebenginesettings \
    qwebengineview

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
                    qwebengineprofile qwebenginescript qwebengineview
