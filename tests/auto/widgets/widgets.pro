TEMPLATE = subdirs

SUBDIRS += \
    qwebengineaccessibility \
    qwebenginedefaultsurfaceformat \
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
