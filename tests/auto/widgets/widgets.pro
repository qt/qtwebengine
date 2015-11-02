TEMPLATE = subdirs

SUBDIRS += \
    qwebengineaccessibility \
    qwebenginepage \
    qwebenginehistory \
    qwebenginehistoryinterface \
    qwebengineinspector \
    qwebengineprofile \
    qwebenginescript \
    qwebengineview

qtHaveModule(positioning) {
    SUBDIRS += positionplugin
    qwebenginepage.depends = positionplugin
}
