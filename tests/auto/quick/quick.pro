include($$QTWEBENGINE_OUT_ROOT/src/webengine/qtwebengine-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webengine-private

TEMPLATE = subdirs

SUBDIRS += \
    dialogs \
    inspectorserver \
    qmltests \
    publicapi \
    qquickwebenginedefaultsurfaceformat \
    qquickwebengineview \
    qtbug-70248

qtConfig(webengine-testsupport) {
    SUBDIRS += \
        qmltests2 \
        qquickwebengineviewgraphics
}

qtConfig(ssl): SUBDIRS += qmltests_ssl

lessThan(QT_MAJOR_VERSION, 6):lessThan(QT_MINOR_VERSION, 14): SUBDIRS -= qmltests qmltests2 qmltests_ssl

# QTBUG-66055
boot2qt: SUBDIRS -= inspectorserver qquickwebengineview qmltests qmltests2
