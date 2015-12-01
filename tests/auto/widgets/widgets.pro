TEMPLATE = subdirs

SUBDIRS += \
    qwebengineaccessibility \
    qwebenginefaviconmanager \
    qwebenginepage \
    qwebenginehistory \
    qwebenginehistoryinterface \
    qwebengineinspector \
    qwebengineprofile \
    qwebenginescript \
    qwebenginesettings \
    qwebengineview

qtHaveModule(positioning) {
    SUBDIRS += positionplugin
    qwebenginepage.depends = positionplugin
}
