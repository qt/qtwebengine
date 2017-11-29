QT_FOR_CONFIG += webengine-private

TEMPLATE = subdirs

SUBDIRS += \
    inspectorserver \
    publicapi \
    qquickwebenginedefaultsurfaceformat \
    qquickwebengineview

qtConfig(webengine-testsupport) {
    SUBDIRS += \
        qmltests \
        qquickwebengineviewgraphics
}

# QTBUG-60268
boot2qt: SUBDIRS -= inspectorserver qquickwebenginedefaultsurfaceformat qquickwebengineview
