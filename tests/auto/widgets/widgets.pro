TEMPLATE = subdirs

SUBDIRS += \
    qwebengineaccessibility \
    qwebenginedefaultsurfaceformat \
    qwebenginefaviconmanager \
    qwebenginepage \
    qwebenginehistory \
    qwebenginehistoryinterface \
    qwebengineinspector \
    qwebengineprofile \
    qwebenginescript \
    qwebenginesettings \
    qwebengineview

# QTBUG-53135, osx does not use hunspell
!contains(WEBENGINE_CONFIG, no_spellcheck):!osx:!cross_compile {
    SUBDIRS += qwebenginespellcheck
}

qtHaveModule(positioning) {
    SUBDIRS += positionplugin
    qwebenginepage.depends = positionplugin
}
